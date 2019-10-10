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


//==============================================================================
ToolBoxAudioProcessor::ToolBoxAudioProcessor()
: AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
                       BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels(64), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
                     #endif
                       ,
#endif
createParameterLayout()), flipXMask (int64 (0)), flipYMask (int64 (0)), flipZMask (int64 (0))
{
    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue ("inputOrderSetting");
    outputOrderSetting = parameters.getRawParameterValue ("outputOrderSetting");
    useSn3dInput = parameters.getRawParameterValue ("useSn3dInput");
    useSn3dOutput = parameters.getRawParameterValue ("useSn3dOutput");
    flipX = parameters.getRawParameterValue ("flipX");
    flipY = parameters.getRawParameterValue ("flipY");
    flipZ = parameters.getRawParameterValue ("flipZ");
    loaWeights = parameters.getRawParameterValue ("loaWeights");
    gain = parameters.getRawParameterValue ("gain");


    // add listeners to parameter changes
    parameters.addParameterListener ("inputOrderSetting", this);
    parameters.addParameterListener ("outputOrderSetting", this);
    parameters.addParameterListener ("flipX", this);
    parameters.addParameterListener ("flipY", this);
    parameters.addParameterListener ("flipZ", this);


    // calculate flip masks


    for (int ch = 0; ch < 64; ++ch)
    {
        int l, m;
        ACNtoLM(ch, l, m);

        if (((m < 0) && (m % 2 == 0)) || ((m > 0) && (m % 2 == 1)))
            flipXMask.setBit(ch);

        if (m < 0)
            flipYMask.setBit(ch);

        if ((l + m) % 2)
            flipZMask.setBit(ch);
    }
}

ToolBoxAudioProcessor::~ToolBoxAudioProcessor()
{
}

//==============================================================================
int ToolBoxAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ToolBoxAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ToolBoxAudioProcessor::setCurrentProgram (int index)
{
}

const String ToolBoxAudioProcessor::getProgramName (int index)
{
    return {};
}

void ToolBoxAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ToolBoxAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *inputOrderSetting, *outputOrderSetting, true);

    doFlipX = *flipX >= 0.5f;
    doFlipY = *flipY >= 0.5f;
    doFlipZ = *flipZ >= 0.5f;

    calculateWeights (previousWeights, input.getNumberOfChannels(), output.getNumberOfChannels());
}

void ToolBoxAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


void ToolBoxAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputOrderSetting, *outputOrderSetting, false);
    ScopedNoDenormals noDenormals;

    const int nChIn = jmin (buffer.getNumChannels(), input.getNumberOfChannels());
    const int nChOut = jmin (buffer.getNumChannels(), output.getNumberOfChannels());
    const int nCh = jmin (nChIn, nChOut);

    const int L = buffer.getNumSamples();

    float weights[64];
    calculateWeights (weights, nChIn, nChOut);

    // apply weights;
    for (int ch = 0; ch < nCh; ++ch)
    {
        if (weights[ch] != previousWeights[ch])
        {
            buffer.applyGainRamp(ch, 0, L, previousWeights[ch], weights[ch]);
            previousWeights[ch] = weights[ch];
        }
        else if (weights[ch] != 1.0f)
        {
            FloatVectorOperations::multiply (buffer.getWritePointer(ch), weights[ch], L);
        }
    }

    // clear not used channels
    for (int ch = nCh; ch < buffer.getNumChannels(); ++ch)
    {
        buffer.clear (ch, 0, L);
        previousWeights[ch] = 0.0f;
    }

}

void ToolBoxAudioProcessor::calculateWeights (float *weights, const int nChannelsIn, const int nChannelsOut)
{
    const int nCh = jmin (nChannelsIn, nChannelsOut);
    const int orderIn = input.getOrder();
    const int orderOut = output.getOrder();

    FloatVectorOperations::fill (weights, Decibels::decibelsToGain (*gain), nCh);

    // create mask for all flips
    if (doFlipX || doFlipY || doFlipZ)
    {
        BigInteger mask (int64(0));
        if (doFlipX)
            mask ^= flipXMask;
        if (doFlipY)
            mask ^= flipYMask;
        if (doFlipZ)
            mask ^= flipZMask;

        for (int ch = 0; ch < nCh; ++ch)
            if (mask[ch])
                weights[ch] *= -1.0f;
    }


    // lower order ambisonics weighting
    if (orderIn < orderOut)
    {
        const int weightType = roundToInt (*loaWeights);
        if (weightType == 1) // maxrE
        {
            FloatVectorOperations::multiply (weights, getMaxRELUT (orderIn), nChannelsIn);
            const float* deWeights = getMaxRELUT (orderOut);
            for (int i = 0; i < nChannelsIn; ++i)
                weights[i] /= deWeights[i];
        }
        else if (weightType == 2) // inPhase
        {
            FloatVectorOperations::multiply (weights, getInPhaseLUT (orderIn), nChannelsIn);
            const float* deWeights = getInPhaseLUT (orderOut);
            for (int i = 0; i < nChannelsIn; ++i)
                weights[i] /= deWeights[i];
        }
    }


    // normalization
    const bool inSN3D = *useSn3dInput >= 0.5f;
    const bool outSN3D = *useSn3dOutput >= 0.5f;
    if (inSN3D != outSN3D)
    {
        if (inSN3D)
            FloatVectorOperations::multiply (weights, sn3d2n3d, nCh);
        else
            FloatVectorOperations::multiply (weights, n3d2sn3d, nCh);
    }
}

//==============================================================================
bool ToolBoxAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ToolBoxAudioProcessor::createEditor()
{
    return new ToolBoxAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void ToolBoxAudioProcessor::getStateInformation (MemoryBlock &destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ToolBoxAudioProcessor::setStateInformation (const void *data, int sizeInBytes)
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
void ToolBoxAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "inputOrderSetting" || parameterID == "outputOrderSetting" )
        userChangedIOSettings = true;
    else if (parameterID == "flipX")
        doFlipX = newValue >= 0.5f;
    else if (parameterID == "flipY")
        doFlipY = newValue >= 0.5f;
    else if (parameterID == "flipZ")
        doFlipZ = newValue >= 0.5f;

}

void ToolBoxAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}

//==============================================================================
std::vector<std::unique_ptr<RangedAudioParameter>> ToolBoxAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<RangedAudioParameter>> params;


    params.push_back (OSCParameterInterface::createParameterTheOldWay ("inputOrderSetting", "Input Ambisonic Order", "",
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

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("useSn3dInput", "Input Normalization", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "SN3D";
                                         else return "N3D";
                                     }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("outputOrderSetting", "Output Ambisonic Order", "",
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

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("useSn3dOutput", "Output Normalization", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "SN3D";
                                         else return "N3D";
                                     }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("flipX", "Flip X axis", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                     [](float value) {if (value >= 0.5f) return "ON";
                                         else return "OFF";}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("flipY", "Flip Y axis", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                     [](float value) {if (value >= 0.5f) return "ON";
                                         else return "OFF";}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("flipZ", "Flip Z axis", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                     [](float value) {if (value >= 0.5f) return "ON";
                                         else return "OFF";}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("loaWeights", "Lower Order Ambisonic Weighting", "",
                                     NormalisableRange<float> (0.0f, 2.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value >= 0.5f && value < 1.5f) return "maxrE";
                                         else if (value >= 1.5f) return "inPhase";
                                         else return "none";},
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("gain", "Gain", "dB",
                                                                       NormalisableRange<float> (-50.0f, 24.0f, 0.01f), 0.0f,
                                                                       [](float value) { return String (value, 2); }, nullptr));


    return params;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ToolBoxAudioProcessor();
}
