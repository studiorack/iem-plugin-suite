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
MatrixMultiplierAudioProcessor::MatrixMultiplierAudioProcessor()
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
    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("MatrixMultiplier"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened

    PropertiesFile::Options options;
    options.applicationName     = "MatrixMultiplier";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";

    properties = new PropertiesFile(options);
    lastDir = File(properties->getValue("configurationFolder"));
}

MatrixMultiplierAudioProcessor::~MatrixMultiplierAudioProcessor()
{

}

void MatrixMultiplierAudioProcessor::setLastDir(File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue("configurationFolder", v);

}

//==============================================================================
const String MatrixMultiplierAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MatrixMultiplierAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MatrixMultiplierAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MatrixMultiplierAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MatrixMultiplierAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MatrixMultiplierAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MatrixMultiplierAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MatrixMultiplierAudioProcessor::setCurrentProgram (int index)
{
}

const String MatrixMultiplierAudioProcessor::getProgramName (int index)
{
    return {};
}

void MatrixMultiplierAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MatrixMultiplierAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, 0, 0, true);

    ProcessSpec specs;
    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = samplesPerBlock;
    specs.numChannels = 64;

    matTrans.prepare(specs);

}

void MatrixMultiplierAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MatrixMultiplierAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void MatrixMultiplierAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, 0, 0, false);
    ScopedNoDenormals noDenormals;

    AudioBlock<float> ab (buffer);
    ProcessContextNonReplacing<float> context (ab, ab);
    matTrans.process(context);
}

//==============================================================================
bool MatrixMultiplierAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MatrixMultiplierAudioProcessor::createEditor()
{
    return new MatrixMultiplierAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void MatrixMultiplierAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    parameters.state.setProperty("lastOpenedConfigurationFile", var(lastFile.getFullPathName()), nullptr);
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}



void MatrixMultiplierAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.state = ValueTree::fromXml (*xmlState);
            if (parameters.state.hasProperty("lastOpenedConfigurationFile"))
            {
                Value val = parameters.state.getPropertyAsValue("lastOpenedConfigurationFile", nullptr);
                if (val.getValue().toString() != "")
                {
                    const File f (val.getValue().toString());
                    loadConfiguration(f);
                }
            }


        }
}

//==============================================================================
void MatrixMultiplierAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "inputChannelsSetting" || parameterID == "outputOrderSetting" )
        userChangedIOSettings = true;
}

void MatrixMultiplierAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MatrixMultiplierAudioProcessor();
}


void MatrixMultiplierAudioProcessor::loadConfiguration(const File& configurationFile)
{
    ReferenceCountedMatrix::Ptr tempMatrix = nullptr;

    Result result = DecoderHelper::parseFileForTransformationMatrix(configurationFile, &tempMatrix);
    if (!result.wasOk()) {
        messageForEditor = result.getErrorMessage();
        return;
    }

    lastFile = configurationFile;

    String output;
    if (tempMatrix != nullptr)
    {
        matTrans.setMatrix(tempMatrix);
        output += "Configuration loaded successfully!\n";
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
