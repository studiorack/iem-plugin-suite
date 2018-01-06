/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://www.iem.at
 
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
DecoderAudioProcessor::DecoderAudioProcessor()
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
parameters(*this, nullptr), test("decodereName", "some text bla bla", 5, 5)
{
    
    
    parameters.createAndAddParameter ("inputOrderSetting", "Ambisonic Order", "",
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
    parameters.createAndAddParameter("useSN3D", "Normalization", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "SN3D";
                                         else return "N3D";
                                     }, nullptr);
    
    parameters.createAndAddParameter("outputChannelsSetting", "Number of input channels ", "",
                                     NormalisableRange<float> (0.0f, 10.0f, 1.0f), 0.0f,
                                     [](float value) {return value < 0.5f ? "Auto" : String(value);}, nullptr);
    
    parameters.createAndAddParameter("param1", "Parameter 1", "",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.1f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("param2", "Parameter 2", "dB",
                                     NormalisableRange<float> (-50.0f, 0.0f, 0.1f), -10.0,
                                     [](float value) {return String(value, 1);}, nullptr);
    
    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("Decoder"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened
    
    
    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue ("inputOrderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    
    outputChannelsSetting = parameters.getRawParameterValue("outputChannelsSetting");
    
    param1 = parameters.getRawParameterValue ("param1");
    param2 = parameters.getRawParameterValue ("param2");
    
    
    // add listeners to parameter changes
    parameters.addParameterListener ("inputChannelsSetting", this);
    parameters.addParameterListener ("outputOrderSetting", this);
    parameters.addParameterListener ("useSN3D", this);
    parameters.addParameterListener ("param1", this);
    parameters.addParameterListener ("param2", this);
    
    
    // global settings for all plug-in instances
    PropertiesFile::Options options;
    options.applicationName     = "Decoder";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";
    
    properties = new PropertiesFile(options);
    lastDir = File(properties->getValue("presetFolder"));
}

DecoderAudioProcessor::~DecoderAudioProcessor()
{
}

void DecoderAudioProcessor::setLastDir(File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue("presetFolder", v);
    
}


//==============================================================================
const String DecoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DecoderAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DecoderAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool DecoderAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double DecoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DecoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int DecoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DecoderAudioProcessor::setCurrentProgram (int index)
{
}

const String DecoderAudioProcessor::getProgramName (int index)
{
    return {};
}

void DecoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DecoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *inputOrderSetting, *outputChannelsSetting, true);
    //TODO: *outputChannelsSetting should be replaced by something like 'decoder.getOutputChannels()'
    
    
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    
    
}

void DecoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DecoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void DecoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputOrderSetting, *outputChannelsSetting, false);
    ScopedNoDenormals noDenormals;
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    
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
bool DecoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DecoderAudioProcessor::createEditor()
{
    return new DecoderAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void DecoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    parameters.state.setProperty("lastOpenedPresetFile", var(lastFile.getFullPathName()), nullptr);
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}



void DecoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.state = ValueTree::fromXml (*xmlState);
    if (parameters.state.hasProperty("lastOpenedPresetFile"))
    {
        Value val = parameters.state.getPropertyAsValue("lastOpenedPresetFile", nullptr);
        if (val.getValue().toString() != "")
        {
            const File f (val.getValue().toString());
            loadPreset(f);
        }
    }
}

//==============================================================================
void DecoderAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG("Parameter with ID " << parameterID << " has changed. New value: " << newValue);
    
    if (parameterID == "inputChannelsSetting" || parameterID == "outputOrderSetting" )
        userChangedIOSettings = true;
}

void DecoderAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}

void DecoderAudioProcessor::loadPreset(const File& presetFile)
{
    ReferenceCountedMatrix::Ptr tempMatrix = nullptr;
    
    Result result = DecoderHelper::parseFileForTransformationMatrix(presetFile, &tempMatrix);
    if (!result.wasOk()) {
        messageForEditor = result.getErrorMessage();
        return;
    }
    
    lastFile = presetFile;
    
    String output;
    if (tempMatrix != nullptr)
    {
        //matTrans.setMatrix(tempMatrix);
        output += "Preset loaded succesfully!\n";
        output += "    Name: \t" + tempMatrix->getName() + "\n";
        output += "    Size: " + String(tempMatrix->getMatrix()->rows()) + "x" + String(tempMatrix->getMatrix()->cols()) + " (output x input)\n";
        output += "    Description: \t" + tempMatrix->getDescription() + "\n";
    }
    else
        output = "ERROR: something went wrong!";
    
    messageForEditor = output;
    messageChanged = true;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DecoderAudioProcessor();
}


