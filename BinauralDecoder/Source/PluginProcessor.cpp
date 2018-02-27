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
BinauralDecoderAudioProcessor::BinauralDecoderAudioProcessor()
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
    
    parameters.createAndAddParameter("applyHeadphoneEq", "Headphone Equalization", "",
                                     NormalisableRange<float>(0.0f, float(headphoneEQs.size()), 1.0f), 0.0f,
                                     [this](float value) {
                                         if (value < 0.5f) return String("OFF");
                                         else return String(this->headphoneEQs[roundToInt(value)-1]);
                                     }, nullptr);
    
    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("BinauralDecoder"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened
    
    
    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue("inputOrderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    applyHeadphoneEq = parameters.getRawParameterValue("applyHeadphoneEq");

    // add listeners to parameter changes
    parameters.addParameterListener ("inputOrderSetting", this);
    parameters.addParameterListener ("applyHeadphoneEq", this);
    
    
    // load IRs
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    WavAudioFormat wavFormat;
    
    MemoryInputStream* mis[7];
    mis[0] = new MemoryInputStream (BinaryData::irsOrd1_wav, BinaryData::irsOrd1_wavSize, false);
    mis[1] = new MemoryInputStream (BinaryData::irsOrd2_wav, BinaryData::irsOrd2_wavSize, false);
    mis[2] = new MemoryInputStream (BinaryData::irsOrd3_wav, BinaryData::irsOrd2_wavSize, false);
    mis[3] = new MemoryInputStream (BinaryData::irsOrd4_wav, BinaryData::irsOrd3_wavSize, false);
    mis[4] = new MemoryInputStream (BinaryData::irsOrd5_wav, BinaryData::irsOrd4_wavSize, false);
    mis[5] = new MemoryInputStream (BinaryData::irsOrd6_wav, BinaryData::irsOrd5_wavSize, false);
    mis[6] = new MemoryInputStream (BinaryData::irsOrd7_wav, BinaryData::irsOrd6_wavSize, false);
    
    
    for (int i = 0; i < 7; ++i)
    {
        irs[i].setSize(2 * square(i + 2), 236);
        ScopedPointer<AudioFormatReader> reader = wavFormat.createReaderFor(mis[i], true);
        reader->read(&irs[i], 0, 236, 0, true, false);
    }
        
    for (int ch = 0; ch < 64; ++ch)
    {
        engines.add(new Convolution);
    }
}

BinauralDecoderAudioProcessor::~BinauralDecoderAudioProcessor()
{
}

//==============================================================================
const String BinauralDecoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BinauralDecoderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BinauralDecoderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BinauralDecoderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BinauralDecoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BinauralDecoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BinauralDecoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BinauralDecoderAudioProcessor::setCurrentProgram (int index)
{
}

const String BinauralDecoderAudioProcessor::getProgramName (int index)
{
    return {};
}

void BinauralDecoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void BinauralDecoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *inputOrderSetting, 0, true);
    
    stereoSum.setSize(2, samplesPerBlock);
    stereoTemp.setSize(2, samplesPerBlock);
    
    ProcessSpec convSpec;
    convSpec.sampleRate = sampleRate;
    convSpec.maximumBlockSize = samplesPerBlock;
    convSpec.numChannels = 2; // convolve two channels (which actually point two one and the same input channel)
    
    EQ.prepare(convSpec);
    for (int i = 0; i < 64; ++i)
        engines[i]->prepare(convSpec);
}

void BinauralDecoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BinauralDecoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void BinauralDecoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputOrderSetting, 0, false);
    ScopedNoDenormals noDenormals;
    
    const int nCh = jmin(buffer.getNumChannels(), input.getNumberOfChannels());
    //const int engineSelect = jmax(input.getOrder(), 1) - 1;
    
    if (*useSN3D >= 0.5f)
        for (int ch = 1; ch < nCh; ++ch)
            buffer.applyGain(ch, 0, buffer.getNumSamples(), sn3d2n3d[ch]);
    
    
    AudioBlock<float> tempBlock (stereoTemp);
    stereoSum.clear();
    AudioBlock<float> sumBlock (stereoSum);
    for (int ch = 0; ch < nCh; ++ch)
    {
        float* channelPtrs[2] = {buffer.getWritePointer(ch), buffer.getWritePointer(ch)};
        AudioBlock<float> inBlock (channelPtrs, 2, buffer.getNumSamples());
        ProcessContextNonReplacing<float> context (inBlock, tempBlock);
        
        engines[ch]->process(context);
        sumBlock += tempBlock;
    }
    
    if (*applyHeadphoneEq >= 0.5f)
    {
        ProcessContextReplacing<float> eqContext (sumBlock);
        EQ.process(eqContext);
    }
    
    buffer.copyFrom(0, 0, stereoSum, 0, 0, buffer.getNumSamples());
    buffer.copyFrom(1, 0, stereoSum, 1, 0, buffer.getNumSamples());
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());
}

//==============================================================================
bool BinauralDecoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BinauralDecoderAudioProcessor::createEditor()
{
    return new BinauralDecoderAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void BinauralDecoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}



void BinauralDecoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.state = ValueTree::fromXml (*xmlState);
}

//==============================================================================
void BinauralDecoderAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "inputOrderSetting")
        userChangedIOSettings = true;
    else if (parameterID == "applyHeadphoneEq")
    {
        const int sel (roundToInt(newValue));
        if (sel > 0)
        {
            int sourceDataSize;
            String name = headphoneEQs[sel-1].replace("-", "") + "_wav";
            auto* sourceData = BinaryData::getNamedResource(name.toUTF8(), sourceDataSize);
            if (sourceData == nullptr)
                DBG("error");
            EQ.loadImpulseResponse(sourceData, sourceDataSize, true, false, 2048, false);
        }
    }
}

void BinauralDecoderAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
    
    ProcessSpec convSpec;
    convSpec.sampleRate = getSampleRate();
    convSpec.maximumBlockSize = getBlockSize();
    convSpec.numChannels = 2; // convolve two channels (which actually point two one and the same input channel)
    
    EQ.prepare(convSpec);
    for (int i = 0; i < 64; ++i)
        engines[i]->prepare(convSpec);
    
    AudioBuffer<float> zeroBuffer (2, getBlockSize());
    zeroBuffer.clear();
    AudioBuffer<float> targetBuffer (2, getBlockSize());
    AudioBlock<float> in(zeroBuffer);
    AudioBlock<float> out(targetBuffer);
    ProcessContextNonReplacing<float> context(in, out);
    
    const int nCh = input.getNumberOfChannels();
    const int irSelect = input.getOrder() > 0 ? input.getOrder() - 1 : 0;
    for (int ch = 0; ch < nCh; ++ch)
    {
        AudioBuffer<float> temp (irs[irSelect].getArrayOfWritePointers() + 2 * ch, 2, 236);
        engines[ch]->copyAndLoadImpulseResponseFromBuffer(temp, 44100.0, true, false, false, 236);
        engines[ch]->prepare(convSpec);
        
    }
    for (int ch = 0; ch < 64; ++ch)
        for (int i = 0; i < (getSampleRate()) / convSpec.maximumBlockSize + 1; ++i)
        {
            engines[ch]->process(context);
        }
    
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BinauralDecoderAudioProcessor();
}
