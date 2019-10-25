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

const StringArray SimpleDecoderAudioProcessor::weightsStrings =  StringArray ("basic", "maxrE", "inphase");

//==============================================================================
SimpleDecoderAudioProcessor::SimpleDecoderAudioProcessor()
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
    // dummy values
    cascadedLowPassCoeffs = IIR::Coefficients<double>::makeLowPass(48000.0, 100.0f);
    cascadedHighPassCoeffs = IIR::Coefficients<double>::makeHighPass(48000.0, 100.0f);

    lowPassCoeffs = IIR::Coefficients<float>::makeHighPass(48000.0, 100.0f);
    highPassCoeffs = IIR::Coefficients<float>::makeFirstOrderHighPass(48000.0, 100.0f);


    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue ("inputOrderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");

    lowPassFrequency = parameters.getRawParameterValue ("lowPassFrequency");
    lowPassGain = parameters.getRawParameterValue ("lowPassGain");
    highPassFrequency = parameters.getRawParameterValue ("highPassFrequency");

    swMode = parameters.getRawParameterValue ("swMode");
    swChannel = parameters.getRawParameterValue("swChannel");
    weights = parameters.getRawParameterValue ("weights");

    // add listeners to parameter changes

    parameters.addParameterListener ("inputOrderSetting", this);
    parameters.addParameterListener ("useSN3D", this);

    parameters.addParameterListener ("lowPassFrequency", this);
    parameters.addParameterListener ("lowPassQ", this);
    parameters.addParameterListener ("lowPassGain", this);
    parameters.addParameterListener ("highPassFrequency", this);
    parameters.addParameterListener ("highPassQ", this);

    parameters.addParameterListener ("swMode", this);
    parameters.addParameterListener ("weights", this);

    highPassSpecs.numChannels = 0;

    // global settings for all plug-in instances
    PropertiesFile::Options options;
    options.applicationName     = "Decoder";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";

    properties.reset (new PropertiesFile (options));
    lastDir = File(properties->getValue("presetFolder"));


    // filters
    highPass1.state = highPassCoeffs;
    highPass2.state = highPassCoeffs;

    lowPass1.reset (new IIR::Filter<float> (lowPassCoeffs));
    lowPass2.reset (new IIR::Filter<float> (lowPassCoeffs));
}

SimpleDecoderAudioProcessor::~SimpleDecoderAudioProcessor()
{
}

void SimpleDecoderAudioProcessor::updateLowPassCoefficients (double sampleRate, float frequency)
{
    frequency = jmin (static_cast<float> (0.5 * sampleRate), frequency);
    *lowPassCoeffs = *IIR::Coefficients<float>::makeLowPass (sampleRate, frequency);

    auto newCoeffs = IIR::Coefficients<double>::makeLowPass (sampleRate, frequency);
    newCoeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients (newCoeffs->coefficients, newCoeffs->coefficients);
    cascadedLowPassCoeffs = newCoeffs;
    guiUpdateLowPassCoefficients = true;
}

void SimpleDecoderAudioProcessor::updateHighPassCoefficients(double sampleRate, float frequency)
{
    frequency = jmin (static_cast<float> (0.5 * sampleRate), frequency);
    *highPassCoeffs = *IIR::Coefficients<float>::makeHighPass (sampleRate, frequency);

    auto newCoeffs = IIR::Coefficients<double>::makeHighPass (sampleRate, frequency);
    newCoeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients (newCoeffs->coefficients, newCoeffs->coefficients);
    cascadedHighPassCoeffs = newCoeffs;
    guiUpdateHighPassCoefficients = true;
}


void SimpleDecoderAudioProcessor::setLastDir(File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue("presetFolder", v);
}


//==============================================================================
int SimpleDecoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleDecoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleDecoderAudioProcessor::setCurrentProgram (int index)
{
}

const String SimpleDecoderAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleDecoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SimpleDecoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *inputOrderSetting, 0, true);

    swBuffer.setSize(1, samplesPerBlock);
    swBuffer.clear();

    ProcessSpec specs;
    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = samplesPerBlock;
    specs.numChannels = 64;
    decoder.prepare(specs);
    decoder.setInputNormalization(*useSN3D >= 0.5f ? ReferenceCountedDecoder::Normalization::sn3d : ReferenceCountedDecoder::Normalization::n3d);

    ReferenceCountedDecoder::Ptr currentDecoder = decoder.getCurrentDecoder();
    if (currentDecoder != nullptr) {
        highPassSpecs.numChannels = currentDecoder->getNumInputChannels();

        // calculate mean omni-signal-gain
        Matrix<float>& decoderMatrix = currentDecoder->getMatrix();
        const int nLsps = (int) decoderMatrix.getNumRows();
        float sumGains = 0.0f;
        for (int i = 0; i < nLsps; ++i)
            sumGains += decoderMatrix(i, 0);

        omniGain = sumGains / nLsps;
    }

    highPassSpecs.sampleRate = sampleRate;
    highPassSpecs.maximumBlockSize = samplesPerBlock;

    updateHighPassCoefficients(sampleRate, *highPassFrequency);
    updateLowPassCoefficients(sampleRate, *lowPassFrequency);

    highPass1.prepare(highPassSpecs);
    highPass1.reset();

    highPass2.prepare(highPassSpecs);
    highPass2.reset();

    lowPass1->prepare(highPassSpecs);
    lowPass1->reset();

    lowPass2->prepare(highPassSpecs);
    lowPass2->reset();

    masterGain.setRampDurationSeconds (0.1f);
    masterGain.prepare ({sampleRate, static_cast<uint32> (samplesPerBlock), 1});

    decoder.setInputNormalization(*useSN3D >= 0.5f ? ReferenceCountedDecoder::Normalization::sn3d : ReferenceCountedDecoder::Normalization::n3d);

    guiUpdateSampleRate = true;
}


void SimpleDecoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


void SimpleDecoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputOrderSetting, 0, false);
    ScopedNoDenormals noDenormals;

    const bool newDecoderWasAvailable = decoder.checkIfNewDecoderAvailable();
    ReferenceCountedDecoder::Ptr retainedDecoder = decoder.getCurrentDecoder();

    if (newDecoderWasAvailable && retainedDecoder != nullptr)
    {
        highPassSpecs.numChannels = decoder.getCurrentDecoder()->getNumInputChannels();
        highPass1.prepare(highPassSpecs);
        highPass2.prepare(highPassSpecs);
        if (decoder.getCurrentDecoder()->getSettings().subwooferChannel != -1)
        {
            parameters.getParameter ("swChannel")->setValueNotifyingHost (parameters.getParameterRange ("swChannel").convertTo0to1 (decoder.getCurrentDecoder()->getSettings().subwooferChannel));
            parameters.getParameter ("swMode")->setValueNotifyingHost (parameters.getParameterRange ("swMode").convertTo0to1 (1)); //discrete
        }

        // calculate mean omni-signal-gain
        Matrix<float>& decoderMatrix = retainedDecoder->getMatrix();
        const int nLsps = (int) decoderMatrix.getNumRows();
        float sumGains = 0.0f;
        for (int i = 0; i < nLsps; ++i)
            sumGains += decoderMatrix(i, 0);

        omniGain = sumGains / nLsps;
    }

    // ====== is a decoder loaded? stop processing if not ===========
    if (retainedDecoder == nullptr)
    {
        buffer.clear();
        return;
    }
    // ==============================================================

    const int nChIn = jmin(retainedDecoder->getNumInputChannels(), buffer.getNumChannels(), input.getNumberOfChannels());
    const int nChOut = jmin(retainedDecoder->getNumOutputChannels(), buffer.getNumChannels());
    const int swProcessing = *swMode;

    for (int ch = jmax(nChIn, nChOut); ch < buffer.getNumChannels(); ++ch) // clear all not needed channels
        buffer.clear(ch, 0, buffer.getNumSamples());

    if (swProcessing > 0)
    {
        swBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
        float correction = sqrt((static_cast<float>(retainedDecoder->getOrder()) + 1));

        if (swProcessing == 1) // subwoofer-mode: discrete
            correction *= sqrt((float) nChOut); // correction for only one subwoofer instead of nChOut loudspeakers

        swBuffer.applyGain(omniGain * correction);

        // low pass filtering
        AudioBlock<float> lowPassAudioBlock = AudioBlock<float>(swBuffer);
        ProcessContextReplacing<float> lowPassContext(lowPassAudioBlock);
        lowPass1->process(lowPassContext);
        lowPass2->process(lowPassContext);
        swBuffer.applyGain(0, 0, swBuffer.getNumSamples(), Decibels::decibelsToGain(*lowPassGain));

        AudioBlock<float> highPassAudioBlock = AudioBlock<float>(buffer.getArrayOfWritePointers(), nChIn, buffer.getNumSamples());
        ProcessContextReplacing<float> highPassContext (highPassAudioBlock);
        highPass1.process(highPassContext);
        highPass2.process(highPassContext);
    }

    // update current weights setting
    auto settings = retainedDecoder->getSettings();
    settings.weights = ReferenceCountedDecoder::Weights (roundToInt (*weights));
    retainedDecoder->setSettings (settings);

    // ambisonic decoding
    const int L = buffer.getNumSamples();
    auto inputAudioBlock = AudioBlock<float> (buffer.getArrayOfWritePointers(), nChIn, L);
    auto outputAudioBlock = AudioBlock<float> (buffer.getArrayOfWritePointers(), nChOut, L);
    decoder.process (inputAudioBlock, outputAudioBlock);

    for (int ch = nChOut; ch < nChIn; ++ch) // clear all not needed channels
        buffer.clear(ch, 0, buffer.getNumSamples());


    // =================== subwoofer processing ==================================
    if (swProcessing == 1)
    {
        const int swCh = ((int)*swChannel) - 1;
        if (swCh < buffer.getNumChannels())
            buffer.copyFrom(swCh, 0, swBuffer, 0, 0, buffer.getNumSamples());
    }

    else if (swProcessing == 2) // virtual subwoofer
    {
        Array<int>& rArray = decoder.getCurrentDecoder()->getRoutingArrayReference();
        for (int ch = rArray.size(); --ch >= 0;)
        {
            const int destCh = rArray.getUnchecked(ch);
            if (destCh < buffer.getNumChannels())
                buffer.addFrom(destCh, 0, swBuffer, 0, 0, buffer.getNumSamples());
        }
    }
    // =================== Master Gain =========================================
    auto overallGainInDecibels = *parameters.getRawParameterValue ("overallGain");
    masterGain.setGainDecibels (overallGainInDecibels);
    AudioBlock<float> ab (buffer.getArrayOfWritePointers(), nChOut,  buffer.getNumSamples());
    ProcessContextReplacing<float> masterContext (ab);
    masterGain.process (masterContext);
}

//==============================================================================
bool SimpleDecoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SimpleDecoderAudioProcessor::createEditor()
{
    return new SimpleDecoderAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void SimpleDecoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = parameters.copyState();

    state.setProperty ("lastOpenedPresetFile", var (lastFile.getFullPathName()), nullptr);

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<XmlElement> xml (state.createXml());
    xml->setTagName (String (JucePlugin_Name)); // converts old "Decoder" state to "SimpleDecoder" state
    copyXmlToBinary (*xml, destData);
}



void SimpleDecoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()) || xmlState->hasTagName ("Decoder")) // compatibility for old "Decoder" state tagName
            parameters.state = ValueTree::fromXml (*xmlState);
    if (parameters.state.hasProperty ("lastOpenedPresetFile"))
    {
        Value val = parameters.state.getPropertyAsValue ("lastOpenedPresetFile", nullptr);
        if (val.getValue().toString() != "")
        {
            auto* weightsParam = parameters.getParameter ("weights");
            const auto savedWeights = weightsParam->getValue();
            const File f (val.getValue().toString());
            loadConfiguration(f);
            weightsParam->setValueNotifyingHost (savedWeights);
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

//==============================================================================
void SimpleDecoderAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "inputOrderSetting")
        userChangedIOSettings = true;
    else if (parameterID == "highPassFrequency")
    {
        updateHighPassCoefficients (highPassSpecs.sampleRate, *highPassFrequency);
    }
    else if (parameterID == "lowPassFrequency")
    {
        updateLowPassCoefficients (highPassSpecs.sampleRate, *lowPassFrequency);
    }
    else if (parameterID == "lowPassGain")
    {
        guiUpdateLowPassGain = true;
    }
    else if (parameterID == "useSN3D")
    {
        decoder.setInputNormalization(*useSN3D >= 0.5f ? ReferenceCountedDecoder::Normalization::sn3d : ReferenceCountedDecoder::Normalization::n3d);
    }
}

void SimpleDecoderAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}

void SimpleDecoderAudioProcessor::loadConfiguration (const File& presetFile)
{
    ReferenceCountedDecoder::Ptr tempDecoder = nullptr;

    Result result = ConfigurationHelper::parseFileForDecoder (presetFile, &tempDecoder);
    if (result.failed())
        messageForEditor = result.getErrorMessage();

    if (tempDecoder != nullptr)
    {
        lastFile = presetFile;
        messageForEditor = "";

        tempDecoder->removeAppliedWeights();
        parameters.getParameterAsValue ("weights").setValue (static_cast<int> (tempDecoder->getSettings().weights));
    }


    decoder.setDecoder (tempDecoder);
    decoderConfig = tempDecoder;

    updateDecoderInfo = true;
    messageChanged = true;
}


//==============================================================================
const bool SimpleDecoderAudioProcessor::processNotYetConsumedOSCMessage (const OSCMessage &message)
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
std::vector<std::unique_ptr<RangedAudioParameter>> SimpleDecoderAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("inputOrderSetting", "Ambisonic Order", "",
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
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay("useSN3D", "Normalization", "",
                                    NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                    [](float value) {
                                        if (value >= 0.5f) return "SN3D";
                                        else return "N3D";
                                    }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("lowPassFrequency", "LowPass Cutoff Frequency", "Hz",
                                     NormalisableRange<float> (20.f, 300.f, 1.0f), 80.f,
                                     [](float value) {return String ((int) value);},
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("lowPassGain", "LowPass Gain", "dB",
                                     NormalisableRange<float> (-20.0f, 10.0, 0.1f), 0.0f,
                                     [](float value) {return String (value, 1);},
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("highPassFrequency", "HighPass Cutoff Frequency", "Hz",
                                     NormalisableRange<float> (20.f, 300.f, 1.f), 80.f,
                                     [](float value) {return String ((int) value);},
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("swMode", "Subwoofer Mode", "",
                                     NormalisableRange<float> (0.0f, 2.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value < 0.5f) return "none";
                                         else if (value >= 0.5f && value < 1.5f) return "Discrete SW";
                                         else return "Virtual SW";}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("swChannel", "SW Channel Number", "",
                                     NormalisableRange<float> (1.0f, 64.0f, 1.0f), 1.0f,
                                     [](float value) { return String ((int) value);}, nullptr));

    params.push_back (std::make_unique<AudioParameterChoice> ("weights", "Ambisonic Weights", weightsStrings, 1));

    params.push_back (std::make_unique<AudioParameterFloat> ("overallGain", "Overall Gain", NormalisableRange<float> (-20.0f, 20.0f, 0.01f), 0.0f, "dB", AudioProcessorParameter::outputGain, [] (float value, int maximumStringLength) { return String (value, maximumStringLength); }, nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleDecoderAudioProcessor();
}
