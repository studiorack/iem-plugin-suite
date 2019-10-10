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
: AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
                      BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::discreteChannels(10), true)
#endif
                  .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                  ,
#endif
createParameterLayout())
{
    // get pointers to the parameters
    inputChannelsSetting = parameters.getRawParameterValue ("inputChannelsSetting");
    enableGains = parameters.getRawParameterValue ("enableGains");
    enableDelays = parameters.getRawParameterValue ("enableDelays");
    speedOfSound = parameters.getRawParameterValue ("speedOfSound");
    distanceExponent = parameters.getRawParameterValue ("distanceExponent");
    gainNormalization = parameters.getRawParameterValue ("gainNormalization");
    referenceX = parameters.getRawParameterValue ("referenceX");
    referenceY = parameters.getRawParameterValue ("referenceY");
    referenceZ = parameters.getRawParameterValue ("referenceZ");



    // add listeners to parameter changes
    parameters.addParameterListener ("inputChannelsSetting", this);
    parameters.addParameterListener ("speedOfSound", this);
    parameters.addParameterListener ("distanceExponent", this);
    parameters.addParameterListener ("gainNormalization", this);

    for (int i = 0; i < 64; ++i)
    {
        enableCompensation[i] = parameters.getRawParameterValue ("enableCompensation" + String (i));
        parameters.addParameterListener ("enableCompensation" + String (i), this);

        distance[i] = parameters.getRawParameterValue ("distance" + String (i));
        parameters.addParameterListener ("distance" + String (i), this);
    }

    // global properties
    PropertiesFile::Options options;
    options.applicationName     = "DistanceCompensator";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";

    properties.reset (new PropertiesFile (options));
    lastDir = File (properties->getValue ("presetFolder"));

    tempValues.resize(64);
}

DistanceCompensatorAudioProcessor::~DistanceCompensatorAudioProcessor()
{
}


float DistanceCompensatorAudioProcessor::distanceToGainInDecibels (const float distance)
{
    jassert(distance >= 1.0f);
    const float gainInDecibels = Decibels::gainToDecibels (1.0f / pow(distance, *distanceExponent));
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

    Result result = ConfigurationHelper::parseFileForLoudspeakerLayout (configFile, loudspeakers, nullptr);
    if (!result.wasOk())
    {
        DBG("Configuration could not be loaded.");
        MailBox::Message newMessage;
        newMessage.messageColour = Colours::red;
        newMessage.headline = "Error loading configuration";
        newMessage.text = result.getErrorMessage();
        messageToEditor = newMessage;
        updateMessage = true;
        DBG(result.getErrorMessage());
    }
    else
    {
        const int nLsp = loudspeakers.getNumChildren();

        loadedLoudspeakerPositions.clear();
        int maxChannel = 0;
        for (int i = 0; i < nLsp; ++i)
        {
            auto lsp = loudspeakers.getChild(i);
            const bool isImaginary = lsp.getProperty("Imaginary");
            if (isImaginary)
                continue;

            const float radius = lsp.getProperty("Radius");
            const float azimuth = lsp.getProperty("Azimuth");
            const float elevation = lsp.getProperty("Elevation");
            const int channel = lsp.getProperty("Channel");

            const Vector3D<float> cart = Conversions<float>::sphericalToCartesian(degreesToRadians(azimuth), degreesToRadians(elevation), radius);
            loadedLoudspeakerPositions.add({cart, channel});
            if (channel > maxChannel)
                maxChannel = channel;
        }

        DBG("num lsps: " << loadedLoudspeakerPositions.size());
        if (maxChannel > 0)
            parameters.getParameter("inputChannelsSetting")->setValueNotifyingHost (parameters.getParameterRange("inputChannelsSetting" ).convertTo0to1(maxChannel));
        updateParameters();
    }
}

//==============================================================================
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

    updateDelays();
    updateGains();
}

void DistanceCompensatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

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

    if (*enableGains > 0.5f)
        gain.process (context);
    if (*enableDelays > 0.5f)
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
  auto state = parameters.copyState();

  auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
  oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

  std::unique_ptr<XmlElement> xml (state.createXml());
  copyXmlToBinary (*xml, destData);
}



void DistanceCompensatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.state = ValueTree::fromXml (*xmlState);
            loadedLoudspeakerPositions.clear();
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
void DistanceCompensatorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "inputChannelsSetting")
    {
        userChangedIOSettings = true;
    }
    else if (parameterID == "speedOfSound")
    {
        updateDelays();
    }
    else if (parameterID == "distanceExponent")
    {
        updateGains();
    }
    else if (parameterID == "gainNormalization")
    {
        updateGains();
    }
    else if (parameterID.startsWith("distance"))
    {
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
    if (updatingParameters.get())
        return;

    tempValues.clearQuick();

    const int selected = roundToInt(*inputChannelsSetting);
    int nCh;
    if (selected > 0)
        nCh = selected;
    else
        nCh = input.getSize();

    for (int i = 0; i < nCh; ++i)
    {
        if (*enableCompensation[i] >= 0.5f)
            tempValues.add (distanceToDelayInSeconds (*distance[i]));
    }

    const int nActive = tempValues.size();
    const float maxDelay = FloatVectorOperations::findMaximum (tempValues.getRawDataPointer(), nActive);

    for (int i = 0; i < nCh; ++i)
    {
        if (*enableCompensation[i] >= 0.5f)
            delay.setDelayTime(i, maxDelay - distanceToDelayInSeconds (*distance[i]));
        else
            delay.setDelayTime(i, 0.0f);
    }
}

void DistanceCompensatorAudioProcessor::updateGains()
{
    if (updatingParameters.get())
        return;
    tempValues.clearQuick();


    const int selected = roundToInt(*inputChannelsSetting);
    int nCh;
    if (selected > 0)
        nCh = selected;
    else
        nCh = input.getSize();

    for (int i = 0; i < nCh; ++i)
    {
        if (*enableCompensation[i] >= 0.5f)
            tempValues.add (distanceToGainInDecibels (*distance[i]));
    }
    const int nActive = tempValues.size();

    const float minGain = FloatVectorOperations::findMinimum (tempValues.getRawDataPointer(), nActive);

    float ref = 0.0f;
    if (*gainNormalization >= 0.5f) // zero mean
    {
        for (int i = 0; i < nActive; ++i)
            ref += tempValues.getUnchecked(i);
        ref /= nActive;
    }
    else
    {
        ref = minGain;
    }


    DBG("Gain ref: " << ref);

    for (int i = 0; i < nCh; ++i)
    {
        if (*enableCompensation[i] >= 0.5f)
        {
            gain.setGainDecibels (i, ref - distanceToGainInDecibels (*distance[i]));
            DBG(i << ": " << ref - distanceToGainInDecibels (*distance[i]));
        }
        else
            gain.setGainDecibels (i, 0.0f);
    }
}

void DistanceCompensatorAudioProcessor::updateParameters()
{
    const int nLsp = loadedLoudspeakerPositions.size();
    if (nLsp == 0)
    {
        DBG("No loudspeakers loaded.");
        MailBox::Message newMessage;
        newMessage.messageColour = Colours::red;
        newMessage.headline = "Can't update reference position.";
        newMessage.text = "The reference position can only be updated, if a loudspeaker layout has been loaded. An already loaded layout will vanish every time the session is reloaded.";
        messageToEditor = newMessage;
        updateMessage = true;
        return;
    }

    updatingParameters = true;

    for (int i = 0; i < 64; ++i)
    {
        parameters.getParameter ("enableCompensation" + String (i))->setValueNotifyingHost (0.0f);
        parameters.getParameter ("distance" + String (i))->setValueNotifyingHost (0.0f);
    }

    const Vector3D<float> reference {*referenceX, *referenceY, *referenceZ};

    for (int i = 0; i < nLsp; ++i)
    {
        auto lsp = loadedLoudspeakerPositions.getReference(i);
        const float radius = jmax(1.0f, (lsp.position - reference).length());

        parameters.getParameter ("enableCompensation" + String (lsp.channel - 1))->setValueNotifyingHost (1.0f);
        parameters.getParameter ("distance" + String (lsp.channel - 1))->setValueNotifyingHost (parameters.getParameterRange ("distance" + String (lsp.channel - 1)).convertTo0to1 (radius));
    }


    updatingParameters = false;

    updateDelays();
    updateGains();
}

void DistanceCompensatorAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());

    updateDelays();
    updateGains();
}


//==============================================================================
const bool DistanceCompensatorAudioProcessor::processNotYetConsumedOSCMessage (const OSCMessage &message)
{
    String prefix ("/" + String (JucePlugin_Name));
    if (! message.getAddressPattern().toString().startsWith (prefix))
        return false;

    OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring (String (JucePlugin_Name).length() + 1));

    if (msg.getAddressPattern().toString ().equalsIgnoreCase ("/loadFile") && msg.size() >= 1)
    {
        if (msg[0].isString ())
        {
            File fileToLoad (msg[0].getString ());
            loadConfiguration (fileToLoad);
        }
        return true;
    }
    else if (msg.getAddressPattern().toString ().equalsIgnoreCase ("/updateReference"))
    {
        updateParameters();
        return true;
    }

    return false;
}



//==============================================================================
std::vector<std::unique_ptr<RangedAudioParameter>> DistanceCompensatorAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<RangedAudioParameter>> params;


    params.push_back (OSCParameterInterface::createParameterTheOldWay ("inputChannelsSetting", "Number of input channels ", "",
                                     NormalisableRange<float> (0.0f, 64.0f, 1.0f), 0.0f,
                                     [](float value) {return value < 0.5f ? "Auto" : String (value);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("enableGains", "Enable Gain Compensation", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "Yes";
                                         else return "No";
                                     }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("enableDelays", "Enable Delay Compensation", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "Yes";
                                         else return "No";
                                     }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("speedOfSound", "Speed of Sound", "m/s",
                                     NormalisableRange<float> (330.0, 350.0, 0.1f), 343.2f,
                                     [](float value) {return String (value, 1);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("distanceExponent", "Distance-Gain Exponent", "",
                                     NormalisableRange<float> (0.5f, 1.5f, 0.1f), 1.0f,
                                     [](float value) {return String (value, 1);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("gainNormalization", "Gain Normalization", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "Zero-mean";
                                         else return "Attenuation only";
                                     }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("referenceX", "Reference position x", "m",
                                     NormalisableRange<float> (-20.0f, 20.0f, 0.01f), 0.0f,
                                     [](float value) {return String (value, 2);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("referenceY", "Reference position x", "m",
                                     NormalisableRange<float> (-20.0f, 20.0f, 0.01f), 0.0f,
                                     [](float value) {return String (value, 2);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("referenceZ", "Reference position x", "m",
                                     NormalisableRange<float> (-20.0f, 20.0f, 0.01f), 0.0f,
                                     [](float value) {return String (value, 2);}, nullptr));

    for (int i = 0; i < 64; ++i)
    {
        params.push_back (OSCParameterInterface::createParameterTheOldWay ("enableCompensation" + String (i), "Enable Compensation of loudspeaker " + String (i + 1), "",
                                        NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                        [](float value) {
                                            if (value >= 0.5f) return "ON";
                                            else return "OFF";
                                        }, nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay ("distance" + String (i), "Distance of loudspeaker " + String (i + 1), "m",
                                        NormalisableRange<float> (1.0f, 50.0f, 0.01f), 5.0f,
                                        [](float value) {return String (value, 2);}, nullptr));
    }

    return params;
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistanceCompensatorAudioProcessor();
}
