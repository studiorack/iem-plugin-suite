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
MatrixMultiplicatorAudioProcessor::MatrixMultiplicatorAudioProcessor()
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
//    parameters.createAndAddParameter("inputChannelsSetting", "Number of input channels ", "",
//                                     NormalisableRange<float> (0.0f, 10.0f, 1.0f), 0.0f,
//                                     [](float value) {return value < 0.5f ? "Auto" : String(value, 0);}, nullptr);
//     
//    parameters.createAndAddParameter("outputChannelsSetting", "Number of output channels ", "",
//                                     NormalisableRange<float> (0.0f, 10.0f, 1.0f), 0.0f,
//                                     [](float value) {return value < 0.5f ? "Auto" : String(value, 0);}, nullptr);
    
    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("MatrixMultiplicator"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened
    
    
    // get pointers to the parameters
//    inputChannelsSetting = parameters.getRawParameterValue("inputChannelsSetting");
//    outputChannelsSetting = parameters.getRawParameterValue ("outputChannelsSetting");


    
    // add listeners to parameter changes
//    parameters.addParameterListener ("inputChannelsSetting", this);
//    parameters.addParameterListener ("outputOrderSetting", this);

    
    PropertiesFile::Options options;
    options.applicationName     = "MatrixMultiplicator";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";
    
    properties = new PropertiesFile(options);
    lastDir = File(properties->getValue("presetFolder"));
}

MatrixMultiplicatorAudioProcessor::~MatrixMultiplicatorAudioProcessor()
{
    
}

void MatrixMultiplicatorAudioProcessor::setLastDir(File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue("presetFolder", v);
    
}

//==============================================================================
const String MatrixMultiplicatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MatrixMultiplicatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MatrixMultiplicatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MatrixMultiplicatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MatrixMultiplicatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MatrixMultiplicatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MatrixMultiplicatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MatrixMultiplicatorAudioProcessor::setCurrentProgram (int index)
{
}

const String MatrixMultiplicatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void MatrixMultiplicatorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MatrixMultiplicatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, 0, 0, true);
    
    ProcessSpec specs;
    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = samplesPerBlock;
    specs.numChannels = 64;
    
    matTrans.prepare(specs);
    
}

void MatrixMultiplicatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MatrixMultiplicatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void MatrixMultiplicatorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, 0, 0, false);
    ScopedNoDenormals noDenormals;
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    AudioBlock<float> ab (buffer);
    ProcessContextReplacing<float> context (ab);
    matTrans.process(context);
}

//==============================================================================
bool MatrixMultiplicatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MatrixMultiplicatorAudioProcessor::createEditor()
{
    return new MatrixMultiplicatorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void MatrixMultiplicatorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    parameters.state.setProperty("lastOpenedPresetFile", var(lastFile.getFullPathName()), nullptr);
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}



void MatrixMultiplicatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
void MatrixMultiplicatorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{    
    if (parameterID == "inputChannelsSetting" || parameterID == "outputOrderSetting" )
        userChangedIOSettings = true;
}

void MatrixMultiplicatorAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MatrixMultiplicatorAudioProcessor();
}


void MatrixMultiplicatorAudioProcessor::loadPreset(const File& presetFile)
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
        matTrans.setMatrix(tempMatrix);
        output += "Preset loaded succesfully!\n";
        output += "    Name: \t" + tempMatrix->getName() + "\n";
        output += "    Size: " + String(tempMatrix->getMatrix().getNumRows()) + "x" + String(tempMatrix->getMatrix().getNumColumns()) + " (output x input)\n";
        output += "    Description: \t" + tempMatrix->getDescription() + "\n";
    }
    else
        output = "ERROR: something went wrong!";

    currentMatrix = tempMatrix;
    messageForEditor = output;
    messageChanged = true;
}
