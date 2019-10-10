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


const StringArray BinauralDecoderAudioProcessor::headphoneEQs = StringArray ("AKG-K141MK2", "AKG-K240DF", "AKG-K240MK2", "AKG-K271MK2", "AKG-K271STUDIO", "AKG-K601", "AKG-K701", "AKG-K702", "AKG-K1000-Closed", "AKG-K1000-Open", "AudioTechnica-ATH-M50", "Beyerdynamic-DT250", "Beyerdynamic-DT770PRO-250Ohms", "Beyerdynamic-DT880", "Beyerdynamic-DT990PRO", "Presonus-HD7", "Sennheiser-HD430", "Sennheiser-HD480", "Sennheiser-HD560ovationII", "Sennheiser-HD565ovation", "Sennheiser-HD600", "Sennheiser-HD650", "SHURE-SRH940");

//==============================================================================
BinauralDecoderAudioProcessor::BinauralDecoderAudioProcessor()
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
        irs[i].setSize(square(i + 2), irLength);
        std::unique_ptr<AudioFormatReader> reader (wavFormat.createReaderFor (mis[i], true));
        reader->read(&irs[i], 0, irLength, 0, true, false);
        irs[i].applyGain (0.3f);
    }
}

BinauralDecoderAudioProcessor::~BinauralDecoderAudioProcessor()
{

    if (fftwWasPlanned)
    {
        fftwf_destroy_plan(fftForward);
        fftwf_destroy_plan(fftBackwardMid);
        fftwf_destroy_plan(fftBackwardSide);
    }

    if (in != nullptr)
        fftwf_free(in);
    if (out != nullptr)
        fftwf_free(out);
    if (accumMid != nullptr)
        fftwf_free(accumMid);
    if (accumSide != nullptr)
        fftwf_free(accumSide);
    if (ifftOutputMid != nullptr)
        fftwf_free(ifftOutputMid);
    if (ifftOutputSide != nullptr)
        fftwf_free(ifftOutputSide);
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

void BinauralDecoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputOrderSetting, 0, false);
    ScopedNoDenormals noDenormals;

    if (buffer.getNumChannels() < 2)
    {
        buffer.clear();
        return;
    }

    const int nCh = jmin(buffer.getNumChannels(), input.getNumberOfChannels());
    const int L = buffer.getNumSamples();
    const int ergL = overlapBuffer.getNumSamples();
    const int overlap = irLengthMinusOne;
    const int copyL = jmin(L, overlap); // copy max L samples of the overlap data

    if (*useSN3D >= 0.5f)
        for (int ch = 1; ch < nCh; ++ch)
            buffer.applyGain(ch, 0, buffer.getNumSamples(), sn3d2n3d[ch]);

    AudioBlock<float> tempBlock (stereoTemp);

    FloatVectorOperations::clear((float*) accumMid, fftLength + 2);
    FloatVectorOperations::clear((float*) accumSide, fftLength + 2);

    const int nZeros = fftLength - L;

    //compute mid signal in frequency domain
    for (int midix = 0; midix < nMidCh; ++midix)
    {
      int ch = mix2cix[midix];

      FloatVectorOperations::clear(&in[L], nZeros); // TODO: only last part
      FloatVectorOperations::copy(in, buffer.getReadPointer(ch), L);
      fftwf_execute(fftForward);

      fftwf_complex* tfMid = (fftwf_complex*) irsFrequencyDomain.getReadPointer(ch);

      for (int i = 0; i < fftLength / 2 + 1; ++i)
      {
        accumMid[i][0]  += out[i][0] *  tfMid[i][0] - out[i][1] *  tfMid[i][1]; //real part
        accumMid[i][1]  += out[i][1] *  tfMid[i][0] + out[i][0] *  tfMid[i][1]; //imag part
      }
    }

    //compute side signal in frequency domain
    for (int sidix = 0; sidix < nSideCh; ++sidix)
    {
        int ch = six2cix[sidix];

        FloatVectorOperations::clear(&in[L], nZeros); // TODO: only last part
        FloatVectorOperations::copy(in, buffer.getReadPointer(ch), L);
        fftwf_execute(fftForward);

        fftwf_complex* tfSide = (fftwf_complex*)irsFrequencyDomain.getReadPointer(ch);

        for (int i = 0; i < fftLength / 2 + 1; ++i)
        {
            accumSide[i][0] += out[i][0] * tfSide[i][0] - out[i][1] * tfSide[i][1];
            accumSide[i][1] += out[i][1] * tfSide[i][0] + out[i][0] * tfSide[i][1];
        }
    }

    fftwf_execute(fftBackwardMid);
    fftwf_execute(fftBackwardSide);


    ///* MS -> LR  */
    FloatVectorOperations::copy(buffer.getWritePointer(0), ifftOutputMid, L);
    FloatVectorOperations::copy(buffer.getWritePointer(1), ifftOutputMid, L);
    FloatVectorOperations::add(buffer.getWritePointer(0), ifftOutputSide, L);
    FloatVectorOperations::subtract(buffer.getWritePointer(1), ifftOutputSide, L);

    FloatVectorOperations::add (buffer.getWritePointer(0), overlapBuffer.getWritePointer(0), copyL);
    FloatVectorOperations::add (buffer.getWritePointer(1), overlapBuffer.getWritePointer(1), copyL);

    if (copyL < overlap) // there is some overlap left, want some?
    {
        const int howManyAreLeft = overlap - L;

                //shift the overlap buffer to the left
        FloatVectorOperations::copy(overlapBuffer.getWritePointer(0), overlapBuffer.getReadPointer(0, L), howManyAreLeft);
        FloatVectorOperations::copy(overlapBuffer.getWritePointer(1), overlapBuffer.getReadPointer(1, L), howManyAreLeft);

                //clear the tail
        FloatVectorOperations::clear(overlapBuffer.getWritePointer(0, howManyAreLeft), ergL - howManyAreLeft);
        FloatVectorOperations::clear(overlapBuffer.getWritePointer(1, howManyAreLeft), ergL - howManyAreLeft);

                /* MS -> LR  */
        FloatVectorOperations::add(overlapBuffer.getWritePointer(0), &ifftOutputMid[L], irLengthMinusOne);
        FloatVectorOperations::add(overlapBuffer.getWritePointer(1), &ifftOutputMid[L], irLengthMinusOne);
        FloatVectorOperations::add(overlapBuffer.getWritePointer(0), &ifftOutputSide[L], irLengthMinusOne);
        FloatVectorOperations::subtract(overlapBuffer.getWritePointer(1), &ifftOutputSide[L], irLengthMinusOne);
    }
    else
    {
                /* MS -> LR  */
        FloatVectorOperations::copy(overlapBuffer.getWritePointer(0), &ifftOutputMid[L], irLengthMinusOne);
        FloatVectorOperations::copy(overlapBuffer.getWritePointer(1), &ifftOutputMid[L], irLengthMinusOne);
        FloatVectorOperations::add(overlapBuffer.getWritePointer(0), &ifftOutputSide[L], irLengthMinusOne);
        FloatVectorOperations::subtract(overlapBuffer.getWritePointer(1), &ifftOutputSide[L], irLengthMinusOne);
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
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}


void BinauralDecoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
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

    int order = jmax(input.getOrder(), 1);
    const int nCh = input.getNumberOfChannels(); // why not jmin(buffer.....)? Is updateBuffers called before the first processBlock?
    DBG("order: " << order);
    DBG("nCh: " << nCh);

    int tmpOrder = sqrt(nCh) - 1;
    if (tmpOrder < order) {
        order = tmpOrder;
    }


    AudioBuffer<float> resampledIRs;
    bool useResampled = false;
    irLength = 236;

    if (sampleRate != irsSampleRate && order != 0) // do resampling!
    {
        useResampled = true;
        double factorReading = irsSampleRate / sampleRate;
        irLength = roundToInt (irLength / factorReading + 0.49);

        MemoryAudioSource memorySource (irs[order - 1], false);
        ResamplingAudioSource resamplingSource (&memorySource, false, nCh);

        resamplingSource.setResamplingRatio (factorReading);
        resamplingSource.prepareToPlay (irLength, sampleRate);

        resampledIRs.setSize(nCh, irLength);
        AudioSourceChannelInfo info;
        info.startSample = 0;
        info.numSamples = irLength;
        info.buffer = &resampledIRs;

        resamplingSource.getNextAudioBlock (info);

        // compensate for more (correlated) samples contributing to output signal
        resampledIRs.applyGain (irsSampleRate / sampleRate);
    }

    irLengthMinusOne = irLength - 1;

    const int prevFftLength = fftLength;

    const int ergL = blockSize + irLength - 1; //max number of nonzero output samples
    fftLength = nextPowerOfTwo(ergL);          //fftLength >= ergL

    overlapBuffer.setSize(2, irLengthMinusOne);
    overlapBuffer.clear();

    if (prevFftLength != fftLength)
    {
        if (fftwWasPlanned)
        {
            fftwf_destroy_plan(fftForward);
            fftwf_destroy_plan(fftBackwardMid);
            fftwf_destroy_plan(fftBackwardSide);
        }

        if (in != nullptr)
            fftwf_free(in);
        if (out != nullptr)
            fftwf_free(out);
        if (accumMid != nullptr)
            fftwf_free(accumMid);
        if (accumSide != nullptr)
            fftwf_free(accumSide);
        if (ifftOutputMid != nullptr)
            fftwf_free(ifftOutputMid);
        if (ifftOutputSide != nullptr)
            fftwf_free(ifftOutputSide);

        in = (float*) fftwf_malloc(sizeof(float) * fftLength);
        out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (fftLength / 2 + 1));
        accumMid = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (fftLength / 2 + 1));
        accumSide = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (fftLength / 2 + 1));
        ifftOutputMid = (float*) fftwf_malloc(sizeof(float) * fftLength);
        ifftOutputSide = (float*) fftwf_malloc(sizeof(float) * fftLength);

        fftForward = fftwf_plan_dft_r2c_1d(fftLength, in, out, FFTW_MEASURE);
        fftBackwardMid = fftwf_plan_dft_c2r_1d(fftLength, accumMid, ifftOutputMid, FFTW_MEASURE);
        fftBackwardSide = fftwf_plan_dft_c2r_1d(fftLength, accumSide, ifftOutputSide, FFTW_MEASURE);
        fftwWasPlanned = true;
    }

    FloatVectorOperations::clear((float*) in, fftLength); // clear (after plan creation!)

    irsFrequencyDomain.setSize(nCh, 2 * (fftLength / 2 + 1));
    irsFrequencyDomain.clear();

    for (int i = 0; i < nCh; ++i)
    {
        const float* src = useResampled ? resampledIRs.getReadPointer(i) : irs[order - 1].getReadPointer(i);
        FloatVectorOperations::multiply((float*)in, src, 1.0 / fftLength, irLength);
        FloatVectorOperations::clear(&in[irLength], fftLength - irLength); // zero padding
        fftwf_execute(fftForward);
        FloatVectorOperations::copy(irsFrequencyDomain.getWritePointer(i), (float*)out, 2 * (fftLength / 2 + 1));
    }

    //get number of mid- and side-channels
    nSideCh = order * (order + 1) / 2;
    nMidCh = square(order + 1) - nSideCh;   //nMidCh = nCh - nSideCh; //nCh should be equalt to (order+1)^2
}


//==============================================================================
std::vector<std::unique_ptr<RangedAudioParameter>> BinauralDecoderAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("inputOrderSetting", "Input Ambisonic Order", "",
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

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("useSN3D", "Input Normalization", "",
                                                       NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                                       [](float value) {
                                                           if (value >= 0.5f) return "SN3D";
                                                           else return "N3D";
                                                       }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("applyHeadphoneEq", "Headphone Equalization", "",
                                                       NormalisableRange<float>(0.0f, float(headphoneEQs.size()), 1.0f), 0.0f,
                                                       [this](float value) {
                                                           if (value < 0.5f) return String("OFF");
                                                           else return String(this->headphoneEQs[roundToInt(value)-1]);
                                                       }, nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BinauralDecoderAudioProcessor();
}
