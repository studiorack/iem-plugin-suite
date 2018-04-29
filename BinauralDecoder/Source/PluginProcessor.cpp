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
    mis[2] = new MemoryInputStream (BinaryData::irsOrd3_wav, BinaryData::irsOrd3_wavSize, false);
    mis[3] = new MemoryInputStream (BinaryData::irsOrd4_wav, BinaryData::irsOrd4_wavSize, false);
    mis[4] = new MemoryInputStream (BinaryData::irsOrd5_wav, BinaryData::irsOrd5_wavSize, false);
    mis[5] = new MemoryInputStream (BinaryData::irsOrd6_wav, BinaryData::irsOrd6_wavSize, false);
    mis[6] = new MemoryInputStream (BinaryData::irsOrd7_wav, BinaryData::irsOrd7_wavSize, false);

    for (int i = 0; i < 7; ++i)
    {
        irs[i].setSize(2 * square(i + 2), 236);
        ScopedPointer<AudioFormatReader> reader = wavFormat.createReaderFor(mis[i], true);
        reader->read(&irs[i], 0, 236, 0, true, false);
    }

}

BinauralDecoderAudioProcessor::~BinauralDecoderAudioProcessor()
{

    if (fftwWasPlanned)
    {
        fftwf_destroy_plan(fftForward);
        fftwf_destroy_plan(fftBackwardLeft);
        fftwf_destroy_plan(fftBackwardRight);
    }

    if (in != nullptr)
        fftwf_free(in);
    if (out != nullptr)
        fftwf_free(out);
    if (accumLeft != nullptr)
        fftwf_free(accumLeft);
    if (accumRight != nullptr)
        fftwf_free(accumRight);
    if (ifftOutputLeft != nullptr)
        fftwf_free(ifftOutputLeft);
    if (ifftOutputRight != nullptr)
        fftwf_free(ifftOutputRight);
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

    stereoTemp.setSize(2, samplesPerBlock);

    ProcessSpec convSpec;
    convSpec.sampleRate = sampleRate;
    convSpec.maximumBlockSize = samplesPerBlock;
    convSpec.numChannels = 2; // convolve two channels (which actually point two one and the same input channel)

    EQ.prepare(convSpec);

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

    const int nIRsPerChannel = input.getNumberOfChannels();
    const int nCh = jmin(buffer.getNumChannels(), input.getNumberOfChannels());
    const int L = buffer.getNumSamples();
    const int ergL = overlapBuffer.getNumSamples();
    const int overlap = irLengthMinusOne;
    const int copyL = jmin(L, overlap); // copy max L samples of the overlap data

    if (*useSN3D >= 0.5f)
        for (int ch = 1; ch < nCh; ++ch)
            buffer.applyGain(ch, 0, buffer.getNumSamples(), sn3d2n3d[ch]);

    AudioBlock<float> tempBlock (stereoTemp);

    FloatVectorOperations::clear((float*) accumLeft, fftLength + 2);
    FloatVectorOperations::clear((float*) accumRight, fftLength + 2);

    const int nZeros = fftLength - L;
    for (int ch = 0; ch < nCh; ++ch)
    {
        FloatVectorOperations::clear(&in[L], nZeros); // TODO: only last part
        FloatVectorOperations::copy(in, buffer.getReadPointer(ch), L);
        fftwf_execute(fftForward);

        fftwf_complex* irLeft = (fftwf_complex*) irsFrequencyDomain.getReadPointer(ch);
        fftwf_complex* irRight = (fftwf_complex*) irsFrequencyDomain.getReadPointer(nIRsPerChannel + ch);

        for (int i = 0; i < fftLength / 2 + 1; ++i)
        {
            accumLeft[i][0]  += out[i][0] *  irLeft[i][0] - out[i][1] *  irLeft[i][1];
            accumLeft[i][1]  += out[i][1] *  irLeft[i][0] + out[i][0] *  irLeft[i][1];
            accumRight[i][0] += out[i][0] * irRight[i][0] - out[i][1] * irRight[i][1];
            accumRight[i][1] += out[i][1] * irRight[i][0] + out[i][0] * irRight[i][1];
        }
    }

    fftwf_execute(fftBackwardLeft);
    fftwf_execute(fftBackwardRight);

    FloatVectorOperations::copy(buffer.getWritePointer(0), ifftOutputLeft, L);
    FloatVectorOperations::copy(buffer.getWritePointer(1), ifftOutputRight, L);

    FloatVectorOperations::add (buffer.getWritePointer(0), overlapBuffer.getWritePointer(0), copyL);
    FloatVectorOperations::add (buffer.getWritePointer(1), overlapBuffer.getWritePointer(1), copyL);

    if (copyL < overlap) // there is some overlap left, want some?
    {
        const int howManyAreLeft = overlap - L;
        FloatVectorOperations::copy(overlapBuffer.getWritePointer(0), overlapBuffer.getReadPointer(0, L), howManyAreLeft);
        FloatVectorOperations::copy(overlapBuffer.getWritePointer(1), overlapBuffer.getReadPointer(1, L), howManyAreLeft);

        FloatVectorOperations::clear(overlapBuffer.getWritePointer(0, howManyAreLeft), ergL - howManyAreLeft);
        FloatVectorOperations::clear(overlapBuffer.getWritePointer(1, howManyAreLeft), ergL - howManyAreLeft);

        FloatVectorOperations::add(overlapBuffer.getWritePointer(0), &ifftOutputLeft[L], irLengthMinusOne);
        FloatVectorOperations::add(overlapBuffer.getWritePointer(1), &ifftOutputRight[L], irLengthMinusOne);
    }
    else
    {
        FloatVectorOperations::copy(overlapBuffer.getWritePointer(0), &ifftOutputLeft[L], irLengthMinusOne);
        FloatVectorOperations::copy(overlapBuffer.getWritePointer(1), &ifftOutputRight[L], irLengthMinusOne);
    }

    if (*applyHeadphoneEq >= 0.5f)
    {
        float* channelData[2] = {buffer.getWritePointer(0), buffer.getWritePointer(1)};
        AudioBlock<float> sumBlock (channelData, 2, L);
        ProcessContextReplacing<float> eqContext (sumBlock);
        EQ.process(eqContext);
    }

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

    const double sampleRate = getSampleRate();
    const int blockSize = getBlockSize();

    const int order = jmax(input.getOrder(), 1);
    const int nCh = input.getNumberOfChannels();
    DBG("order: " << order);
    DBG("nCh: " << nCh);

    int irLength = 236;
    AudioBuffer<float> resampledIRs;
    bool useResampled = false;

    if (sampleRate != 44100.0) // do resampling!
    {
        useResampled = true;
        double factorReading = 44100.0f / sampleRate;
        irLength = roundToInt (236 / factorReading + 0.49);

        MemoryAudioSource memorySource (irs[order - 1], false);
        ResamplingAudioSource resamplingSource (&memorySource, false, 2 * nCh);

        resamplingSource.setResamplingRatio (factorReading);
        resamplingSource.prepareToPlay (irLength, sampleRate);

        resampledIRs.setSize(2 * nCh, irLength);
        AudioSourceChannelInfo info;
        info.startSample = 0;
        info.numSamples = irLength;
        info.buffer = &resampledIRs;

        resamplingSource.getNextAudioBlock (info);
    }

    irLengthMinusOne = irLength - 1;

    const int prevFftLength = fftLength;

    const int ergL = blockSize + irLength - 1;
    fftLength = nextPowerOfTwo(ergL);

    overlapBuffer.setSize(2, irLengthMinusOne);
    overlapBuffer.clear();

    if (prevFftLength != fftLength)
    {
        if (fftwWasPlanned)
        {
            fftwf_destroy_plan(fftForward);
            fftwf_destroy_plan(fftBackwardLeft);
            fftwf_destroy_plan(fftBackwardRight);
        }

        if (in != nullptr)
            fftwf_free(in);
        if (out != nullptr)
            fftwf_free(out);
        if (accumLeft != nullptr)
            fftwf_free(accumLeft);
        if (accumRight != nullptr)
            fftwf_free(accumRight);
        if (ifftOutputLeft != nullptr)
            fftwf_free(ifftOutputLeft);
        if (ifftOutputRight != nullptr)
            fftwf_free(ifftOutputRight);

        in = (float*) fftwf_malloc(sizeof(float) * fftLength);
        out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (fftLength / 2 + 1));
        accumLeft = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (fftLength / 2 + 1));
        accumRight = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (fftLength / 2 + 1));
        ifftOutputLeft = (float*) fftwf_malloc(sizeof(float) * fftLength);
        ifftOutputRight = (float*) fftwf_malloc(sizeof(float) * fftLength);

        fftForward = fftwf_plan_dft_r2c_1d(fftLength, in, out, FFTW_MEASURE);
        fftBackwardLeft = fftwf_plan_dft_c2r_1d(fftLength, accumLeft, ifftOutputLeft, FFTW_MEASURE);
        fftBackwardRight = fftwf_plan_dft_c2r_1d(fftLength, accumRight, ifftOutputRight, FFTW_MEASURE);
        fftwWasPlanned = true;
    }

    FloatVectorOperations::clear((float*) in, fftLength); // clear (after plan creation!)

    irsFrequencyDomain.setSize(2 * nCh, 2 * (fftLength / 2 + 1));
    irsFrequencyDomain.clear();

    for (int i = 0; i < nCh; ++i)
    {
        {  // left channel
            const float* src = useResampled ? resampledIRs.getReadPointer(2*i) : irs[order - 1].getReadPointer(2 * i);
            FloatVectorOperations::multiply((float*) in, src, 1.0 / fftLength, irLength);
            FloatVectorOperations::clear(&in[irLength], fftLength - irLength); // zero padding
            fftwf_execute(fftForward);
            FloatVectorOperations::copy(irsFrequencyDomain.getWritePointer(i), (float*) out, 2 * (fftLength / 2 + 1));
        }

        {  // right channel
            const float* src = useResampled ? resampledIRs.getReadPointer(2*i + 1) : irs[order - 1].getReadPointer(2 * i + 1);
            FloatVectorOperations::multiply((float*) in, src, 1.0 / fftLength, irLength);
            FloatVectorOperations::clear(&in[irLength], fftLength - irLength); // zero padding
            fftwf_execute(fftForward);
            FloatVectorOperations::copy(irsFrequencyDomain.getWritePointer(nCh + i), (float*) out, 2 * (fftLength / 2 + 1));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BinauralDecoderAudioProcessor();
}
