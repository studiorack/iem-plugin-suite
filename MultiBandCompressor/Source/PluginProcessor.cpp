/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Markus Huber
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
MultiBandCompressorAudioProcessor::MultiBandCompressorAudioProcessor()
     : AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
                           BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                           .withInput  ("Input",  AudioChannelSet::discreteChannels (64), true)
#endif
                           .withOutput ("Output", AudioChannelSet::discreteChannels (64), true)
#endif
                           ,
#endif
                           createParameterLayout()),
       maxNumFilters (ceil (64 / filterRegisterSize))
{
    const String inputSettingID = "orderSetting";
    orderSetting = parameters.getRawParameterValue (inputSettingID);
    parameters.addParameterListener (inputSettingID, this);


    for (int i = 0; i < numFreqBands-1; ++i)
    {
        const String crossoverID ("crossover" + String (i));

        crossovers[i] = parameters.getRawParameterValue (crossoverID);

        lowPassLRCoeffs[i] = IIR::Coefficients<double>::makeLowPass (lastSampleRate, *crossovers[i]);
        highPassLRCoeffs[i] = IIR::Coefficients<double>::makeHighPass (lastSampleRate, *crossovers[i]);

        calculateCoefficients (i);

        iirLPCoefficients[i] = IIR::Coefficients<float>::makeLowPass (lastSampleRate, *crossovers[i]);
        iirHPCoefficients[i] = IIR::Coefficients<float>::makeHighPass (lastSampleRate, *crossovers[i]);
        iirAPCoefficients[i] = IIR::Coefficients<float>::makeAllPass (lastSampleRate, *crossovers[i]);

        parameters.addParameterListener (crossoverID, this);

        iirLP[i].clear();
        iirLP2[i].clear();
        iirHP[i].clear();
        iirHP2[i].clear();
        iirAP[i].clear();

        for (int ch = 0; ch < maxNumFilters; ++ch)
        {
            iirLP[i].add (new IIR::Filter<filterFloatType> (iirLPCoefficients[i]));
            iirLP2[i].add (new IIR::Filter<filterFloatType> (iirLPCoefficients[i]));
            iirHP[i].add (new IIR::Filter<filterFloatType> (iirHPCoefficients[i]));
            iirHP2[i].add (new IIR::Filter<filterFloatType> (iirHPCoefficients[i]));
            iirAP[i].add (new IIR::Filter<filterFloatType> (iirAPCoefficients[i]));
        }
    }

    for (int i = 0; i < numFreqBands; ++i)
    {
        for (int ch = 0; ch < maxNumFilters; ++ch)
        {
            freqBandsBlocks[i].push_back (HeapBlock<char> ());
        }

        const String thresholdID ("threshold" + String (i));
        const String kneeID ("knee" + String (i));
        const String attackID ("attack" + String (i));
        const String releaseID ("release" + String (i));
        const String ratioID ("ratio" + String (i));
        const String makeUpGainID ("makeUpGain" + String (i));
        const String bypassID ("bypass" + String (i));
        const String soloID ("solo" + String (i));

        threshold[i] = parameters.getRawParameterValue (thresholdID);
        knee[i] = parameters.getRawParameterValue (kneeID);
        attack[i] = parameters.getRawParameterValue (attackID);
        release[i] = parameters.getRawParameterValue (releaseID);
        ratio[i] = parameters.getRawParameterValue (ratioID);
        makeUpGain[i] = parameters.getRawParameterValue (makeUpGainID);
        bypass[i] = parameters.getRawParameterValue (bypassID);

        parameters.addParameterListener (thresholdID, this);
        parameters.addParameterListener (kneeID, this);
        parameters.addParameterListener (attackID, this);
        parameters.addParameterListener (releaseID, this);
        parameters.addParameterListener (ratioID, this);
        parameters.addParameterListener (makeUpGainID, this);
        parameters.addParameterListener (bypassID, this);
        parameters.addParameterListener (soloID, this);
    }

    soloArray.clear();

    copyCoeffsToProcessor();

    for (int ch = 0; ch < maxNumFilters; ++ch)
    {
        interleavedBlockData.push_back (HeapBlock<char> ());
    }
}

MultiBandCompressorAudioProcessor::~MultiBandCompressorAudioProcessor()
{
}

std::vector<std::unique_ptr<RangedAudioParameter>> MultiBandCompressorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    const float crossoverPresets [numFreqBands-1] = { 80.0f, 440.0f, 2200.0f };

    auto floatParam = std::make_unique<AudioParameterFloat> ("orderSetting",
                                                        "Ambisonics Order",
                                                        NormalisableRange<float>(0.0f, 8.0f, 1.0f), 0.0f,
                                                        "",
                                                        AudioProcessorParameter::genericParameter,
                                                        [](float value, int maximumStringLength)
                                                        {
                                                            if (value >= 0.5f && value < 1.5f) return "0th";
                                                            else if (value >= 1.5f && value < 2.5f) return "1st";
                                                            else if (value >= 2.5f && value < 3.5f) return "2nd";
                                                            else if (value >= 3.5f && value < 4.5f) return "3rd";
                                                            else if (value >= 4.5f && value < 5.5f) return "4th";
                                                            else if (value >= 5.5f && value < 6.5f) return "5th";
                                                            else if (value >= 6.5f && value < 7.5f) return "6th";
                                                            else if (value >= 7.5f) return "7th";
                                                            else return "Auto";
                                                       }, nullptr);
    params.push_back (std::move (floatParam));

    floatParam = std::make_unique<AudioParameterFloat> ("useSN3D", "Normalization",
                                                        NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                                        "",
                                                        AudioProcessorParameter::genericParameter,
                                                        [](float value, int maximumStringLength)
                                                        {
                                                            if (value >= 0.5f) return "SN3D";
                                                            else return "N3D";
                                                        },
                                                        nullptr);
    params.push_back (std::move (floatParam));


    // Crossovers
    for (int i = 0; i < numFreqBands-1; ++i)
    {
        floatParam = std::make_unique<AudioParameterFloat> ("crossover" + String (i),
                                                            "Crossover " + String (i),
                                                            NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.4f),
                                                            crossoverPresets[i], "Hz",
                                                            AudioProcessorParameter::genericParameter,
                                                            std::function <String (float value, int maximumStringLength)> ([](float v, int m) {return String (v, m);}),
                                                            std::function< float (const String &text)> ([](const String &t){return t.getFloatValue();}));
        params.push_back (std::move (floatParam));
    }


    for (int i = 0; i < numFreqBands; ++i)
    {
        // compressor threshold
        floatParam = std::make_unique<AudioParameterFloat>("threshold" + String (i),
                                                           "Threshold " + String (i),
                                                           NormalisableRange<float> (-50.0f, 10.0f, 0.1f),
                                                           -10.0f, "dB",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String (v, 1);}),
                                                           std::function< float (const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back (std::move (floatParam));

        // knee
        floatParam = std::make_unique<AudioParameterFloat>("knee" + String (i),
                                                           "Knee Width " + String (i),
                                                           NormalisableRange<float> (0.0f, 30.0f, 0.1f),
                                                           0.0f, "dB",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String (v, 1);}),
                                                           std::function< float (const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back (std::move (floatParam));

        // attack
        floatParam = std::make_unique<AudioParameterFloat>("attack" + String (i),
                                                           "Attack Time " + String (i),
                                                           NormalisableRange<float> (0.0f, 100.0f, 0.1f),
                                                           30.0f, "ms",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String (v, 1);}),
                                                           std::function< float (const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back (std::move (floatParam));

        // release
        floatParam = std::make_unique<AudioParameterFloat>("release" + String (i),
                                                           "Release Time " + String (i),
                                                           NormalisableRange<float> (0.0f, 500.0f, 0.1f),
                                                           150.0f, "ms",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String (v, 1);}),
                                                           std::function< float (const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back (std::move (floatParam));

        // ratio
        floatParam = std::make_unique<AudioParameterFloat>("ratio" + String (i),
                                                           "Ratio " + String (i),
                                                           NormalisableRange<float> (1.0f, 16.0f, 0.1f),
                                                           4.0f, " : 1",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){
                                                             return (v > 15.9f) ? String ("inf") : String (v, 1);}),
                                                           std::function< float (const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back (std::move (floatParam));

        // makeUpGain
        floatParam = std::make_unique<AudioParameterFloat>("makeUpGain" + String (i),
                                                           "MakUp Gain " + String (i),
                                                           NormalisableRange<float> (-10.0f, 20.0f, 0.1f),
                                                           0.0f, "dB",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String (v, 1);}),
                                                           std::function< float (const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back (std::move (floatParam));


        auto boolParam = std::make_unique<AudioParameterBool>("bypass" + String (i),
                                                           "Bypass compression on band " + String (i),
                                                           false);
        params.push_back (std::move (boolParam));


        boolParam = std::make_unique<AudioParameterBool>("solo" + String (i),
                                                           "Solo band " + String (i),
                                                           false);
        params.push_back (std::move (boolParam));
    }

    return params;
}

void MultiBandCompressorAudioProcessor::calculateCoefficients (const int i)
{
    jassert (lastSampleRate > 0.0);

    const float crossoverFrequency = jmin (static_cast<float> (0.5 * lastSampleRate), *crossovers[i]);

    double b0, b1, b2, a0, a1, a2;
    double K = std::tan (MathConstants<double>::pi * (crossoverFrequency) / lastSampleRate);
    double den = 1 + MathConstants<double>::sqrt2 * K + pow (K, double (2.0));

    // calculate coeffs for 2nd order Butterworth
    a0 = 1.0;
    a1 = (2*(pow (K,2.0) - 1)) / den;
    a2 = (1 - MathConstants<double>::sqrt2*K + pow (K,2.0)) / den;

    // HP
    b0 = 1.0 / den;
    b1 = -2.0 * b0;
    b2 = b0;
    iirTempHPCoefficients[i] = new IIR::Coefficients<float>(b0, b1, b2, a0, a1, a2);

    // also calculate 4th order Linkwitz-Riley for GUI
    IIR::Coefficients<double>::Ptr coeffs (new IIR::Coefficients<double>(b0, b1, b2, a0, a1, a2));
    coeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients (coeffs->coefficients, coeffs->coefficients);
    *highPassLRCoeffs[i] = *coeffs;

    // LP
    b0 = pow (K,2.0) /   den;
    b1 = 2.0 * b0;
    b2 = b0;
    iirTempLPCoefficients[i] = new IIR::Coefficients<float>(b0, b1, b2, a0, a1, a2);

    coeffs.reset();
    coeffs = (new IIR::Coefficients<double>(b0, b1, b2, a0, a1, a2));
    coeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients (coeffs->coefficients, coeffs->coefficients);
    *lowPassLRCoeffs[i] = *coeffs;

    // Allpass equivalent to 4th order Linkwitz-Riley crossover
    iirTempAPCoefficients[i] = new IIR::Coefficients<float>(a2, a1, a0, a0, a1, a2);

}

void MultiBandCompressorAudioProcessor::copyCoeffsToProcessor()
{
    for (int b = 0; b < numFreqBands-1; ++b)
    {
        *iirLPCoefficients[b] = *iirTempLPCoefficients[b]; // LP
        *iirHPCoefficients[b] = *iirTempHPCoefficients[b]; // HP
        *iirAPCoefficients[b] = *iirTempAPCoefficients[b]; // AP
    }

    userChangedFilterSettings = false;
}

//==============================================================================
int MultiBandCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultiBandCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiBandCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const String MultiBandCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultiBandCompressorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MultiBandCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *orderSetting, *orderSetting, true);

    lastSampleRate = sampleRate;

    ProcessSpec monoSpec;
    monoSpec.sampleRate = sampleRate;
    monoSpec.maximumBlockSize = samplesPerBlock;
    monoSpec.numChannels = 1;

    inputPeak = Decibels::gainToDecibels (-INFINITY);
    outputPeak = Decibels::gainToDecibels (-INFINITY);

    for (int i = 0; i < numFreqBands-1; ++i)
    {
        calculateCoefficients (i);
    }

    copyCoeffsToProcessor();

    interleaved.clear();
    for (int ch = 0; ch < maxNumFilters; ++ch)
    {
        interleaved.add (new AudioBlock<filterFloatType> (interleavedBlockData[ch], 1, samplesPerBlock));
    }

    for (int i = 0; i < numFreqBands-1; ++i)
    {
        for (int ch = 0; ch < maxNumFilters; ++ch)
        {
            iirLP[i][ch]->reset (filterFloatType (0.0f));
            iirLP2[i][ch]->reset (filterFloatType (0.0f));
            iirHP[i][ch]->reset (filterFloatType (0.0f));
            iirHP2[i][ch]->reset (filterFloatType (0.0f));
            iirAP[i][ch]->reset (filterFloatType (0.0f));
            iirLP[i][ch]->prepare (monoSpec);
            iirLP2[i][ch]->prepare (monoSpec);
            iirHP[i][ch]->prepare (monoSpec);
            iirHP2[i][ch]->prepare (monoSpec);
            iirAP[i][ch]->prepare (monoSpec);
        }
    }

    for (int i = 0; i < numFreqBands; ++i)
    {
        compressors[i].prepare (monoSpec);
        compressors[i].setThreshold (*threshold[i]);
        compressors[i].setKnee (*knee[i]);
        compressors[i].setAttackTime (*attack[i] * 0.001f);
        compressors[i].setReleaseTime (*release[i] * 0.001f);
        compressors[i].setRatio (*ratio[i] > 15.9f ? INFINITY :  *ratio[i]);
        compressors[i].setMakeUpGain (*makeUpGain[i]);

        freqBands[i].clear();
        for (int ch = 0; ch < maxNumFilters; ++ch)
        {
            freqBands[i].add (new AudioBlock<filterFloatType> (freqBandsBlocks[i][ch], 1, samplesPerBlock));
        }
    }

    zero = AudioBlock<float> (zeroData, filterRegisterSize, samplesPerBlock);
    zero.clear();

    temp = AudioBlock<float> (tempData, 64, samplesPerBlock);
    temp.clear();

    gains = AudioBlock<float> (gainData, 1, samplesPerBlock);
    gains.clear();

    tempBuffer.setSize (64, samplesPerBlock);

    repaintFilterVisualization = true;
}

void MultiBandCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiBandCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void MultiBandCompressorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    if (userChangedIOSettings)
    {
        checkInputAndOutput (this, *orderSetting, *orderSetting, false);
    }

    const int numChannels = jmin (buffer.getNumChannels(), input.getNumberOfChannels());
    ScopedNoDenormals noDenormals;

    for (int i = numChannels; i < getTotalNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }

    auto* inout = channelPointers.getData();
    const int L = buffer.getNumSamples();
    const int numSimdFilters =  1 + (numChannels - 1) / filterRegisterSize;
    gainChannelPointer = gains.getChannelPointer (0);

    tempBuffer.clear();
    gains.clear();
    zero.clear();
    temp.clear();

    // update iir filter coefficients
    if (userChangedFilterSettings.get())
        copyCoeffsToProcessor();

    inputPeak = Decibels::gainToDecibels (buffer.getMagnitude (0, 0, L));

    // Interleave
    int partial = numChannels % filterRegisterSize;
    if (partial == 0)
    {
        for (int i = 0; i < numSimdFilters; ++i)
        {
            // interleaved[i]->clear(); // broken with JUCE 5.4.5
            clear (*interleaved[i]);
            AudioDataConverters::interleaveSamples (buffer.getArrayOfReadPointers() + i* filterRegisterSize,
                                                    reinterpret_cast<float*> (interleaved[i]->getChannelPointer (0)),
                                                    L, filterRegisterSize);
        }
    }
    else
    {
        int i;
        for (i = 0; i < numSimdFilters-1; ++i)
        {
            // interleaved[i]->clear(); // broken with JUCE 5.4.5
            clear (*interleaved[i]);
            AudioDataConverters::interleaveSamples (buffer.getArrayOfReadPointers() + i* filterRegisterSize,
                                                    reinterpret_cast<float*> (interleaved[i]->getChannelPointer (0)),
                                                    L, filterRegisterSize);
        }

        const float* addr[filterRegisterSize];
        int ch;
        for (ch = 0; ch < partial; ++ch)
        {
            addr[ch] = buffer.getReadPointer (i * filterRegisterSize + ch);
        }
        for (; ch < filterRegisterSize; ++ch)
        {
            addr[ch] = zero.getChannelPointer (ch);
        }
        // interleaved[i]->clear(); // broken with JUCE 5.4.5
        clear (*interleaved[i]);
        AudioDataConverters::interleaveSamples (addr,
                                                reinterpret_cast<float*> (interleaved[i]->getChannelPointer (0)),
                                                L, filterRegisterSize);
    }


    //  filter block diagram
    //                                | ---> HP3 ---> |
    //        | ---> HP2 ---> AP1 --->|               + ---> |
    //        |                       | ---> LP3 ---> |      |
    //     -->|                                              + --->
    //        |                       | ---> HP1 ---> |      |
    //        | ---> LP2 ---> AP3 --->|               + ---> |
    //                                | ---> LP1 ---> |
    for (int i = 0; i < numSimdFilters; ++i)
    {
        const SIMDRegister<float>* chPtrInterleaved[1] = {interleaved[i]->getChannelPointer (0)};
        AudioBlock<SIMDRegister<float>> abInterleaved (const_cast<SIMDRegister<float>**> (chPtrInterleaved), 1, L);

        const SIMDRegister<float>* chPtrLow[1] = {freqBands[FrequencyBands::Low][i]->getChannelPointer (0)};
        AudioBlock<SIMDRegister<float>> abLow (const_cast<SIMDRegister<float>**> (chPtrLow), 1, L);

        const SIMDRegister<float>* chPtrMidLow[1] = {freqBands[FrequencyBands::MidLow][i]->getChannelPointer (0)};
        AudioBlock<SIMDRegister<float>> abMidLow (const_cast<SIMDRegister<float>**> (chPtrMidLow), 1, L);

        const SIMDRegister<float>* chPtrMidHigh[1] = {freqBands[FrequencyBands::MidHigh][i]->getChannelPointer (0)};
        AudioBlock<SIMDRegister<float>> abMidHigh (const_cast<SIMDRegister<float>**> (chPtrMidHigh), 1, L);

        const SIMDRegister<float>* chPtrHigh[1] = {freqBands[FrequencyBands::High][i]->getChannelPointer (0)};
        AudioBlock<SIMDRegister<float>> abHigh (const_cast<SIMDRegister<float>**> (chPtrHigh), 1, L);


        iirLP[1][i]->process (ProcessContextNonReplacing<filterFloatType> (abInterleaved, abLow));
        iirHP[1][i]->process (ProcessContextNonReplacing<filterFloatType> (abInterleaved, abHigh));

        iirLP2[1][i]->process (ProcessContextReplacing<filterFloatType> (abLow));
        iirHP2[1][i]->process (ProcessContextReplacing<filterFloatType> (abHigh));

        iirAP[2][i]->process (ProcessContextReplacing<filterFloatType> (abLow));
        iirAP[0][i]->process (ProcessContextReplacing<filterFloatType> (abHigh));

        iirHP[0][i]->process (ProcessContextNonReplacing<filterFloatType> (abLow, abMidLow));
        iirHP2[0][i]->process (ProcessContextReplacing<filterFloatType> (abMidLow));

        iirLP[0][i]->process (ProcessContextReplacing<filterFloatType> (abLow));
        iirLP2[0][i]->process (ProcessContextReplacing<filterFloatType> (abLow));

        iirLP[2][i]->process (ProcessContextNonReplacing<filterFloatType> (abHigh, abMidHigh));
        iirLP2[2][i]->process (ProcessContextReplacing<filterFloatType> (abMidHigh));

        iirHP[2][i]->process (ProcessContextReplacing<filterFloatType> (abHigh));
        iirHP2[2][i]->process (ProcessContextReplacing<filterFloatType> (abHigh));
    }


    for (int i = 0; i < numFreqBands; ++i)
    {
        if (! soloArray.isZero())
        {
            if (! soloArray[i])
            {
                maxGR[i] = 0.0f;
                maxPeak[i] = -INFINITY;
                continue;
            }
        }

        // Deinterleave
        for (int j = 0; j < numChannels; ++j)
        {
            inout[j] = temp.getChannelPointer (j);
        }

        if (partial == 0)
        {
            for (int n = 0; n < numSimdFilters; ++n)
            {
                  AudioDataConverters::deinterleaveSamples (reinterpret_cast<float*>(freqBands[i][n]->getChannelPointer (0)),
                                                           const_cast<float**>(inout + n*filterRegisterSize),
                                                           L, filterRegisterSize);
            }
        }
        else
        {
            int n;
            for (n = 0; n < numSimdFilters-1; ++n)
            {
                AudioDataConverters::deinterleaveSamples (reinterpret_cast<float*> (freqBands[i][n]->getChannelPointer (0)),
                                                          const_cast<float**>(inout + n*filterRegisterSize),
                                                          L, filterRegisterSize);
            }

            float* addr[filterRegisterSize];
            int ch;
            for (ch = 0; ch < partial; ++ch)
            {
                addr[ch] = const_cast<float*> (inout[n*filterRegisterSize + ch]);
            }
            for (; ch < filterRegisterSize; ++ch)
            {
                addr[ch] = zero.getChannelPointer (ch);
            }
            AudioDataConverters::deinterleaveSamples (reinterpret_cast<float*> (freqBands[i][n]->getChannelPointer (0)), addr, L, filterRegisterSize);
            zero.clear();
        }

        // Compress
        if (*bypass[i] < 0.5f)
        {
            compressors[i].getGainFromSidechainSignal (inout[0], gainChannelPointer, L);
            maxGR[i] = Decibels::gainToDecibels (FloatVectorOperations::findMinimum (gainChannelPointer, L)) - *makeUpGain[i];
            maxPeak[i] = compressors[i].getMaxLevelInDecibels();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                FloatVectorOperations::addWithMultiply (tempBuffer.getWritePointer (ch), inout[ch], gainChannelPointer, L);
            }
        }
        else
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                FloatVectorOperations::add (tempBuffer.getWritePointer (ch), inout[ch], L);
            }
            maxGR[i] = 0.0f;
            maxPeak[i] = Decibels::gainToDecibels (-INFINITY);
        }
    }

    outputPeak = Decibels::gainToDecibels (tempBuffer.getMagnitude (0, 0, L));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.copyFrom (ch, 0, tempBuffer, ch, 0, L);
    }
}

//==============================================================================
bool MultiBandCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MultiBandCompressorAudioProcessor::createEditor()
{
    return new MultiBandCompressorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void MultiBandCompressorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
  auto state = parameters.copyState();

  auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
  oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

  std::unique_ptr<XmlElement> xml (state.createXml());
  copyXmlToBinary (*xml, destData);
}


void MultiBandCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
void MultiBandCompressorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG ("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID.startsWith ("crossover"))
    {
        calculateCoefficients (parameterID.getLastCharacters (1).getIntValue());
        userChangedFilterSettings = true;
        repaintFilterVisualization = true;
    }
    else if (parameterID.startsWith ("threshold"))
    {
        const int compId = parameterID.getLastCharacters (1).getIntValue();
        compressors[compId].setThreshold (newValue);
        characteristicHasChanged[compId] = true;
    }
    else if (parameterID.startsWith ("knee"))
    {
        const int compId = parameterID.getLastCharacters (1).getIntValue();
        compressors[compId].setKnee (newValue);
        characteristicHasChanged[compId] = true;
    }
    else if (parameterID.startsWith ("attack"))
    {
        compressors[parameterID.getLastCharacters (1).getIntValue()].setAttackTime (newValue * 0.001f);
    }
    else if (parameterID.startsWith ("release"))
    {
        compressors[parameterID.getLastCharacters (1).getIntValue()].setReleaseTime (newValue * 0.001f);
    }
    else if (parameterID.startsWith ("ratio"))
    {
        const int compId = parameterID.getLastCharacters (1).getIntValue();
        if (newValue > 15.9f)
            compressors[compId].setRatio (INFINITY);
        else
            compressors[compId].setRatio (newValue);

        characteristicHasChanged[compId] = true;
    }
    else if (parameterID.startsWith ("makeUpGain"))
    {
        const int compId = parameterID.getLastCharacters (1).getIntValue();
        compressors[compId].setMakeUpGain (newValue);
        characteristicHasChanged[compId] = true;
    }
    else if (parameterID.startsWith ("solo"))
    {
        if (newValue >= 0.5f)
            soloArray.setBit (parameterID.getLastCharacters (1).getIntValue());
        else
            soloArray.clearBit (parameterID.getLastCharacters (1).getIntValue());
    }
    else if (parameterID == "orderSetting")
    {
        userChangedIOSettings = true;
    }

}

void MultiBandCompressorAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiBandCompressorAudioProcessor();
}

inline void MultiBandCompressorAudioProcessor::clear (AudioBlock<filterFloatType>& ab)
{
    const int N = static_cast<int> (ab.getNumSamples()) * filterRegisterSize;
    const int nCh = static_cast<int> (ab.getNumChannels());

    for (int ch = 0; ch < nCh; ++ch)
        FloatVectorOperations::clear (reinterpret_cast<float*> (ab.getChannelPointer (ch)), N);
}
