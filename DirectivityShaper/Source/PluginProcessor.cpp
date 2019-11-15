/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */


#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr float filterTypePresets[] = {1.0f, 2.0f, 2.0f, 3.0f};
static constexpr float filterFrequencyPresets[] = {200.0f, 300.0f, 1600.0f, 2200.0f};

//==============================================================================
DirectivityShaperAudioProcessor::DirectivityShaperAudioProcessor()
: AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
                      BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::mono(), true)
#endif
                  .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                  ,
#endif
createParameterLayout())
{
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    probeAzimuth = parameters.getRawParameterValue ("probeAzimuth");
    probeElevation = parameters.getRawParameterValue ("probeElevation");
    probeRoll = parameters.getRawParameterValue ("probeRoll");
    probeLock = parameters.getRawParameterValue ("probeLock");
    normalization = parameters.getRawParameterValue("normalization");

    parameters.addParameterListener("orderSetting", this);
    parameters.addParameterListener("probeLock", this);
    parameters.addParameterListener("probeAzimuth", this);
    parameters.addParameterListener("probeElevation", this);
    parameters.addParameterListener("probeRoll", this);


    for (int i = 0; i < numberOfBands; ++i)
    {
        filterType[i] = parameters.getRawParameterValue ("filterType" + String(i));
        filterFrequency[i] = parameters.getRawParameterValue ("filterFrequency" + String(i));
        filterQ[i] = parameters.getRawParameterValue ("filterQ" + String(i));
        filterGain[i] = parameters.getRawParameterValue ("filterGain" + String(i));
        order[i] = parameters.getRawParameterValue ("order" + String(i));
        shape[i] = parameters.getRawParameterValue ("shape" + String(i));
        azimuth[i] = parameters.getRawParameterValue ("azimuth" + String(i));
        elevation[i] = parameters.getRawParameterValue ("elevation" + String(i));
        parameters.addParameterListener("filterType" + String(i), this);
        parameters.addParameterListener("filterFrequency" + String(i), this);
        parameters.addParameterListener("filterQ" + String(i), this);
        parameters.addParameterListener("filterGain" + String(i), this);
        parameters.addParameterListener("azimuth" + String(i), this);
        parameters.addParameterListener("elevation" + String(i), this);
        parameters.addParameterListener("order" + String(i), this);
        parameters.addParameterListener("shape" + String(i), this);
        parameters.addParameterListener("normalization", this);

        probeGains[i] = 0.0f;
    }


    FloatVectorOperations::clear(shOld[0], 64 * numberOfBands);
    FloatVectorOperations::clear(weights[0], 8 * numberOfBands);


    for (int i = 0; i < numberOfBands; ++i)
    {
        filter[i].coefficients = createFilterCoefficients(roundToInt(*filterType[i]), 44100, *filterFrequency[i], *filterQ[i]);
    }
}

inline dsp::IIR::Coefficients<float>::Ptr DirectivityShaperAudioProcessor::createFilterCoefficients(int type, double sampleRate, double frequency, double Q)
{
    frequency = jmin (0.5 * sampleRate, frequency);
    switch (type) {
        case 1:
            return IIR::Coefficients<float>::makeLowPass(sampleRate, frequency, Q);
            break;
        case 2:
            return IIR::Coefficients<float>::makeBandPass(sampleRate, frequency, Q);
            break;
        case 3:
            return IIR::Coefficients<float>::makeHighPass(sampleRate, frequency, Q);
            break;
        default:
            return IIR::Coefficients<float>::makeAllPass(sampleRate, frequency, Q);
            break;
    }
}
DirectivityShaperAudioProcessor::~DirectivityShaperAudioProcessor()
{
}

//==============================================================================
int DirectivityShaperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int DirectivityShaperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DirectivityShaperAudioProcessor::setCurrentProgram (int index)
{
}

const String DirectivityShaperAudioProcessor::getProgramName (int index)
{
    return {};
}

void DirectivityShaperAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DirectivityShaperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, 1, *orderSetting, true);

    for (int i = 0; i < numberOfBands; ++i)
    {
        *filter[i].coefficients = *createFilterCoefficients(roundToInt(*filterType[i]), sampleRate, *filterFrequency[i], *filterQ[i]);
        filter[i].reset();
    }
    repaintFV = true;

    filteredBuffer.setSize(numberOfBands, samplesPerBlock);
}

void DirectivityShaperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


void DirectivityShaperAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, 1, *orderSetting);
    ScopedNoDenormals noDenormals;

    const bool applySN3D = *useSN3D > 0.5f;

    int nChToWorkWith = jmin(buffer.getNumChannels(), output.getNumberOfChannels());
    const int orderToWorkWith = isqrt(nChToWorkWith) - 1;
    nChToWorkWith = squares[orderToWorkWith+1];

    const int numSamples = buffer.getNumSamples();

    AudioBlock<float> inBlock = AudioBlock<float>(buffer.getArrayOfWritePointers(), 1, numSamples);
    for (int i = 0; i < numberOfBands; ++i)
    {
        //filteredBuffer.copyFrom(i, 0, buffer, 0, 0, numSamples);
        AudioBlock<float> outBlock = AudioBlock<float>(filteredBuffer.getArrayOfWritePointers() + i, 1, numSamples);
        filter[i].process(ProcessContextNonReplacing<float>(inBlock, outBlock));
    }

    buffer.clear();

    float sh[64];
    float probeSH[64];

    {
        Vector3D<float> pos = Conversions<float>::sphericalToCartesian(degreesToRadians(*probeAzimuth), degreesToRadians(*probeElevation));
        SHEval(orderToWorkWith, pos.x, pos.y, pos.z, probeSH, false); // decoding -> false
        if (applySN3D)
        { // reverting SN3D in probeSH
            FloatVectorOperations::multiply(probeSH, sn3d2n3d, nChToWorkWith);
        }
    }

    WeightsHelper::Normalization norm;
    if (*normalization < 0.5f)
        norm = WeightsHelper::Normalization::BasicDecode;
    else if (*normalization >= 0.5f && *normalization < 1.5f)
        norm = WeightsHelper::Normalization::OnAxis;
    else
        norm = WeightsHelper::Normalization::ConstantEnergy;


    for (int b = 0; b < numberOfBands; ++b)
    {
        float tempWeights[8];
        const int nWeights = WeightsHelper::getWeights(*order[b], *shape[b], tempWeights);

        // fill higher orders with zeros
        for (int i = nWeights; i < 8; ++i)
            tempWeights[i] = 0.0f;

        // ==== COPY WEIGHTS FOR GUI VISUALIZATION ====
        // copy non-normalized weights for GUI
        FloatVectorOperations::copy(weights[b], tempWeights, 8);

        // normalize weights for GUI (7th order decode)
        WeightsHelper::applyNormalization(weights[b], *order[b], 7, norm);
        // ============================================

        // normalize weights for audio
        WeightsHelper::applyNormalization(tempWeights, *order[b], orderToWorkWith, norm, applySN3D);


        Vector3D<float> pos = Conversions<float>::sphericalToCartesian(degreesToRadians(*azimuth[b]), degreesToRadians(*elevation[b]));
        SHEval(orderToWorkWith, pos.x, pos.y, pos.z, sh, true); // encoding -> true

        float temp = 0.0f;
        float shTemp[64];
        FloatVectorOperations::multiply(shTemp, sh, Decibels::decibelsToGain(*filterGain[b]), 64);
        for (int i = 0; i < nChToWorkWith; ++i)
        {
            shTemp[i] *= tempWeights[isqrt(i)];
            temp += shTemp[i] * probeSH[i];
            buffer.addFromWithRamp(i, 0, filteredBuffer.getReadPointer(b), numSamples, shOld[b][i], shTemp[i]);
        }

        probeGains[b] = std::abs(temp);

        if (probeChanged)
        {
            probeChanged = false;
            repaintFV = true;
            repaintSphere = true;
        }
        FloatVectorOperations::copy(shOld[b], shTemp, 64);
    }

    if (changeWeights)
    {
        changeWeights = false;
        repaintDV = true;
        repaintXY = true;
        repaintFV = true;
    }
}

//==============================================================================
bool DirectivityShaperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DirectivityShaperAudioProcessor::createEditor()
{
    return new DirectivityShaperAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void DirectivityShaperAudioProcessor::getStateInformation (MemoryBlock &destData)
{
  auto state = parameters.copyState();

  auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
  oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

  std::unique_ptr<XmlElement> xml (state.createXml());
  copyXmlToBinary (*xml, destData);
}

void DirectivityShaperAudioProcessor::setStateInformation (const void *data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort")) // legacy
            {
                oscParameterInterface.getOSCReceiver().connect (parameters.state.getProperty ("OSCPort", var (-1)));
                parameters.state.removeProperty ("OSCPort", nullptr);
            }

            auto oscConfig = parameters.state.getChildWithName ("OSCConfig");
            if (oscConfig.isValid())
                oscParameterInterface.setConfig (oscConfig);
        }
}

//==============================================================================
void DirectivityShaperAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "orderSetting")
    {
        userChangedIOSettings = true;
        changeWeights = true;
    }
    else if (parameterID == "probeLock")
    {
        if (newValue >= 0.5f && !toggled)
        {
            DBG("toggled");

            float ypr[3];
            ypr[2] = 0.0f;
            for (int i = 0; i < numberOfBands; ++i)
            {
                iem::Quaternion<float> probeQuat;
                float probeypr[3];
                probeypr[0] = degreesToRadians(*probeAzimuth);
                probeypr[1] = degreesToRadians(*probeElevation);
                probeypr[2] = - degreesToRadians(*probeRoll);
                probeQuat.fromYPR(probeypr);
                probeQuat.conjugate();

                ypr[0] = degreesToRadians(*azimuth[i]);
                ypr[1] = degreesToRadians(*elevation[i]);
                quats[i].fromYPR(ypr);
                quats[i] = probeQuat*quats[i];
            }
            toggled = true;
        }
        else if (newValue < 0.5f)
            toggled = false;
    }
    else if ((parameterID == "probeAzimuth") ||  (parameterID == "probeElevation") ||  (parameterID == "probeRoll"))
    {

        if (toggled)
        {
            DBG("moving");
            moving = true;
            iem::Quaternion<float> probeQuat;
            float ypr[3];
            ypr[0] = degreesToRadians(*probeAzimuth);
            ypr[1] = degreesToRadians(*probeElevation);
            ypr[2] = - degreesToRadians(*probeRoll);
            probeQuat.fromYPR(ypr);

            for (int i = 0; i < numberOfBands; ++i)
            {
                iem::Quaternion<float> temp = probeQuat*quats[i];
                temp.toYPR(ypr);
                parameters.getParameter ("azimuth" + String (i))->setValueNotifyingHost (parameters.getParameterRange ("azimuth" + String (i)).convertTo0to1 (radiansToDegrees (ypr[0])));
                parameters.getParameter ("elevation" + String (i))->setValueNotifyingHost (parameters.getParameterRange ("elevation" + String (i)).convertTo0to1 (radiansToDegrees(ypr[1])));
            }
            moving = false;
            repaintSphere = true;
        }
        else
            probeChanged = true;

    }
    else if (parameterID.startsWith("azimuth") || parameterID.startsWith("elevation"))
    {
        if (toggled && !moving)
        {
            float ypr[3];
            ypr[2] = 0.0f;
            for (int i = 0; i < numberOfBands; ++i)
            {
                iem::Quaternion<float> probeQuat;
                float probeypr[3];
                probeypr[0] = degreesToRadians(*probeAzimuth);
                probeypr[1] = degreesToRadians(*probeElevation);
                probeypr[2] = - degreesToRadians(*probeRoll);
                probeQuat.fromYPR(probeypr);
                probeQuat.conjugate();

                ypr[0] = degreesToRadians(*azimuth[i]);
                ypr[1] = degreesToRadians(*elevation[i]);
                quats[i].fromYPR(ypr);
                quats[i] = probeQuat*quats[i];
            }
        }
        repaintSphere = true;
        probeChanged = true;
    }
    else if (parameterID.startsWith("filter"))
    {
        int i = parameterID.getLastCharacters(1).getIntValue();
        *filter[i].coefficients = *createFilterCoefficients(roundToInt(*filterType[i]), getSampleRate(), *filterFrequency[i], *filterQ[i]);
        repaintFV = true;
    }
    else if (parameterID.startsWith("order") || parameterID.startsWith("shape"))
    {
        changeWeights = true;
    }
    else if (parameterID == "normalization")
        changeWeights = true;
}


//==============================================================================
std::vector<std::unique_ptr<RangedAudioParameter>> DirectivityShaperAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("orderSetting", "Directivity Order", "",
                                     NormalisableRange<float> (0.0f, 8.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value >= 0.5f && value < 1.5f) return "0th";
                                         else if (value >= 1.5f && value < 2.5f) return "1st";
                                         else if (value >= 2.5f && value < 3.5f) return "2nd";
                                         else if (value >= 3.5f && value < 4.5f) return "3rd";
                                         else if (value >= 4.5f && value < 5.5f) return "4th";
                                         else if (value >= 5.5f && value < 6.5f) return "5th";
                                         else if (value >= 6.5f && value < 7.5f) return "6th";
                                         else if (value >= 7.5f) return "7th";
                                         else return "Auto";},
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("useSN3D", "Directivity Normalization", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) { if (value >= 0.5f ) return "SN3D";
                                         else return "N3D"; },
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("probeAzimuth", "probe Azimuth", CharPointer_UTF8 (R"(°)"),
                                    NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                    [](float value) {return String(value, 2);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("probeElevation", "probe Elevation", CharPointer_UTF8 (R"(°)"),
                                    NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                    [](float value) {return String(value, 2);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("probeRoll", "probe Roll", CharPointer_UTF8 (R"(°)"),
                                    NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                    [](float value) {return String(value, 2);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("probeLock", "Lock Directions", "",
                                    NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0,
                                    [](float value) {return (value >= 0.5f) ? "locked" : "not locked";}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("normalization", "Directivity Normalization", "",
                                    NormalisableRange<float> (0.0f, 2.0f, 1.0f), 1.0,
                                    [](float value) {
                                        if (value >= 0.5f && value < 1.5f) return "on axis";
                                        else if (value >= 1.5f && value < 2.5f) return "constant energy";
                                        else return "basic decode";
                                    }, nullptr));

    for (int i = 0; i < numberOfBands; ++i)
    {
        params.push_back (OSCParameterInterface::createParameterTheOldWay ("filterType" + String(i), "Filter Type " + String(i+1), "",
                                        NormalisableRange<float> (0.0f, 3.0f, 1.0f),  filterTypePresets[i],
                                        [](float value) {
                                            if (value >= 0.5f && value < 1.5f) return "Low-pass";
                                            else if (value >= 1.5f && value < 2.5f) return "Band-pass";
                                            else if (value >= 2.5f) return "High-pass";
                                            else return "All-pass";},
                                        nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay ("filterFrequency" + String(i), "Filter Frequency " + String(i+1), "Hz",
                                        NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.4f), filterFrequencyPresets[i],
                                        [](float value) { return String((int) value); }, nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay ("filterQ" + String(i), "Filter Q " + String(i+1), "",
                                        NormalisableRange<float> (0.05f, 10.0f, 0.05f), 0.5f,
                                        [](float value) { return String(value, 2); },
                                        nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay ("filterGain" + String(i), "Filter Gain " + String(i+1), "dB",
                                        NormalisableRange<float> (-60.0f, 10.0f, 0.1f), 0.0f,
                                        [](float value) { return (value >= -59.9f) ? String(value, 1) : "-inf"; },
                                        nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay ("order" + String(i), "Order Band " + String(i+1), "",
                                        NormalisableRange<float> (0.0f, 7.0f, 0.01f), 0.0,
                                        [](float value) { return String(value, 2); }, nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay ("shape" + String(i), "Shape Band " + String(i+1), "",
                                        NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0,
                                        [](float value) { return String(value, 2); }, nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay ("azimuth" + String(i), "Azimuth Band " + String(i+1), CharPointer_UTF8 (R"(°)"),
                                        NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                        [](float value) { return String(value, 2); }, nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay ("elevation" + String(i), "Elevation Band " + String(i+1), CharPointer_UTF8 (R"(°)"),
                                        NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                        [](float value) {return String(value, 2);}, nullptr));
    }

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DirectivityShaperAudioProcessor();
}
