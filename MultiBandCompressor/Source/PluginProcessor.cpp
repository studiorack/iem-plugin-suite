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
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels (64), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels (64), true)
                     #endif
                       ),
       parameters (*this, nullptr, "PARAMETERS", createParameterLayout()),
       oscParams (parameters), maxNumFilters (ceil (64 / filterRegisterSize))
#endif
{  
    const String inputSettingID = "orderSetting";
    orderSetting = parameters.getRawParameterValue (inputSettingID);
    parameters.addParameterListener (inputSettingID, this);
    oscParams.addParameterID(inputSettingID);
  
  
    for (int i = 0; i < numFreqBands-1; ++i)
    {
        const String crossoverID("crossover" + String(i));
      
        crossovers[i] = parameters.getRawParameterValue(crossoverID);
      
        lowPassLRCoeffs[i] = IIR::Coefficients<double>::makeLowPass(lastSampleRate, *crossovers[i]);
        highPassLRCoeffs[i] = IIR::Coefficients<double>::makeHighPass(lastSampleRate, *crossovers[i]);
      
        calculateCoefficients(i);
      
        iirLPCoefficients[i] = IIR::Coefficients<float>::makeLowPass(lastSampleRate, *crossovers[i]);
        iirHPCoefficients[i] = IIR::Coefficients<float>::makeHighPass(lastSampleRate, *crossovers[i]);
        iirAPCoefficients[i] = IIR::Coefficients<float>::makeAllPass(lastSampleRate, *crossovers[i]);
      
        parameters.addParameterListener(crossoverID, this);
        oscParams.addParameterID(crossoverID);
      
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
      
        const String thresholdID("threshold" + String(i));
        const String kneeID("knee" + String(i));
        const String attackID("attack" + String(i));
        const String releaseID("release" + String(i));
        const String ratioID("ratio" + String(i));
        const String makeUpGainID("makeUpGain" + String(i));
        const String bypassID("bypass" + String(i));
        const String soloID("solo" + String(i));
      
        threshold[i] = parameters.getRawParameterValue(thresholdID);
        knee[i] = parameters.getRawParameterValue(kneeID);
        attack[i] = parameters.getRawParameterValue(attackID);
        release[i] = parameters.getRawParameterValue(releaseID);
        ratio[i] = parameters.getRawParameterValue(ratioID);
        makeUpGain[i] = parameters.getRawParameterValue(makeUpGainID);
        bypass[i] = parameters.getRawParameterValue(bypassID);
        soloArray.clear();

        parameters.addParameterListener(thresholdID, this);
        parameters.addParameterListener(kneeID, this);
        parameters.addParameterListener(attackID, this);
        parameters.addParameterListener(releaseID, this);
        parameters.addParameterListener(ratioID, this);
        parameters.addParameterListener(makeUpGainID, this);
        parameters.addParameterListener(bypassID, this);
        parameters.addParameterListener(soloID, this);

        oscParams.addParameterID(thresholdID);
        oscParams.addParameterID(kneeID);
        oscParams.addParameterID(attackID);
        oscParams.addParameterID(releaseID);
        oscParams.addParameterID(ratioID);
        oscParams.addParameterID(makeUpGainID);
        oscParams.addParameterID(bypassID);
        oscParams.addParameterID(soloID);
    }
  
    copyCoeffsToProcessor();

    for (int ch = 0; ch < maxNumFilters; ++ch)
    {
        interleavedBlockData.push_back (HeapBlock<char> ());
    }
  
    oscReceiver.addListener (this);
}

MultiBandCompressorAudioProcessor::~MultiBandCompressorAudioProcessor()
{
}

ParameterLayout MultiBandCompressorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    const float crossoverPresets [numFreqBands-1] = { 80.0f, 440.0f, 2200.0f };
  
    std::unique_ptr<AudioParameterFloat> floatParam;
    std::unique_ptr<AudioParameterBool> boolParam;

    floatParam = std::make_unique<AudioParameterFloat> ("orderSetting",
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
        floatParam = std::make_unique<AudioParameterFloat> ("crossover" + String(i),
                                                            "Crossover " + String(i),
                                                            NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.4f),
                                                            crossoverPresets[i], "Hz",
                                                            AudioProcessorParameter::genericParameter,
                                                            std::function <String (float value, int maximumStringLength)> ([](float v, int m) {return String (v, m);}),
                                                            std::function< float(const String &text)> ([](const String &t){return t.getFloatValue();}));
        params.push_back( std::move (floatParam));
    }


    for (int i = 0; i < numFreqBands; ++i)
    {
        // compressor threshold
        floatParam = std::make_unique<AudioParameterFloat>("threshold" + String(i),
                                                           "Threshold " + String(i),
                                                           NormalisableRange<float> (-50.0f, 10.0f, 0.1f),
                                                           -10.0f, "dB",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String(v, 1);}),
                                                           std::function< float(const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back( std::move (floatParam));
      
        // knee
        floatParam = std::make_unique<AudioParameterFloat>("knee" + String(i),
                                                           "Knee Width " + String(i),
                                                           NormalisableRange<float> (0.0f, 30.0f, 0.1f),
                                                           0.0f, "dB",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String(v, 1);}),
                                                           std::function< float(const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back( std::move (floatParam));
      
        // attack
        floatParam = std::make_unique<AudioParameterFloat>("attack" + String(i),
                                                           "Attack Time " + String(i),
                                                           NormalisableRange<float> (0.0f, 100.0f, 0.1f),
                                                           30.0f, "ms",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String(v, 1);}),
                                                           std::function< float(const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back( std::move (floatParam));

        // release
        floatParam = std::make_unique<AudioParameterFloat>("release" + String(i),
                                                           "Release Time " + String(i),
                                                           NormalisableRange<float> (0.0f, 500.0f, 0.1f),
                                                           150.0f, "ms",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String(v, 1);}),
                                                           std::function< float(const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back( std::move (floatParam));
      
        // ratio
        floatParam = std::make_unique<AudioParameterFloat>("ratio" + String(i),
                                                           "Ratio " + String(i),
                                                           NormalisableRange<float> (1.0f, 16.0f, 0.1f),
                                                           4.0f, " : 1",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){
                                                             return (v > 15.9f) ? String("inf") : String(v, 1);}),
                                                           std::function< float(const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back( std::move (floatParam));
      
        // makeUpGain
        floatParam = std::make_unique<AudioParameterFloat>("makeUpGain" + String(i),
                                                           "MakUp Gain " + String(i),
                                                           NormalisableRange<float> (-10.0f, 20.0f, 0.1f),
                                                           0.0f, "dB",
                                                           AudioProcessorParameter::genericParameter,
                                                           std::function <String (float value, int maximumStringLength)> ([](float v, int m){return String(v, 1);}),
                                                           std::function< float(const String &text)> ([](const String &t){return t.getFloatValue();})
                                                          );
        params.push_back( std::move (floatParam));
      
      
        boolParam = std::make_unique<AudioParameterBool>("bypass" + String(i),
                                                           "Compression on band " + String(i) + " enabled",
                                                           false);
        params.push_back( std::move (boolParam));

      
        boolParam = std::make_unique<AudioParameterBool>("solo" + String(i),
                                                           "Put band " + String(i) + " in Solo mode",
                                                           false);
        params.push_back( std::move (boolParam));
    }
  
    return { params.begin(), params.end() };
}

void MultiBandCompressorAudioProcessor::calculateCoefficients(const int i)
{
    jassert (lastSampleRate > 0.0);
    jassert ((*crossovers[i]) > 0 && (*crossovers[i]) <= static_cast<float> (lastSampleRate * 0.5));
  
    double b0, b1, b2, a0, a1, a2;
    double K = std::tan (MathConstants<double>::pi * (*crossovers[i]) / lastSampleRate);
    double Q = 1 / MathConstants<double>::sqrt2;
    double den = Q*pow(K, double(2.0)) + K + Q;
  
    // calculate coeffs for 2nd order Butterworth
    a0 = 1.0;
    a1 = (2*Q*(pow(K,2.0) - 1)) / den;
    a2 = (Q*pow(K,2.0) - K + Q) / den;
  
    // HP
    b0 = Q / den;
    b1 = -2.0 * b0;
    b2 = b0;
    iirTempHPCoefficients[i] = new IIR::Coefficients<float>(b0, b1, b2, a0, a1, a2);
  
    // also calculate 4th order Linkwitz-Riley for GUI
    IIR::Coefficients<double>::Ptr coeffs(new IIR::Coefficients<double>(b0, b1, b2, a0, a1, a2));
    coeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients(coeffs->coefficients, coeffs->coefficients);
    *highPassLRCoeffs[i] = *coeffs;
  
    // LP
    b0 = b0 * pow(K,2.0);
    b1 = 2.0 * b0;
    b2 = b0;
    iirTempLPCoefficients[i] = new IIR::Coefficients<float>(b0, b1, b2, a0, a1, a2);
  
    coeffs.reset();
    coeffs = (new IIR::Coefficients<double>(b0, b1, b2, a0, a1, a2));
    coeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients(coeffs->coefficients, coeffs->coefficients);
    *lowPassLRCoeffs[i] = *coeffs;
  
    // Allpass equivalent to 4th order Linkwitz-Riley
    double re = -(a1/2.0);
    double im = sqrt(abs(pow(-re,2.0) - a2));
    a0 = 1.0;
    a1 = -2.0 * re;
    a2 = pow(re,2.0) + pow(im,2.0);
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
const String MultiBandCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiBandCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultiBandCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultiBandCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultiBandCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

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
    numChannels = jmin (getTotalNumInputChannels(), input.getNumberOfChannels());

    ProcessSpec monoSpec;
    monoSpec.sampleRate = sampleRate;
    monoSpec.maximumBlockSize = samplesPerBlock;
    monoSpec.numChannels = 1;
  
    inputPeak = Decibels::gainToDecibels (-INFINITY);
    outputPeak = Decibels::gainToDecibels (-INFINITY);
  
    for (int i = 0; i < numFreqBands-1; ++i)
    {
        calculateCoefficients(i);
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
            iirLP[i][ch]->reset(filterFloatType (0.0f));
            iirLP2[i][ch]->reset(filterFloatType (0.0f));
            iirHP[i][ch]->reset(filterFloatType (0.0f));
            iirHP2[i][ch]->reset(filterFloatType (0.0f));
            iirAP[i][ch]->reset(filterFloatType (0.0f));
            iirLP[i][ch]->prepare(monoSpec);
            iirLP2[i][ch]->prepare(monoSpec);
            iirHP[i][ch]->prepare(monoSpec);
            iirHP2[i][ch]->prepare(monoSpec);
            iirAP[i][ch]->prepare(monoSpec);
        }
    }
  
    for (int i = 0; i < numFreqBands; ++i)
    {
        compressors[i].prepare(monoSpec);
        compressors[i].setThreshold(*threshold[i]);
        compressors[i].setKnee(*knee[i]);
        compressors[i].setAttackTime(*attack[i] * 0.001f);
        compressors[i].setReleaseTime(*release[i] * 0.001f);
        compressors[i].setRatio(*ratio[i] > 15.9f ? INFINITY :  *ratio[i]);
        compressors[i].setMakeUpGain(*makeUpGain[i]);
      
        freqBands[i].clear();
        for (int ch = 0; ch < maxNumFilters; ++ch)
        {
            freqBands[i].add(new AudioBlock<filterFloatType> (freqBandsBlocks[i][ch], 1, samplesPerBlock));
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
        numChannels = jmin(buffer.getNumChannels(), input.getSize());
    }
  
    ScopedNoDenormals noDenormals;
    
    for (int i = numChannels; i < getTotalNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }

    auto* inout = channelPointers.getData();
    const int L = buffer.getNumSamples();
    const int numSimdFilters =  1 + (numChannels - 1) / filterRegisterSize;
    gainChannelPointer = gains.getChannelPointer(0);
  
    tempBuffer.clear();
    gains.clear();
    zero.clear();
    temp.clear();
    
    // update iir filter coefficients
    if (userChangedFilterSettings.get())
    {
        copyCoeffsToProcessor();
    }

    inputPeak = Decibels::gainToDecibels (buffer.getMagnitude (0, 0, L));

    // Interleave
    int partial = numChannels % filterRegisterSize;
    if (partial == 0)
    {
        for (int i = 0; i < numSimdFilters; ++i)
        {
            interleaved[i]->clear();
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
            interleaved[i]->clear();
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
            addr[ch] = zero.getChannelPointer(ch);
        }
        interleaved[i]->clear();
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
        iirLP[1][i]->process (ProcessContextNonReplacing<filterFloatType> (*interleaved[i], *freqBands[FrequencyBands::Low][i]));
        iirHP[1][i]->process (ProcessContextNonReplacing<filterFloatType> (*interleaved[i], *freqBands[FrequencyBands::High][i]));
        iirLP2[1][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::Low][i]));
        iirHP2[1][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::High][i]));

        iirAP[2][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::Low][i]));
        iirAP[0][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::High][i]));

        iirHP[0][i]->process (ProcessContextNonReplacing<filterFloatType> (*freqBands[FrequencyBands::Low][i], *freqBands[FrequencyBands::MidLow][i]));
        iirHP2[0][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::MidLow][i]));

        iirLP[0][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::Low][i]));
        iirLP2[0][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::Low][i]));

        iirLP[2][i]->process (ProcessContextNonReplacing<filterFloatType> (*freqBands[FrequencyBands::High][i], *freqBands[FrequencyBands::MidHigh][i]));
        iirLP2[2][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::MidHigh][i]));

        iirHP[2][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::High][i]));
        iirHP2[2][i]->process (ProcessContextReplacing<filterFloatType> (*freqBands[FrequencyBands::High][i]));

    }
  
  
    for (int i = 0; i < numFreqBands; ++i)
    {
        if (!(soloArray.empty()))
        {
            if (!(soloArray.count(i)))
            {
                maxGR[i] = 0.0f;
                maxPeak[i] = Decibels::gainToDecibels (-INFINITY);
                continue;
            }
        }
      
        // Deinterleave
        for (int j = 0; j < numChannels; ++j)
        {
            inout[j] = temp.getChannelPointer(j);
        }
      
        if (partial == 0)
        {
            for (int n = 0; n < numSimdFilters; ++n)
            {
                  AudioDataConverters::deinterleaveSamples(reinterpret_cast<float*>(freqBands[i][n]->getChannelPointer(0)),
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
            compressors[i].getGainFromSidechainSignal(inout[0], gainChannelPointer, L);
            maxGR[i] = Decibels::gainToDecibels(FloatVectorOperations::findMinimum(gainChannelPointer, L)) - *makeUpGain[i];
            maxPeak[i] = compressors[i].getMaxLevelInDecibels();
          
            for (int ch = 0; ch < numChannels; ++ch)
            {
                FloatVectorOperations::addWithMultiply(tempBuffer.getWritePointer(ch), inout[ch], gainChannelPointer, L);
            }
        }
        else
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                FloatVectorOperations::add(tempBuffer.getWritePointer(ch), inout[ch], L);
            }
            maxGR[i] = 0.0f;
            maxPeak[i] = Decibels::gainToDecibels (-INFINITY);
        }
    }
  
    outputPeak = Decibels::gainToDecibels (tempBuffer.getMagnitude (0, 0, L));
  
    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.copyFrom(ch, 0, tempBuffer, ch, 0, L);
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    state.setProperty ("OSCPort", var (oscReceiver.getPortNumber()), nullptr);
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}


void MultiBandCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort"))
            {
                oscReceiver.connect (parameters.state.getProperty ("OSCPort", var (-1)));
            }
        }
}

//==============================================================================
void MultiBandCompressorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG ("Parameter with ID " << parameterID << " has changed. New value: " << newValue);
  
    if (parameterID.startsWith("crossover"))
    {
        calculateCoefficients(parameterID.getLastCharacters(1).getIntValue());
        userChangedFilterSettings = true;
        repaintFilterVisualization = true;
    }
    else if (parameterID.startsWith("threshold"))
    {
        compressors[parameterID.getLastCharacters(1).getIntValue()].setThreshold(newValue);
    }
    else if (parameterID.startsWith("knee"))
    {
        compressors[parameterID.getLastCharacters(1).getIntValue()].setKnee(newValue);
    }
    else if (parameterID.startsWith("attack"))
    {
        compressors[parameterID.getLastCharacters(1).getIntValue()].setAttackTime(newValue * 0.001f);
    }
    else if (parameterID.startsWith("release"))
    {
        compressors[parameterID.getLastCharacters(1).getIntValue()].setReleaseTime(newValue * 0.001f);
    }
    else if (parameterID.startsWith("ratio"))
    {
        if (newValue > 15.9f)
            compressors[parameterID.getLastCharacters(1).getIntValue()].setRatio(INFINITY);
        else
            compressors[parameterID.getLastCharacters(1).getIntValue()].setRatio(newValue);
    }
    else if (parameterID.startsWith("makeUpGain"))
    {
        compressors[parameterID.getLastCharacters(1).getIntValue()].setMakeUpGain(newValue);
    }
    else if (parameterID.startsWith("solo"))
    {
        if (newValue >= 0.5f)
        {
            soloArray.insert(parameterID.getLastCharacters(1).getIntValue());
        }
        else
        {
            soloArray.erase(parameterID.getLastCharacters(1).getIntValue());
        }
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
pointer_sized_int MultiBandCompressorAudioProcessor::handleVstPluginCanDo (int32 index,
                                                                     pointer_sized_int value, void* ptr, float opt)
{
    auto text = (const char*) ptr;
    auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

    if (matches ("wantsChannelCountNotifications"))
        return 1;
    return 0;
}

//==============================================================================
void MultiBandCompressorAudioProcessor::oscMessageReceived (const OSCMessage &message)
{
    String prefix ("/" + String (JucePlugin_Name));
    if (! message.getAddressPattern().toString().startsWith (prefix))
        return;

    OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring (String (JucePlugin_Name).length() + 1));

    oscParams.processOSCMessage (msg);
}

void MultiBandCompressorAudioProcessor::oscBundleReceived (const OSCBundle &bundle)
{
    for (int i = 0; i < bundle.size(); ++i)
    {
        auto elem = bundle[i];
        if (elem.isMessage())
            oscMessageReceived (elem.getMessage());
        else if (elem.isBundle())
            oscBundleReceived (elem.getBundle());
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiBandCompressorAudioProcessor();
}
