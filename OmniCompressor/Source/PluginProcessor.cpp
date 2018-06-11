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
OmniCompressorAudioProcessor::OmniCompressorAudioProcessor()
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

    parameters.createAndAddParameter("threshold", "Threshold", "dB",
                                     NormalisableRange<float> (-50.0f, 10.0f, 0.1f), -10.0,
                                     [](float value) {return String(value, 1);}, nullptr);

    parameters.createAndAddParameter("knee", "Knee", "dB",
                                     NormalisableRange<float> (0.0f, 10.0f, 0.1f), 0.0f,
                                     [](float value) {return String(value, 1);}, nullptr);

    parameters.createAndAddParameter("attack", "Attack Time", "ms",
                                     NormalisableRange<float> (0.0f, 100.0f, 0.1f), 30.0,
                                     [](float value) {return String(value, 1);}, nullptr);

    parameters.createAndAddParameter("release", "Release Time", "ms",
                                     NormalisableRange<float> (0.0f, 500.0f, 0.1f), 150.0,
                                     [](float value) {return String(value, 1);}, nullptr);

    parameters.createAndAddParameter("ratio", "Ratio", " : 1",
                                     NormalisableRange<float> (1.0f, 16.0f, .2f), 4.0,
                                     [](float value) {
                                         if (value > 15.9f)
                                             return String("inf");
                                         return String(value, 1);

                                     }, nullptr);

    parameters.createAndAddParameter("outGain", "MakeUp Gain", "dB",
                                     NormalisableRange<float> (-10.0f, 20.0f, 0.1f), 0.0,
                                     [](float value) {return String(value, 1);}, nullptr);

    parameters.state = ValueTree (Identifier ("OmniCompressor"));

    parameters.addParameterListener("orderSetting", this);


    orderSetting = parameters.getRawParameterValue("orderSetting");
    threshold = parameters.getRawParameterValue ("threshold");
    knee = parameters.getRawParameterValue("knee");
    outGain = parameters.getRawParameterValue ("outGain");
    ratio = parameters.getRawParameterValue ("ratio");
    attack = parameters.getRawParameterValue ("attack");
    release = parameters.getRawParameterValue ("release");
    GR = 0.0f;
}


OmniCompressorAudioProcessor::~OmniCompressorAudioProcessor()
{

}

//==============================================================================
const String OmniCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OmniCompressorAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool OmniCompressorAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double OmniCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OmniCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int OmniCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OmniCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const String OmniCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void OmniCompressorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

void OmniCompressorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "orderSetting") userChangedIOSettings = true;
}

//==============================================================================
void OmniCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *orderSetting, *orderSetting, true);

    RMS.resize(samplesPerBlock);
    gains.resize(samplesPerBlock);
    allGR.resize(samplesPerBlock);

    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;
    spec.maximumBlockSize = samplesPerBlock;

    compressor.prepare(spec);
}

void OmniCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OmniCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void OmniCompressorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *orderSetting, *orderSetting);

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int bufferSize = buffer.getNumSamples();

    const int numCh = jmin(buffer.getNumChannels(), input.getNumberOfChannels(), output.getNumberOfChannels());
    //const int ambisonicOrder = jmin(input.getOrder(), output.getOrder());
    const float* bufferReadPtr = buffer.getReadPointer(0);

    if (*ratio > 15.9f)
        compressor.setRatio(INFINITY);
    else
        compressor.setRatio(*ratio);

    compressor.setKnee(*knee);

    compressor.setAttackTime(*attack * 0.001f);
    compressor.setReleaseTime(*release * 0.001f);
    compressor.setThreshold(*threshold);
    compressor.setMakeUpGain(*outGain);

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    compressor.getGainFromSidechainSignal(bufferReadPtr, gains.getRawDataPointer(), bufferSize);
    maxRMS = compressor.getMaxLevelInDecibels();

    maxGR = Decibels::gainToDecibels(FloatVectorOperations::findMinimum(gains.getRawDataPointer(), bufferSize)) - *outGain;

    for (int channel = 0; channel < numCh; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        FloatVectorOperations::multiply(channelData, gains.getRawDataPointer(), bufferSize);
    }
}

//==============================================================================
bool OmniCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* OmniCompressorAudioProcessor::createEditor()
{
    return new OmniCompressorAudioProcessorEditor (*this,parameters);
}

//==============================================================================
void OmniCompressorAudioProcessor::getStateInformation (MemoryBlock &destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void OmniCompressorAudioProcessor::setStateInformation (const void *data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (ValueTree::fromXml (*xmlState));
}

//==============================================================================
pointer_sized_int OmniCompressorAudioProcessor::handleVstPluginCanDo (int32 index,
                                                                     pointer_sized_int value, void* ptr, float opt)
{
    auto text = (const char*) ptr;
    auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

    if (matches ("wantsChannelCountNotifications"))
        return 1;
    return 0;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmniCompressorAudioProcessor();
}


