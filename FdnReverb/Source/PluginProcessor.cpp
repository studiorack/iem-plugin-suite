/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Sebastian Grill
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
FdnReverbAudioProcessor::FdnReverbAudioProcessor()

#ifndef JucePlugin_PreferredChannelConfigurations
:AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                 .withInput  ("Input",  AudioChannelSet::discreteChannels(64), true)
#endif
                 .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                 ),
#endif

parameters (*this, nullptr), oscParams (parameters)
{
    oscParams.createAndAddParameter ("delayLength", "Room Size", "",
                                      NormalisableRange<float> (1.0f, 30.0f, 1.0f), 20.0f,
                                      [](float value) {return String (value, 0);},
                                      nullptr);
    oscParams.createAndAddParameter ("revTime", "Reverberation Time", "s",
                                      NormalisableRange<float> (0.1f, 9.0f, 0.1f), 5.f,
                                      [](float value) {return String (value, 1);},
                                      nullptr);

    oscParams.createAndAddParameter ("lowCutoff", "Lows Cutoff Frequency", "Hz",
                                      NormalisableRange<float> (20.f, 20000.f, 1.f, 0.2f), 100.f,
                                      [](float value) {return String (value, 0);},
                                      nullptr);
    oscParams.createAndAddParameter ("lowQ", "Lows Q Factor", "",
                                      NormalisableRange<float> (0.01f, 0.9f, 0.01f), 0.5f,
                                      [](float value) {return String (value, 2);},
                                      nullptr);
    oscParams.createAndAddParameter ("lowGain",
                                      "Lows Gain", "dB/s",
                                      NormalisableRange<float> (-80.0f, 6.0, 0.1f), 1.f,
                                      [](float value) {return String (value, 1);},
                                      nullptr);

    oscParams.createAndAddParameter ("highCutoff", "Highs Cutoff Frequency", "Hz",
                                      NormalisableRange<float> (20.f, 20000.f, 1.f, 0.2f), 2000.f,
                                      [](float value) {return String (value, 0);},
                                      nullptr);
    oscParams.createAndAddParameter ("highQ", "Highs Q Factor", "",
                                      NormalisableRange<float> (0.01f, 0.9f, 0.01f), 0.5f,
                                      [](float value) {return String (value, 2);},
                                      nullptr);
    oscParams.createAndAddParameter ("highGain",
                                      "Highs Gain", "dB/s",
                                      NormalisableRange<float> (-80.0f, 4.0f, 0.1f), -10.f,
                                      [](float value) {return String (value, 1);},
                                      nullptr);


//    oscParams.createAndAddParameter ("fdnSize",
//                                      "FDN size", "",
//                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
//                                      [](float value) {return value >= 0.5f ? "big" : "small";},
//                                      nullptr);
    oscParams.createAndAddParameter ("dryWet", "Dry/Wet", "",
                                      NormalisableRange<float> (0.f, 1.f, 0.01f), 0.5f,
                                      [](float value) {return String (value, 2);},
                                      nullptr);


    parameters.state = ValueTree (Identifier ("FdnReverb"));

    parameters.addParameterListener ("delayLength", this);
    parameters.addParameterListener ("revTime", this);
    parameters.addParameterListener ("highCutoff", this);
    parameters.addParameterListener ("highQ", this);
    parameters.addParameterListener ("highGain", this);
    parameters.addParameterListener ("lowCutoff", this);
    parameters.addParameterListener ("lowQ", this);
    parameters.addParameterListener ("lowGain", this);
//    parameters.addParameterListener ("fdnSize", this);
    parameters.addParameterListener ("dryWet", this);

    delayLength = parameters.getRawParameterValue ("delayLength");
    revTime = parameters.getRawParameterValue ("revTime");
    highCutoff = parameters.getRawParameterValue ("highCutoff");
    highQ = parameters.getRawParameterValue ("highQ");
    highGain = parameters.getRawParameterValue ("highGain");
    lowCutoff = parameters.getRawParameterValue ("lowCutoff");
    lowQ = parameters.getRawParameterValue ("lowQ");
    lowGain = parameters.getRawParameterValue ("lowGain");
//    fdnSize = parameters.getRawParameterValue("fdnSize");
    wet = parameters.getRawParameterValue("dryWet");

    fdn.setFdnSize(FeedbackDelayNetwork::big);

    oscReceiver.addListener (this);
}

FdnReverbAudioProcessor::~FdnReverbAudioProcessor()
{
}

//==============================================================================
const String FdnReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FdnReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FdnReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double FdnReverbAudioProcessor::getTailLengthSeconds() const
{
    return 10.0;
}

int FdnReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FdnReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FdnReverbAudioProcessor::setCurrentProgram (int index)
{
}

const String FdnReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void FdnReverbAudioProcessor::changeProgramName (int index, const String& newName)
{
}

void FdnReverbAudioProcessor::parameterChanged (const String & parameterID, float newValue)
{
    if (parameterID == "delayLength")
        fdn.setDelayLength (*delayLength);
    else if (parameterID == "revTime")
        fdn.setT60InSeconds (*revTime);
    else if (parameterID == "dryWet")
        fdn.setDryWet (*wet);
//    else if (parameterID == "fdnSize")
//        fdn.setFdnSize(*fdnSize >= 0.5f ? FeedbackDelayNetwork::big : FeedbackDelayNetwork::small);
    else
        {
            updateFilterParameters();
        }
}

void FdnReverbAudioProcessor::updateFilterParameters()
{
    FeedbackDelayNetwork::FilterParameter lowShelf;
    FeedbackDelayNetwork::FilterParameter highShelf;

    lowShelf.frequency = *lowCutoff;
    lowShelf.q = *lowQ;
    lowShelf.linearGain = Decibels::decibelsToGain(*lowGain);

    highShelf.frequency = *highCutoff;
    highShelf.q = *highQ;
    highShelf.linearGain = Decibels::decibelsToGain(*highGain);

    fdn.setFilterParameter (lowShelf, highShelf);
}
//==============================================================================
void FdnReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    updateFilterParameters();

    ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 64;
    fdn.prepare (spec);
    maxPossibleChannels = getTotalNumInputChannels();
}

//------------------------------------------------------------------------------
void FdnReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

//------------------------------------------------------------------------------
void FdnReverbAudioProcessor::reset()
{
    fdn.reset();
}

//------------------------------------------------------------------------------
#ifndef JucePlugin_PreferredChannelConfigurations
bool FdnReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

//------------------------------------------------------------------------------
void FdnReverbAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    dsp::AudioBlock<float> block (buffer);
    fdn.process (dsp::ProcessContextReplacing<float> (block));
}

//------------------------------------------------------------------------------
void FdnReverbAudioProcessor::setNetworkOrder (int order)
{
//    FeedbackDelayNetwork::FdnSize size;
//    if (order == 64)
//        size = FeedbackDelayNetwork::FdnSize::big;
//    else
//        size = FeedbackDelayNetwork::FdnSize::small;
//
//    fdn.setFdnSize (size);
}

//------------------------------------------------------------------------------
void FdnReverbAudioProcessor::setFreezeMode (bool freezeState)
{
    fdn.setFreeze (freezeState);
}

void FdnReverbAudioProcessor::getT60ForFrequencyArray (double* frequencies, double* t60Data, size_t numSamples)
{
        fdn.getT60ForFrequencyArray(frequencies, t60Data, numSamples);
}

//==============================================================================
bool FdnReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* FdnReverbAudioProcessor::createEditor()
{
    return new FdnReverbAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void FdnReverbAudioProcessor::getStateInformation (MemoryBlock &destData)
{
    auto state = parameters.copyState();
    state.setProperty ("OSCPort", var(oscReceiver.getPortNumber()), nullptr);
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FdnReverbAudioProcessor::setStateInformation (const void *data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort"))
            {
                oscReceiver.connect (parameters.state.getProperty ("OSCPort", var (-1)));
            }
        }
}


//==============================================================================
pointer_sized_int FdnReverbAudioProcessor::handleVstPluginCanDo (int32 index,
                                                                     pointer_sized_int value, void* ptr, float opt)
{
    auto text = (const char*) ptr;
    auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

    if (matches ("wantsChannelCountNotifications"))
        return 1;
    return 0;
}

//==============================================================================
void FdnReverbAudioProcessor::oscMessageReceived (const OSCMessage &message)
{
    OSCAddressPattern pattern ("/" + String(JucePlugin_Name) + "/*");
    if (! pattern.matches(OSCAddress(message.getAddressPattern().toString())))
        return;

    OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring(String(JucePlugin_Name).length() + 1));

    oscParams.processOSCMessage (msg);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FdnReverbAudioProcessor();
}
