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
SimpleDecoderAudioProcessor::SimpleDecoderAudioProcessor()
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
    // dummy values
    cascadedLowPassCoeffs = IIR::Coefficients<double>::makeLowPass(48000.0, 100.0f);
    cascadedHighPassCoeffs = IIR::Coefficients<double>::makeHighPass(48000.0, 100.0f);
    
    lowPassCoeffs = IIR::Coefficients<float>::makeHighPass(48000.0, 100.0f);
    highPassCoeffs = IIR::Coefficients<float>::makeFirstOrderHighPass(48000.0, 100.0f);

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
    
    parameters.createAndAddParameter ("lowPassFrequency", "LowPass Cutoff Frequency", "Hz",
                                      NormalisableRange<float> (20.f, 300.f, 1.0f), 80.f,
                                      [](float value) {return String (value, 0);},
                                      nullptr);
    parameters.createAndAddParameter ("lowPassGain", "LowPass Gain", "dB",
                                      NormalisableRange<float> (-20.0f, 10.0, 0.1f), 0.0f,
                                      [](float value) {return String (value, 1);},
                                      nullptr);
    
    parameters.createAndAddParameter ("highPassFrequency", "HighPass Cutoff Frequency", "Hz",
                                      NormalisableRange<float> (20.f, 300.f, 1.f), 80.f,
                                      [](float value) {return String (value, 0);},
                                      nullptr);

    parameters.createAndAddParameter ("swMode", "Subwoofer Mode", "",
                                     NormalisableRange<float> (0.0f, 2.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value < 0.5f) return "none";
                                         else if (value >= 0.5f && value < 1.5f) return "Discrete SW";
                                         else return "Virtual SW";}, nullptr);
    
    parameters.createAndAddParameter ("swChannel", "SW Channel Number", "",
                                      NormalisableRange<float> (1.0f, 64.0f, 1.0f), 1.0f,
                                      [](float value) { return String (value, 0);}, nullptr);
    
    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("Decoder"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened
    
    
    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue ("inputOrderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    
    lowPassFrequency = parameters.getRawParameterValue ("lowPassFrequency");
    lowPassGain = parameters.getRawParameterValue ("lowPassGain");
    highPassFrequency = parameters.getRawParameterValue ("highPassFrequency");
    
    swMode = parameters.getRawParameterValue ("swMode");
    swChannel = parameters.getRawParameterValue("swChannel");
    
    // add listeners to parameter changes
    
    parameters.addParameterListener ("inputOrderSetting", this);
    parameters.addParameterListener ("useSN3D", this);
    
    parameters.addParameterListener ("lowPassFrequency", this);
    parameters.addParameterListener ("lowPassQ", this);
    parameters.addParameterListener ("lowPassGain", this);
    parameters.addParameterListener ("highPassFrequency", this);
    parameters.addParameterListener ("highPassQ", this);
    
    parameters.addParameterListener ("swMode", this);
    
    
    
    highPassSpecs.numChannels = 0;
    
    // global settings for all plug-in instances
    PropertiesFile::Options options;
    options.applicationName     = "Decoder";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";
    
    properties = new PropertiesFile(options);
    lastDir = File(properties->getValue("presetFolder"));
    

    // filters
    
    highPass1.state = highPassCoeffs;
    highPass2.state = highPassCoeffs;
    
    lowPass1 = new IIR::Filter<float>(lowPassCoeffs);
    lowPass2 = new IIR::Filter<float>(lowPassCoeffs);
}

SimpleDecoderAudioProcessor::~SimpleDecoderAudioProcessor()
{
}

void SimpleDecoderAudioProcessor::updateLowPassCoefficients(double sampleRate, float frequency)
{
    *lowPassCoeffs = *IIR::Coefficients<float>::makeLowPass(sampleRate, frequency);

    cascadedLowPassCoeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients
    (
     IIR::Coefficients<double>::makeLowPass(sampleRate, frequency)->coefficients,
     IIR::Coefficients<double>::makeLowPass(sampleRate, frequency)->coefficients
     );
}

void SimpleDecoderAudioProcessor::updateHighPassCoefficients(double sampleRate, float frequency)
{
    *highPassCoeffs = *IIR::Coefficients<float>::makeHighPass(sampleRate, frequency);

    cascadedHighPassCoeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients
    (
     IIR::Coefficients<double>::makeHighPass(sampleRate, frequency)->coefficients,
     IIR::Coefficients<double>::makeHighPass(sampleRate, frequency)->coefficients
     );
}


void SimpleDecoderAudioProcessor::setLastDir(File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue("presetFolder", v);
}


//==============================================================================
const String SimpleDecoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleDecoderAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SimpleDecoderAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SimpleDecoderAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SimpleDecoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

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
    
    decoder.setInputNormalization(*useSN3D >= 0.5f ? ReferenceCountedDecoder::Normalization::sn3d : ReferenceCountedDecoder::Normalization::n3d);
}


void SimpleDecoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleDecoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void SimpleDecoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputOrderSetting, 0, false);
    ScopedNoDenormals noDenormals;
    
    if (decoder.checkIfNewDecoderAvailable() && decoder.getCurrentDecoder() != nullptr)
    {
        highPassSpecs.numChannels = decoder.getCurrentDecoder()->getNumInputChannels();
        highPass1.prepare(highPassSpecs);
        highPass2.prepare(highPassSpecs);
        if (decoder.getCurrentDecoder()->getSettings().subwooferChannel != -1)
        {
            parameters.getParameterAsValue("swChannel").setValue(decoder.getCurrentDecoder()->getSettings().subwooferChannel);
            parameters.getParameterAsValue("swMode").setValue(1); //discrete
        }
    }
    
    // ====== is a decoder loaded? stop processing if not ===========
    if (decoder.getCurrentDecoder() == nullptr)
    {
        buffer.clear();
        return;
    }
    // ==============================================================
    
    const int nChIn = jmin(decoder.getCurrentDecoder()->getNumInputChannels(), buffer.getNumChannels(), input.getNumberOfChannels());
    const int nChOut = jmin(decoder.getCurrentDecoder()->getNumOutputChannels(), buffer.getNumChannels());
    const int swProcessing = *swMode;
    
    for (int ch = nChIn; ch < buffer.getNumChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());
    
    if (swProcessing > 0)
    {
        swBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
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
    
    AudioBlock<float> SimpleDecoderAudioBlock = AudioBlock<float>(buffer.getArrayOfWritePointers(), jmax(nChIn, nChOut), buffer.getNumSamples());
    ProcessContextReplacing<float> decoderContext (SimpleDecoderAudioBlock);
    decoder.process(SimpleDecoderAudioBlock);

    
    // =================== subwoofer processing ==================================
    if (swProcessing == 1 && nChOut < buffer.getNumChannels())
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
    // ======================================================================
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    parameters.state.setProperty("lastOpenedPresetFile", var(lastFile.getFullPathName()), nullptr);
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}



void SimpleDecoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
void SimpleDecoderAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "inputOrderSetting")
        userChangedIOSettings = true;
    else if (parameterID == "highPassFrequency")
    {
        updateHighPassCoefficients(highPassSpecs.sampleRate, *highPassFrequency);
        updateFv = true;
    }
    else if (parameterID == "lowPassFrequency")
    {
        updateLowPassCoefficients(highPassSpecs.sampleRate, *lowPassFrequency);
        updateFv = true;
    }
    else if (parameterID == "lowPassGain")
    {
        updateFv = true;
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

void SimpleDecoderAudioProcessor::loadPreset(const File& presetFile)
{
    ReferenceCountedDecoder::Ptr tempDecoder = nullptr;
    
    Result result = DecoderHelper::parseFileForDecoder(presetFile, &tempDecoder);
    if (!result.wasOk()) {
        messageForEditor = result.getErrorMessage();
    }
    
    decoder.setDecoder(tempDecoder);
    
    if (tempDecoder != nullptr)
    {
        tempDecoder->processAppliedWeights();
        lastFile = presetFile;
        messageForEditor = "";
    }

    
    //highPassFilters.prepare(highPassSpecs);
    decoderConfig = tempDecoder;
    
    messageChanged = true;
    
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleDecoderAudioProcessor();
}
