/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 http://www.iem.at
 
 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
AmbisonicCompressorAudioProcessor::AmbisonicCompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::discreteChannels(64), true)
#endif
                  .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                  ),
#endif
parameters (*this, nullptr)
{
    parameters.createAndAddParameter("orderSetting", "Ambisonics Order", "",
                                     NormalisableRange<float>(0.0f, 8.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value >= 0.5f && value < 1.5f) return "0th";
                                         else if (value >= 1.5f && value < 2.5f) return "1st";
                                         else if (value >= 2.5f && value < 3.5f) return "2nd";
                                         else if (value >= 3.5f && value < 4.5f) return "3rd";
                                         else if (value >= 4.5f && value < 5.5f) return "4th";
                                         else if (value >= 5.5f && value < 6.5f) return "5th";
                                         else if (value >= 6.5f && value < 7.5f) return "6th";
                                         else if (value >= 7.5f) return "7th";
                                         else return "Auto";
                                     }, nullptr);
    
    parameters.createAndAddParameter("useSN3D", "Normalization", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "SN3D";
                                         else return "N3D";
                                     }, nullptr);
    
    parameters.createAndAddParameter("inGain", "Input Gain ", "dB",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.1f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("threshold", "Threshold", "dB",
                                     NormalisableRange<float> (-50.0f, 0.0f, 0.1f), -10.0,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("attack", "Attack Time", "ms",
                                     NormalisableRange<float> (1.0f, 100.0f, 0.1f), 30.0,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("release", "Release Time", "ms",
                                     NormalisableRange<float> (1.0f, 500.0f, 0.1f), 150.0,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("ratio", "Ratio", "",
                                     NormalisableRange<float> (1.0f, 10.0f, .5f), 4.0,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("outGain", "MakeUp Gain", "dB",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.10f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.state = ValueTree (Identifier ("OmniCompressor"));
    
    parameters.addParameterListener("orderSetting", this);
    
    orderSetting = parameters.getRawParameterValue("orderSetting");
    inGain = parameters.getRawParameterValue ("inGain");
    threshold = parameters.getRawParameterValue ("threshold");
    outGain = parameters.getRawParameterValue ("outGain");
    ratio = parameters.getRawParameterValue ("ratio");
    attack = parameters.getRawParameterValue ("attack");
    release = parameters.getRawParameterValue ("release");
    GR = 0.0f;
}


AmbisonicCompressorAudioProcessor::~AmbisonicCompressorAudioProcessor()
{

}

//==============================================================================
const String AmbisonicCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AmbisonicCompressorAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AmbisonicCompressorAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double AmbisonicCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AmbisonicCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AmbisonicCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmbisonicCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const String AmbisonicCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void AmbisonicCompressorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

void AmbisonicCompressorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "orderSetting") userChangedIOSettings = true;
}

//==============================================================================
void AmbisonicCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *orderSetting, *orderSetting, true);
    
    RMS.resize(samplesPerBlock);
    gains.resize(samplesPerBlock);
    allGR.resize(samplesPerBlock);
    
    double A = exp(-1.0/(sampleRate*0.01)); //multiplicated value after sampleRate is rms time
    meanSqrFilter.setCoefficients(juce::IIRCoefficients(1.0 - A, 0.0, 0.0, 1.0, - A, 0.0));
}

void AmbisonicCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AmbisonicCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void AmbisonicCompressorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *orderSetting, *orderSetting);
    
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int sampleRate = this->getSampleRate();
    const int bufferSize = buffer.getNumSamples();
    
    
    const int numCh = jmin(buffer.getNumChannels(), input.getNumberOfChannels(), output.getNumberOfChannels());
    const int ambisonicOrder = jmin(input.getOrder(), output.getOrder());
    const float* bufferReadPtr = buffer.getReadPointer(0);

    
    FloatVectorOperations::multiply(RMS.getRawDataPointer(), bufferReadPtr, bufferReadPtr, bufferSize);
    meanSqrFilter.processSamples(RMS.getRawDataPointer(), bufferSize);
    for (int i = 0; i<bufferSize; ++i) RMS.setUnchecked(i, Decibels::gainToDecibels(ambisonicOrder+1.0f) + 0.5f * Decibels::gainToDecibels(RMS[i],-120.0f) + *inGain);
    //maxRMS = FloatVectorOperations::findMaximum(RMS.getRawDataPointer(), bufferSize);
    
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
    float attackGain = expf(-1000.0/sampleRate / *attack);
    float releaseGain = expf(-1000.0/sampleRate / *release);
    float ratioFactor = -1.0f * (1 - 1/ *ratio);
    maxRMS = FloatVectorOperations::findMaximum(RMS.getRawDataPointer(), bufferSize);
    
    FloatVectorOperations::add(RMS.getRawDataPointer(), -1.0 * *threshold, bufferSize);

    float sumGain = *outGain + *inGain;
    float envSample = 0;
    for (int i = 0; i<bufferSize; ++i)
    {
        envSample = RMS[i]>0.0 ? RMS[i] : 0.0f;
        if (GR < envSample) {
            //GR = envSample + attackGain * (GR - envSample);
            GR = envSample + attackGain * (GR - envSample);
        } else {
            GR = envSample + releaseGain * (GR - envSample);
        }
        allGR.set(i, GR * ratioFactor);
        gains.set(i, Decibels::decibelsToGain(allGR[i] + sumGain));
        
    }
    maxGR = FloatVectorOperations::findMaximum(allGR.getRawDataPointer(), bufferSize);
    
    
    

    for (int channel = 0; channel < numCh; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        FloatVectorOperations::multiply(channelData, gains.getRawDataPointer(), bufferSize);
    }
}

//==============================================================================
bool AmbisonicCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AmbisonicCompressorAudioProcessor::createEditor()
{
    return new AmbisonicCompressorAudioProcessorEditor (*this,parameters);
}

//==============================================================================
void AmbisonicCompressorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AmbisonicCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmbisonicCompressorAudioProcessor();
}

