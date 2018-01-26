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
static constexpr float filterFrequencyPresets[] = {100.0f, 500.0f, 2000.0f, 4000.0f};

//==============================================================================
DirectivityShaperAudioProcessor::DirectivityShaperAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::mono(), true)
#endif
                  .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                  ),
#endif
parameters(*this, nullptr)
{
    parameters.createAndAddParameter ("orderSetting", "Input Order", "",
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
                                      nullptr);
    
    parameters.createAndAddParameter("masterYaw", "Master Yaw", "",
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("masterPitch", "Master Pitch", "",
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("masterRoll", "Master Roll", "",
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("masterToggle", "Lock Directions", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0,
                                     [](float value) {return (value >= 0.5f) ? "locked" : "not locked";}, nullptr);
    parameters.createAndAddParameter("normalization", "Directivity Normalization", "",
                                     NormalisableRange<float> (0.0f, 2.0f, 1.0f), 1.0,
                                     [](float value) {
                                         if (value >= 0.5f && value < 1.5f) return "on axis";
                                         else if (value >= 1.5f && value < 2.5f) return "constant energy";
                                         else return "basic decode";
                                     }, nullptr);
    
    for (int i = 0; i < numberOfBands; ++i)
    {
        parameters.createAndAddParameter("filterType" + String(i), "Filter Type " + String(i+1), "",
                                         NormalisableRange<float> (0.0f, 3.0f, 1.0f),  filterTypePresets[i],
                                         [](float value) {
                                             if (value >= 0.5f && value < 1.5f) return "Low-pass";
                                             else if (value >= 1.5f && value < 2.5f) return "Band-pass";
                                             else if (value >= 2.5f) return "High-pass";
                                             else return "All-pass";},
                                         nullptr);
        
        parameters.createAndAddParameter("filterFrequency" + String(i), "Filter Frequency " + String(i+1), "Hz",
                                         NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.4f), filterFrequencyPresets[i],
                                         [](float value) {return String(value);}, nullptr);
        parameters.createAndAddParameter("filterQ" + String(i), "Filter Q " + String(i+1), "",
                                         NormalisableRange<float> (0.05f, 10.0f, 0.05f), 0.707f,
                                         [](float value) {return (value >= -59.9f) ? String(value) : "-inf";},
                                         nullptr);
        parameters.createAndAddParameter("filterGain" + String(i), "Filter Gain " + String(i+1), "dB",
                                         NormalisableRange<float> (-60.0f, 10.0f, 0.1f), 0.0f,
                                         [](float value) {return (value >= -59.9f) ? String(value) : "-inf";},
                                         nullptr);
        parameters.createAndAddParameter("order" + String(i), "Order Band " + String(i+1), "",
                                         NormalisableRange<float> (0.0f, 7.0f, 0.01f), 0.0,
                                         [](float value) {return String(value);}, nullptr);
        parameters.createAndAddParameter("shape" + String(i), "Shape Band " + String(i+1), "",
                                         NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0,
                                         [](float value) {return String(value);}, nullptr);
        parameters.createAndAddParameter("yaw" + String(i), "Yaw Band " + String(i+1), "",
                                         NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                         [](float value) {return String(value);}, nullptr);
        parameters.createAndAddParameter("pitch" + String(i), "Pitch Band " + String(i+1), "",
                                         NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                         [](float value) {return String(value);}, nullptr);
    }
    
    
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    masterYaw = parameters.getRawParameterValue ("masterYaw");
    masterPitch = parameters.getRawParameterValue ("masterPitch");
    masterRoll = parameters.getRawParameterValue ("masterRoll");
    masterToggle = parameters.getRawParameterValue ("masterToggle");
    normalization = parameters.getRawParameterValue("normalization");
    
    parameters.addParameterListener("orderSetting", this);
    parameters.addParameterListener("masterToggle", this);
    parameters.addParameterListener("masterYaw", this);
    parameters.addParameterListener("masterPitch", this);
    parameters.addParameterListener("masterRoll", this);
    
    
    for (int i = 0; i < numberOfBands; ++i)
    {
        filterType[i] = parameters.getRawParameterValue ("filterType" + String(i));
        filterFrequency[i] = parameters.getRawParameterValue ("filterFrequency" + String(i));
        filterQ[i] = parameters.getRawParameterValue ("filterQ" + String(i));
        filterGain[i] = parameters.getRawParameterValue ("filterGain" + String(i));
        order[i] = parameters.getRawParameterValue ("order" + String(i));
        shape[i] = parameters.getRawParameterValue ("shape" + String(i));
        yaw[i] = parameters.getRawParameterValue ("yaw" + String(i));
        pitch[i] = parameters.getRawParameterValue ("pitch" + String(i));
        parameters.addParameterListener("filterType" + String(i), this);
        parameters.addParameterListener("filterFrequency" + String(i), this);
        parameters.addParameterListener("filterQ" + String(i), this);
        parameters.addParameterListener("filterGain" + String(i), this);
        parameters.addParameterListener("yaw" + String(i), this);
        parameters.addParameterListener("pitch" + String(i), this);
        
        probeGains[i] = 0.0f;
    }
    
    parameters.state = ValueTree (Identifier ("DirectivityShaper"));
    

    FloatVectorOperations::clear(shOld[0], 64 * numberOfBands);
    FloatVectorOperations::clear(weights[0], 8 * numberOfBands);
    
    
    for (int i = 0; i < numberOfBands; ++i)
    {
        filter[i].coefficients = createFilterCoefficients(roundToInt(*filterType[i]), 44100, *filterFrequency[i], *filterQ[i]);
    }
    
}

inline dsp::IIR::Coefficients<float>::Ptr DirectivityShaperAudioProcessor::createFilterCoefficients(int type, double sampleRate, double frequency, double Q)
{
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
const String DirectivityShaperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DirectivityShaperAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DirectivityShaperAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool DirectivityShaperAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double DirectivityShaperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

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
    
    filteredBuffer.setSize(numberOfBands, samplesPerBlock);
}

void DirectivityShaperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DirectivityShaperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void DirectivityShaperAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, 1, *orderSetting);
    ScopedNoDenormals noDenormals;    
    
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
    float masterSH[64];
    
    {
        Vector3D<float> pos = yawPitchToCartesian(degreesToRadians(*masterYaw), degreesToRadians(*masterPitch));
        SHEval(orderToWorkWith, pos.x, pos.y, pos.z, masterSH);
    }
    
    updateFv = true;
    
    for (int b = 0; b < numberOfBands; ++b)
    {
        float orderBlend, integer;
        orderBlend = modff(*order[b], &integer);
        int lowerOrder = roundToInt(integer);
        if (lowerOrder == 7) {
            lowerOrder = 6;
            orderBlend = 1.0f;
        }
        int higherOrder = lowerOrder + 1;
        
        float tempWeights[8];
        if (*shape[b]>=0.5f)
        {
            float blend = *shape[b] * 2.0f - 1.0f;
            for (int i=0; i<=orderToWorkWith; ++i)
            {
                tempWeights[i] = (1.0f-blend) *  maxRe[lowerOrder][i] + blend * inPhase[lowerOrder][i];
                tempWeights[i] *= (1.0f-orderBlend);
                tempWeights[i] += orderBlend * ((1.0f-blend) *  maxRe[higherOrder][i] + blend * inPhase[higherOrder][i]); ;
            }
        }
        else
        {
            float blend = *shape[b] * 2.0f;
            for (int i = 0; i <= orderToWorkWith; ++i)
            {
                tempWeights[i] = (1.0f-blend) *  basic[lowerOrder][i] + blend * maxRe[lowerOrder][i];
                tempWeights[i] *= (1.0f-orderBlend);
                tempWeights[i] += orderBlend * ((1.0f-blend) *  basic[higherOrder][i] + blend * maxRe[higherOrder][i]); ;
            }
        }
        
        float cor = 4.0f * M_PI / (higherOrder*higherOrder + (2 * higherOrder+1)*orderBlend);
        cor = cor/correction(orderToWorkWith)/correction(orderToWorkWith);
        
        if (*normalization >= 0.5f && *normalization < 1.5f)
        {
            float cor2 = 0.0f;
            for (int i = 0; i <= orderToWorkWith; ++i)
                cor2 += (2*i + 1)*tempWeights[i];
            float cor3;
            if (higherOrder > orderToWorkWith)
                cor3 = (squares[orderToWorkWith+1]);
            else
                cor3 = (squares[higherOrder] + (2 * higherOrder + 1)*orderBlend);

            cor2 = cor3/cor2;
            cor *= cor2;
        }
        
        if (*normalization >= 1.5f)
        {
            float sum = 0.0f;
            for (int i = 0; i <= orderToWorkWith; ++i)
                sum += tempWeights[i]  * tempWeights[i] * (2*i + 1);
            sum = 1.0f / sqrt(sum) * (orderToWorkWith + 1);
            
            for (int i = 0; i < 8; ++i )
                tempWeights[i] = tempWeights[i] * sum;
        }
        else
            for (int i = 0; i < 8; ++i )
                tempWeights[i] = tempWeights[i] * cor;

            
        Vector3D<float> pos = yawPitchToCartesian(degreesToRadians(*yaw[b]), degreesToRadians(*pitch[b]));
        SHEval(orderToWorkWith, pos.x, pos.y, pos.z, sh);
        
        float temp = 0.0f;
        float shTemp[64];
        FloatVectorOperations::multiply(shTemp, sh, Decibels::decibelsToGain(*filterGain[b]), 64);
        for (int i = 0; i < nChToWorkWith; ++i)
        {
            shTemp[i] *= tempWeights[isqrt(i)];
            temp += shTemp[i] * masterSH[i];
            buffer.addFromWithRamp(i, 0, filteredBuffer.getReadPointer(b), numSamples,shOld[b][i], shTemp[i]);
        }
        probeGains[b] = std::abs(temp);
        
        cor = correction(orderToWorkWith)/correction(7);
        cor *= cor;
        FloatVectorOperations::copy(shOld[b], shTemp, 64);
        for (int i = 0; i <= orderToWorkWith ; ++i)
            weights[b][i] = cor*tempWeights[i];
        for (int i = orderToWorkWith + 1; i < 8; ++i)
            weights[b][i] = 0.0f;
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
void DirectivityShaperAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DirectivityShaperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
void DirectivityShaperAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "orderSetting") userChangedIOSettings = true;
    else if (parameterID == "masterToggle")
    {
       if (newValue >= 0.5f && !toggled)
       {
        DBG("toggled");
        
        float ypr[3];
        ypr[2] = 0.0f;
        for (int i = 0; i < numberOfBands; ++i)
        {
            iem::Quaternion<float> masterQuat;
            float masterypr[3];
            masterypr[0] = degreesToRadians(*masterYaw);
            masterypr[1] = degreesToRadians(*masterPitch);
            masterypr[2] = degreesToRadians(*masterRoll);
            masterQuat.fromYPR(masterypr);
            masterQuat.conjugate();
            
            ypr[0] = degreesToRadians(*yaw[i]);
            ypr[1] = degreesToRadians(*pitch[i]);
            quats[i].fromYPR(ypr);
            quats[i] = masterQuat*quats[i];
        }
        toggled = true;
       }
        else if (newValue < 0.5f)
            toggled = false;
    }
    else if (toggled && ((parameterID == "masterYaw") ||  (parameterID == "masterPitch") ||  (parameterID == "masterRoll")))
    {
        DBG("moving");
        moving = true;
        iem::Quaternion<float> masterQuat;
        float ypr[3];
        ypr[0] = degreesToRadians(*masterYaw);
        ypr[1] = degreesToRadians(*masterPitch);
        ypr[2] = degreesToRadians(*masterRoll);
        masterQuat.fromYPR(ypr);
        
        for (int i = 0; i < numberOfBands; ++i)
        {
            iem::Quaternion<float> temp = masterQuat*quats[i];
            temp.toYPR(ypr);
            parameters.getParameterAsValue("yaw" + String(i)).setValue(radiansToDegrees(ypr[0]));
            parameters.getParameterAsValue("pitch" + String(i)).setValue(radiansToDegrees(ypr[1]));
        }
        moving = false;
    }
    else if (toggled && !moving &&  (parameterID.startsWith("yaw") || parameterID.startsWith("pitch")))
    {
        DBG("yawPitch");
        float ypr[3];
        ypr[2] = 0.0f;
        for (int i = 0; i < numberOfBands; ++i)
        {
            iem::Quaternion<float> masterQuat;
            float masterypr[3];
            masterypr[0] = degreesToRadians(*masterYaw);
            masterypr[1] = degreesToRadians(*masterPitch);
            masterypr[2] = degreesToRadians(*masterRoll);
            masterQuat.fromYPR(masterypr);
            masterQuat.conjugate();
            
            ypr[0] = degreesToRadians(*yaw[i]);
            ypr[1] = degreesToRadians(*pitch[i]);
            quats[i].fromYPR(ypr);
            quats[i] = masterQuat*quats[i];
        }
    }
    else if (parameterID.startsWith("filter"))
    {
        int i = parameterID.getLastCharacters(1).getIntValue();
        *filter[i].coefficients = *createFilterCoefficients(roundToInt(*filterType[i]), getSampleRate(), *filterFrequency[i], *filterQ[i]);
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DirectivityShaperAudioProcessor();
}

inline Vector3D<float> DirectivityShaperAudioProcessor::yawPitchToCartesian(float yawInRad, float pitchInRad) {
    float cosPitch = cos(pitchInRad);
    return Vector3D<float>(cosPitch * cos(yawInRad), cosPitch * sin(yawInRad), sin(-1.0f * pitchInRad));
}

inline Point<float> DirectivityShaperAudioProcessor::cartesianToYawPitch(Vector3D<float> pos) {
    float hypxy = sqrt(pos.x*pos.x+pos.y*pos.y);
    return Point<float>(atan2(pos.y,pos.x), atan2(hypxy,pos.z)-M_PI/2);
}
