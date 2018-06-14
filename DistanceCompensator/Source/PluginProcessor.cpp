/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
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
DistanceCompensatorAudioProcessor::DistanceCompensatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels(10), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
                     #endif
                       ),
#endif
parameters(*this, nullptr)
{
    parameters.createAndAddParameter ("inputChannelsSetting", "Number of input channels ", "",
                                     NormalisableRange<float> (0.0f, 64.0f, 1.0f), 0.0f,
                                     [](float value) {return value < 0.5f ? "Auto" : String(value);}, nullptr);

    parameters.createAndAddParameter ("enableGains", "Enable Gain", "",
                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                      [](float value) {
                                          if (value >= 0.5f) return "Yes";
                                          else return "No";
                                      }, nullptr);

    parameters.createAndAddParameter ("enableDelays", "Enable Delay", "",
                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                      [](float value) {
                                          if (value >= 0.5f) return "Yes";
                                          else return "No";
                                      }, nullptr);

    parameters.createAndAddParameter ("speedOfSound", "Speed of Sound", "m/s",
                                     NormalisableRange<float> (330.0, 350.0, 0.1f), 334.2f,
                                     [](float value) {return String(value);}, nullptr);

    for (int i = 0; i < 64; ++i)
    {
        parameters.createAndAddParameter("enableCompensation" + String(i), "Enable Compensation of loudspeaker " + String(i + 1), "",
                                         NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                         [](float value) {
                                             if (value >= 0.5f) return "ON";
                                             else return "OFF";
                                         }, nullptr);

        parameters.createAndAddParameter("distance" + String(i), "Distance of loudspeaker " + String(i + 1), "m",
                                         NormalisableRange<float> (1.0f, 50.0f, 0.01f), 1.0f,
                                         [](float value) {return String(value, 2);}, nullptr);
    }

    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("DistanceCompensator"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened


    // get pointers to the parameters
    inputChannelsSetting = parameters.getRawParameterValue ("inputChannelsSetting");
    speedOfSound = parameters.getRawParameterValue ("speedOfSound");
    


    // add listeners to parameter changes
    parameters.addParameterListener ("inputChannelsSetting", this);
    parameters.addParameterListener ("speedOfSound", this);

    for (int i = 0; i < 64; ++i)
    {
        enableCompensation[i] = parameters.getRawParameterValue ("enableCompensation" + String(i));
        parameters.addParameterListener ("enableCompensation" + String(i), this);

        distance[i] = parameters.getRawParameterValue ("distance" + String(i));
        parameters.addParameterListener ("distance" + String(i), this);
    }


    // global properties
    PropertiesFile::Options options;
    options.applicationName     = "DistanceCompensator";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";

    properties = new PropertiesFile (options);
    lastDir = File (properties->getValue("presetFolder"));

    tempValues.resize(64);
}

DistanceCompensatorAudioProcessor::~DistanceCompensatorAudioProcessor()
{
}


float DistanceCompensatorAudioProcessor::distanceToGainInDecibels (const float distance)
{
    jassert(distance >= 1.0f);
    const float gainInDecibels = Decibels::gainToDecibels (1.0f / distance);
    return gainInDecibels;
}

float DistanceCompensatorAudioProcessor::distanceToDelayInSeconds (const float distance)
{
    jassert(distance >= 1.0f);
    const float delayInSeconds = distance / *speedOfSound;
    return delayInSeconds;
}

void DistanceCompensatorAudioProcessor::setLastDir(File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue ("presetFolder", v);
}

void DistanceCompensatorAudioProcessor::loadConfiguration (const File& configFile)
{
    ValueTree loudspeakers {"Loudspeakers"};

    Result result = DecoderHelper::parseFileForLoudspeakerLayout (configFile, loudspeakers, nullptr);
    if (!result.wasOk())
    {
        DBG("Configuration could not be loaded.");
        MailBox::Message newMessage;
        newMessage.messageColour = Colours::red;
        newMessage.headline = "Error loading configuration";
        newMessage.text = result.getErrorMessage() ;
        messageToEditor = newMessage;
        updateMessage = true;
    }
}

//==============================================================================
const String DistanceCompensatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistanceCompensatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DistanceCompensatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DistanceCompensatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DistanceCompensatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DistanceCompensatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DistanceCompensatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DistanceCompensatorAudioProcessor::setCurrentProgram (int index)
{
}

const String DistanceCompensatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void DistanceCompensatorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DistanceCompensatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *inputChannelsSetting, 0, true);

    ProcessSpec specs;
    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = samplesPerBlock;
    specs.numChannels = 64;

    gain.prepare (specs);
    delay.prepare (specs);

    for (int i = 0; i < 64; ++i)
    {
        gain.setGainDecibels (i, - 1.0f * distanceToGainInDecibels (*distance[i]));
        delay.setDelayTime (i, distanceToDelayInSeconds (*distance[i]));
    }

}

void DistanceCompensatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistanceCompensatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void DistanceCompensatorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputChannelsSetting, 0, false);
    ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    AudioBlock<float> ab (buffer);
    ProcessContextReplacing<float> context (ab);

    //gain.process (context);
    delay.process (context);
}

//==============================================================================
bool DistanceCompensatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DistanceCompensatorAudioProcessor::createEditor()
{
    return new DistanceCompensatorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void DistanceCompensatorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}



void DistanceCompensatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.state = ValueTree::fromXml (*xmlState);
}

//==============================================================================
void DistanceCompensatorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "inputChannelsSetting" || parameterID == "outputOrderSetting" )
        userChangedIOSettings = true;

    if (parameterID.startsWith("distance"))
    {
        const int id = parameterID.substring(8).getIntValue();

        updateDelays();
        updateGains();
    }

    else if (parameterID.startsWith("enableCompensation"))
    {
        updateDelays();
        updateGains();
    }
}

void DistanceCompensatorAudioProcessor::updateDelays()
{
    tempValues.clear();


    for (int i = 0; i < 64; ++i)
    {
        if (*enableCompensation[i] >= 0.5f)
            tempValues.add (distanceToDelayInSeconds (*distance[i]));
    }
    const int nActive = tempValues.size();
    DBG("aktiv: " << nActive);
    const float maxDelay = FloatVectorOperations::findMaximum (tempValues.getRawDataPointer(), nActive);
    DBG("max : " << maxDelay);
    const float minDelay = FloatVectorOperations::findMinimum (tempValues.getRawDataPointer(), nActive);
    DBG("max : " << minDelay);

    for (int i = 0; i < 64; ++i)
    {
        if (*enableCompensation[i] >= 0.5f)
            delay.setDelayTime(i, maxDelay - distanceToDelayInSeconds (*distance[i]));
        else
            delay.setDelayTime(i, 0.0f);
    }
}

void DistanceCompensatorAudioProcessor::updateGains()
{
    tempValues.clear();

    for (int i = 0; i < 64; ++i)
    {
        if (*enableCompensation[i] >= 0.5f)
            tempValues.add (distanceToGainInDecibels (*distance[i]));
    }
    const int nActive = tempValues.size();
    const float maxGain = FloatVectorOperations::findMaximum (tempValues.getRawDataPointer(), nActive);
    const float minGain = FloatVectorOperations::findMinimum (tempValues.getRawDataPointer(), nActive);
    const float mean = 0.5f * (maxGain + minGain);


    for (int i = 0; i < 64; ++i)
    {
        if (*enableCompensation[i] >= 0.5f)
            gain.setGainDecibels (i, mean - distanceToGainInDecibels (*distance[i]));
        else
            gain.setGainDecibels (i, 0.0f);
    }
}

void DistanceCompensatorAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());    
}

//==============================================================================
pointer_sized_int DistanceCompensatorAudioProcessor::handleVstPluginCanDo (int32 index,
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
    return new DistanceCompensatorAudioProcessor();
}
