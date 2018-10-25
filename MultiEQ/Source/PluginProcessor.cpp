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
MultiEQAudioProcessor::MultiEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels (10), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels (64), true)
                     #endif
                       ),
#endif
parameters (*this, nullptr), oscParams (parameters)
{
    oscParams.createAndAddParameter ("inputChannelsSetting", "Number of input channels ", "",
                                     NormalisableRange<float> (0.0f, 10.0f, 1.0f), 0.0f,
                                     [](float value) {return value < 0.5f ? "Auto" : String (value);}, nullptr);




    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("MultiEQ"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened


    // get pointers to the parameters
    inputChannelsSetting = parameters.getRawParameterValue ("inputChannelsSetting");



    // add listeners to parameter changes
    parameters.addParameterListener ("inputChannelsSetting", this);


    oscReceiver.addListener (this);
}

MultiEQAudioProcessor::~MultiEQAudioProcessor()
{
}

//==============================================================================
const String MultiEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultiEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultiEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultiEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MultiEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultiEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiEQAudioProcessor::setCurrentProgram (int index)
{
}

const String MultiEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultiEQAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MultiEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *inputChannelsSetting, *inputChannelsSetting, true);

    // Use this method as the place to do any pre-playback
    // initialisation that you need..



}

void MultiEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void MultiEQAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *inputChannelsSetting, *inputChannelsSetting, false);
    ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool MultiEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MultiEQAudioProcessor::createEditor()
{
    return new MultiEQAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void MultiEQAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    state.setProperty ("OSCPort", var (oscReceiver.getPortNumber()), nullptr);
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}


void MultiEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
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
void MultiEQAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG ("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "inputChannelsSetting")
        userChangedIOSettings = true;
}

void MultiEQAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());
}

//==============================================================================
pointer_sized_int MultiEQAudioProcessor::handleVstPluginCanDo (int32 index,
                                                                     pointer_sized_int value, void* ptr, float opt)
{
    auto text = (const char*) ptr;
    auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

    if (matches ("wantsChannelCountNotifications"))
        return 1;
    return 0;
}

//==============================================================================
void MultiEQAudioProcessor::oscMessageReceived (const OSCMessage &message)
{
    String prefix ("/" + String (JucePlugin_Name));
    if (! message.getAddressPattern().toString().startsWith (prefix))
        return;

    OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring (String (JucePlugin_Name).length() + 1));

    oscParams.processOSCMessage (msg);
}

void MultiEQAudioProcessor::oscBundleReceived (const OSCBundle &bundle)
{
    for (int i = 0; i < bundle.size(); ++i)
    {
        auto elem = bundle[i];
        if (elem.isMessage())
            oscMessageReceived (elem.getMessage());
        else if (elem.isBundle())
            oscBundleReceived (elem.getBundle());
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiEQAudioProcessor();
}
