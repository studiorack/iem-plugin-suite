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
MatrixTransformerAudioProcessor::MatrixTransformerAudioProcessor()
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
parameters(*this, nullptr)
{
    parameters.createAndAddParameter("inputChannelsSetting", "Number of input channels ", "",
                                     NormalisableRange<float> (0.0f, 10.0f, 1.0f), 0.0f,
                                     [](float value) {return value < 0.5f ? "Auto" : String(value, 0);}, nullptr);
     
    parameters.createAndAddParameter("outputChannelsSetting", "Number of output channels ", "",
                                     NormalisableRange<float> (0.0f, 10.0f, 1.0f), 0.0f,
                                     [](float value) {return value < 0.5f ? "Auto" : String(value, 0);}, nullptr);
    
    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("MatrixTransformer"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened
    
    
    // get pointers to the parameters
    inputChannelsSetting = parameters.getRawParameterValue("inputChannelsSetting");
    outputChannelsSetting = parameters.getRawParameterValue ("outputChannelsSetting");


    
    // add listeners to parameter changes
    parameters.addParameterListener ("inputChannelsSetting", this);
    parameters.addParameterListener ("outputOrderSetting", this);
    parameters.addParameterListener ("useSN3D", this);

    
    PropertiesFile::Options options;
    options.applicationName     = "MatrixTransformer";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";
    
    properties = new PropertiesFile(options);
    lastDir = File(properties->getValue("presetFolder"));
}

MatrixTransformerAudioProcessor::~MatrixTransformerAudioProcessor()
{
    
}

void MatrixTransformerAudioProcessor::setLastDir(File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue("presetFolder", v);
    
}

//==============================================================================
const String MatrixTransformerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MatrixTransformerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MatrixTransformerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MatrixTransformerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MatrixTransformerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MatrixTransformerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MatrixTransformerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MatrixTransformerAudioProcessor::setCurrentProgram (int index)
{
}

const String MatrixTransformerAudioProcessor::getProgramName (int index)
{
    return {};
}

void MatrixTransformerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MatrixTransformerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *inputChannelsSetting, *outputChannelsSetting, true);
    
    ProcessSpec specs;
    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = samplesPerBlock;
    specs.numChannels = 64;
    
    matTrans.prepare(specs);
    
}

void MatrixTransformerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MatrixTransformerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void MatrixTransformerAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputChannelsSetting, *outputChannelsSetting, false);
    ScopedNoDenormals noDenormals;
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    AudioBlock<float> ab (buffer);
    ProcessContextReplacing<float> context (ab);
    matTrans.process(context);
}

//==============================================================================
bool MatrixTransformerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MatrixTransformerAudioProcessor::createEditor()
{
    return new MatrixTransformerAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void MatrixTransformerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    parameters.state.setProperty("lastOpenedPresetFile", var(lastFile.getFullPathName()), nullptr);
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}



void MatrixTransformerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
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
}

//==============================================================================
void MatrixTransformerAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{    
    if (parameterID == "inputChannelsSetting" || parameterID == "outputOrderSetting" )
        userChangedIOSettings = true;
}

void MatrixTransformerAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MatrixTransformerAudioProcessor();
}


void MatrixTransformerAudioProcessor::loadPreset(const File& presetFile)
{
    ReferenceCountedMatrix::Ptr tempMatrix = nullptr;
    
    Result result = DecoderHelper::parseFileForMatrix(presetFile, &tempMatrix);
    if (!result.wasOk()) {
        messageForEditor = result.getErrorMessage();
        return;
    }

    lastFile = presetFile;
    
    String output;
    if (tempMatrix != nullptr)
    {
        matTrans.setMatrix(tempMatrix);
        output += "Preset loaded succesfully!\n";
        output += "    NAME: \t" + tempMatrix->getName() + "\n";
        output += "    INPUT SIZE: " + String(tempMatrix->getMatrix()->cols()) + "\n";
        output += "    OUTPUT SIZE: " + String(tempMatrix->getMatrix()->rows());
    }
    else
        output = "ERROR: something went wrong!";

    messageForEditor = output;
    messageChanged = true;
}
