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
 : AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
                       BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels(64), true)
#endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                       ,
#endif
createParameterLayout())
{
    PropertiesFile::Options options;
    options.applicationName     = "MatrixMultiplier";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";

    properties.reset (new PropertiesFile(options));
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


void MatrixMultiplierAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, 0, 0, false);
    ScopedNoDenormals noDenormals;

    AudioBlock<float> ab (buffer);
    matTrans.processReplacing (ab);
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
    auto state = parameters.copyState();

    state.setProperty("lastOpenedConfigurationFile", var(lastFile.getFullPathName()), nullptr);

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}



void MatrixMultiplierAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("lastOpenedConfigurationFile"))
            {
                Value val = parameters.state.getPropertyAsValue ("lastOpenedConfigurationFile", nullptr);
                if (val.getValue().toString() != "")
                {
                    const File f (val.getValue().toString());
                    loadConfiguration (f);
                }
            }
            if (parameters.state.hasProperty ("OSCPort")) // legacy
            {
                oscParameterInterface.getOSCReceiver().connect (parameters.state.getProperty ("OSCPort", var (-1)));
                parameters.state.removeProperty ("OSCPort", nullptr);
            }

            auto oscConfig = parameters.state.getChildWithName ("OSCConfig");
            if (oscConfig.isValid())
                oscParameterInterface.setConfig (oscConfig);
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

    Result result = ConfigurationHelper::parseFileForTransformationMatrix(configurationFile, &tempMatrix);
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


//==============================================================================
const bool MatrixMultiplierAudioProcessor::processNotYetConsumedOSCMessage (const OSCMessage &message)
{
    if (message.getAddressPattern().toString().equalsIgnoreCase ("/" + String (JucePlugin_Name) + "/loadFile") && message.size() >= 1)
    {
        if (message[0].isString())
        {
            File fileToLoad (message[0].getString());
            loadConfiguration (fileToLoad);
            return true;
        }
    }

    return false;
}



//==============================================================================
std::vector<std::unique_ptr<RangedAudioParameter>> MatrixMultiplierAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    return params;
}
