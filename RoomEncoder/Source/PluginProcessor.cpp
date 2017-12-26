/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 http://www.iem.at
 
 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RoomEncoderAudioProcessor::RoomEncoderAudioProcessor()
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
parameters (*this, nullptr)
{
    parameters.createAndAddParameter ("directivityOrderSetting", "Input Directivity Order", "",
                                      NormalisableRange<float> (0.0f, 8.0f, 1.0f), 1.0f,
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
    
    parameters.createAndAddParameter ("orderSetting", "Output Ambisonics Order", "",
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
    parameters.createAndAddParameter ("useSN3D", "Normalization", "",
                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                      [](float value) { if (value >= 0.5f ) return "SN3D";
                                          else return "N3D";},
                                      nullptr);
    
    parameters.createAndAddParameter("roomX", "room size x", "m",
                                     NormalisableRange<float> (1.0f, 30.0f, 0.01f), 10.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("roomY", "room size y", "m",
                                     NormalisableRange<float> (1.0f, 30.0f, 0.01f), 11.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("roomZ", "room size z", "m",
                                     NormalisableRange<float> (1.0f, 20.0f, 0.01f), 7.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("sourceX", "source position x", "m",
                                     NormalisableRange<float> (-15.0f, 15.0f, 0.0001f), 1.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("sourceY", "source position y", "m",
                                     NormalisableRange<float> (-15.0f, 15.0f, 0.001f), 1.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("sourceZ", "source position z", "m",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.001f), 1.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("listenerX", "listener position x", "m",
                                     NormalisableRange<float> (-15.0f, 15.0f, 0.001f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("listenerY", "listener position y", "m",
                                     NormalisableRange<float> (-15.0f, 15.0f, 0.001f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("listenerZ", "listener position z", "m",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.001f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("numRefl", "number of reflections", "",
                                     NormalisableRange<float> (0.0f, nImgSrc-1, 1.0f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("lowShelfFreq", "LowShelf Frequency", "Hz",
                                     NormalisableRange<float> (20.0f, 20000.0f, 1.0f), 100.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("lowShelfGain", "LowShelf Gain", "dB",
                                     NormalisableRange<float> (-15.0f, 5.0f, 0.1f), -5.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("highShelfFreq", "HighShelf Frequency", "Hz",
                                     NormalisableRange<float> (20., 20000.0f, 1.0f), 8000.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("highShelfGain", "HighShelf Gain", "dB",
                                     NormalisableRange<float> (-15.0f, 5.0f, 0.1f), -5.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("reflCoeff", "Reflection Coefficient", "",
                                     NormalisableRange<float> (-15.0f, 0.0f, 0.01f), -1.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter ("syncChannel", "Synchronize to Channel", "",
                                      NormalisableRange<float> (0.0f, 4.0f, 1.0f), 0.0f,
                                      [](float value) {
                                          if (value >= 0.5f && value < 1.5f) return "Channel 1";
                                          else if (value >= 1.5f && value < 2.5f) return "Channel 2";
                                          else if (value >= 2.5f && value < 3.5f) return "Channel 3";
                                          else if (value >= 3.5f) return "Channel 4";
                                          else return "None";},
                                      nullptr);
    
    parameters.createAndAddParameter ("syncRoomSize", "Synchronize Room Dimensions", "",
                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                      [](float value) {
                                          if (value >= 0.5f) return "YES";
                                          else return "NO";},
                                      nullptr);
    
    parameters.createAndAddParameter ("syncReflection", "Synchronize Reflection Properties", "",
                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                      [](float value) {
                                          if (value >= 0.5f) return "YES";
                                          else return "NO";},
                                      nullptr);
    
    parameters.createAndAddParameter ("syncListener", "Synchronize Listener Position", "",
                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                      [](float value) {
                                          if (value >= 0.5f) return "YES";
                                          else return "NO";},
                                      nullptr);
    
    
    parameters.state = ValueTree (Identifier ("RoomEncoder"));
    
    directivityOrderSetting = parameters.getRawParameterValue ("directivityOrderSetting");
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    
    roomX = parameters.getRawParameterValue ("roomX");
    roomY = parameters.getRawParameterValue ("roomY");
    roomZ = parameters.getRawParameterValue ("roomZ");
    
    sourceX = parameters.getRawParameterValue ("sourceX");
    sourceY = parameters.getRawParameterValue ("sourceY");
    sourceZ = parameters.getRawParameterValue ("sourceZ");
    listenerX = parameters.getRawParameterValue ("listenerX");
    listenerY = parameters.getRawParameterValue ("listenerY");
    listenerZ = parameters.getRawParameterValue ("listenerZ");
    numRefl = parameters.getRawParameterValue ("numRefl");
    reflCoeff = parameters.getRawParameterValue ("reflCoeff");
    
    syncChannel = parameters.getRawParameterValue ("syncChannel");
    syncRoomSize = parameters.getRawParameterValue ("syncRoomSize");
    syncReflection = parameters.getRawParameterValue ("syncReflection");
    syncListener = parameters.getRawParameterValue ("syncListener");
    
    lowShelfFreq = parameters.getRawParameterValue ("lowShelfFreq");
    lowShelfGain = parameters.getRawParameterValue ("lowShelfGain");
    highShelfFreq = parameters.getRawParameterValue ("highShelfFreq");
    highShelfGain = parameters.getRawParameterValue ("highShelfGain");
    
    parameters.addParameterListener ("directivityOrderSetting", this);
    parameters.addParameterListener ("orderSetting", this);
    parameters.addParameterListener ("lowShelfFreq", this);
    parameters.addParameterListener ("lowShelfGain", this);
    parameters.addParameterListener ("highShelfFreq", this);
    parameters.addParameterListener ("highShelfGain", this);
    parameters.addParameterListener ("reflCoeff", this);
    parameters.addParameterListener ("numRefl", this);
    parameters.addParameterListener ("listenerX", this);
    parameters.addParameterListener ("listenerY", this);
    parameters.addParameterListener ("listenerZ", this);
    parameters.addParameterListener ("roomX", this);
    parameters.addParameterListener ("roomY", this);
    parameters.addParameterListener ("roomZ", this);
    
    t = *roomX;
    b = *roomY;
    h = *roomZ;
    
    _numRefl = 0;
    
    sourcePos = Vector3D<float>(*sourceX, *sourceY, *sourceZ);
    listenerPos = Vector3D<float>(*listenerX, *listenerY, *listenerZ);
    
    for (int i = 0; i<nImgSrc;++i) {
        oldDelay[i] = 44100/343.2f*interpMult; //init oldRadius
        allGains[i] = 0.0f;
        FloatVectorOperations::clear(SHcoeffsOld[i], 64);
        FloatVectorOperations::clear((float *) &SHsampleOld[i], 64);
    }
    
    lowShelfCoefficients = IIR::Coefficients<float>::makeLowShelf(48000, *lowShelfFreq, 0.707f, Decibels::decibelsToGain(*lowShelfGain));
    highShelfCoefficients = IIR::Coefficients<float>::makeHighShelf(48000, *highShelfFreq, 0.707f, Decibels::decibelsToGain(*highShelfGain));
    
    startTimer(50);
}

RoomEncoderAudioProcessor::~RoomEncoderAudioProcessor()
{
}

//==============================================================================
const String RoomEncoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RoomEncoderAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool RoomEncoderAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double RoomEncoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RoomEncoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int RoomEncoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RoomEncoderAudioProcessor::setCurrentProgram (int index)
{
}

const String RoomEncoderAudioProcessor::getProgramName (int index)
{
    return {};
}

void RoomEncoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void RoomEncoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    bufferSize = round(180.0/343.2*sampleRate) + samplesPerBlock + 100;
    bufferSize += samplesPerBlock - bufferSize%samplesPerBlock;
    
    delayBuffer.setSize(nChOutput, bufferSize);
    delayBuffer.clear();
    delayBufferWritePtrArray = delayBuffer.getArrayOfWritePointers();

    checkOrderUpdateBuffers(roundFloatToInt(*directivityOrderSetting)-1, roundFloatToInt(*orderSetting)-1);
    
    dist2smpls = sampleRate/343.2f*interpMult; //factor 128 is a small hack for Lagrange lookuptable
    
    readOffset = 0;
    bufferReadIdx = 0;
    
    monoBuffer.setSize(1, bufferSize);
    monoBuffer.clear();
    
    lowShelfArray.clear();
    highShelfArray.clear();
    interleavedData.clear();
    
    for (int i = 0; i<16; ++i)
    {
        lowShelfArray.add(new IIR::Filter<juce::dsp::SIMDRegister<float>>(lowShelfCoefficients));
        lowShelfArray.getLast()->reset();
        highShelfArray.add(new IIR::Filter<juce::dsp::SIMDRegister<float>>(highShelfCoefficients));
        highShelfArray.getLast()->reset();
        lowShelfArray2.add(new IIR::Filter<juce::dsp::SIMDRegister<float>>(lowShelfCoefficients));
        lowShelfArray2.getLast()->reset();
        highShelfArray2.add(new IIR::Filter<juce::dsp::SIMDRegister<float>>(highShelfCoefficients));
        highShelfArray2.getLast()->reset();
        interleavedData.add(new AudioBlock<SIMDRegister<float>> (interleavedBlockData[i], 1, samplesPerBlock));
        interleavedData.getLast()->clear();
    }
    zero = AudioBlock<float> (zeroData, SIMDRegister<float>::size(), samplesPerBlock);
    zero.clear();
    
    if (editorFv != nullptr)
        editorFv->setSampleRate(sampleRate);
}

void RoomEncoderAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RoomEncoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif


void RoomEncoderAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "orderSetting" || parameterID == "directivityOrderSetting") userChangedOrderSettings = true;
    else if (parameterID == "reflCoeff") {
        if (editorFv != nullptr) editorFv->setOverallGainInDecibels(*reflCoeff);
        updateFv = true;
    }
    else if (parameterID == "lowShelfFreq" || parameterID == "lowShelfGain" ||
        parameterID == "highShelfFreq" || parameterID == "highShelfGain") userChangedFilterSettings = true;
    
    if (*syncChannel >= 0.5f && !readingSharedParams)
    {
        int ch = (int) *syncChannel;
        RoomParams& roomParam = sharedParams.get().rooms.getReference(ch);
        
        bool sRoom(*syncRoomSize>=0.5f);
        bool sListener(*syncListener>=0.5f);
        bool sReflections(*syncReflection>=0.5f);

        if (sRoom && (parameterID == "roomX" || parameterID == "roomY" || parameterID == "roomZ"))
        {
            roomParam.roomX = *roomX;
            roomParam.roomY = *roomY;
            roomParam.roomZ = *roomZ;
            roomParam.validRoomData = true;
        }
        else if (sListener && (parameterID == "listenerX" || parameterID == "listenerY" || parameterID == "listenerZ"))
        {
            roomParam.listenerX = *listenerX;
            roomParam.listenerY = *listenerY;
            roomParam.listenerZ = *listenerZ;
            roomParam.validListenerData = true;
        }
        else if (sReflections && (parameterID == "reflCoeff" || parameterID == "numRefl" ||
                                  parameterID == "lowShelfFreq" || parameterID == "lowShelfGain" ||
                                  parameterID == "highShelfFreq" || parameterID == "highShelfGain"))
        {
            roomParam.reflCoeff = *reflCoeff;
            roomParam.numRefl = *numRefl;
            roomParam.lowShelfFreq = *lowShelfFreq;
            roomParam.lowShelfGain = *lowShelfGain;
            roomParam.highShelfFreq = *highShelfFreq;
            roomParam.highShelfGain = *highShelfGain;
            
            roomParam.validReflectionData = true;
        }

    }
}

void RoomEncoderAudioProcessor::updateFilterCoefficients(int sampleRate) {
    *lowShelfCoefficients = *IIR::Coefficients<float>::makeLowShelf(sampleRate, *lowShelfFreq, 0.707f, Decibels::decibelsToGain(*lowShelfGain));
    *highShelfCoefficients = *IIR::Coefficients<float>::makeHighShelf(sampleRate, *highShelfFreq, 0.707f, Decibels::decibelsToGain(*highShelfGain));
    userChangedFilterSettings = false;
    if (editorFv != nullptr)
        updateFv = true;
}

void RoomEncoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON); // alternative?: fesetenv(FE_DFL_DISABLE_SSE_DENORMS_ENV);
    
    if (userChangedOrderSettings) checkOrderUpdateBuffers(roundFloatToInt(*directivityOrderSetting)-1, roundFloatToInt(*orderSetting)-1);
    
    // =============================== settings and parameters
    int maxNChIn = nChInput;
    if (buffer.getNumChannels() < nChInput) maxNChIn = buffer.getNumChannels();
    int maxNChOut = nChOutput;
    if (buffer.getNumChannels() < nChOutput) maxNChOut = buffer.getNumChannels();
    
    const int sampleRate = getSampleRate();
    const int L = buffer.getNumSamples();
    const float oneOverL = 1.0/((double) L);
    
    float* pBufferWrite = buffer.getWritePointer(0);
    const float* pBufferRead = buffer.getReadPointer(0);
    
    const int nSIMDFilters = 1 + (maxNChIn-1)/SIMDRegister<float>::size();
    
    // update iir filter coefficients
    if (userChangedFilterSettings) updateFilterCoefficients(sampleRate);
    
    
    //interleave input data
    int partial = maxNChIn%SIMDRegister<float>::size();
    if (partial == 0)
    {
        for (int i = 0; i<nSIMDFilters; ++i)
        {
            AudioDataConverters::interleaveSamples(buffer.getArrayOfReadPointers() + i*SIMDRegister<float>::size(),
                                                   reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)), L,
                                                   static_cast<int> (SIMDRegister<float>::size()));
        }
    }
    else
    {
        int i;
        for (i = 0; i<nSIMDFilters-1; ++i)
        {
            AudioDataConverters::interleaveSamples(buffer.getArrayOfReadPointers() + i*SIMDRegister<float>::size(),
                                                   reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)), L,
                                                   static_cast<int> (SIMDRegister<float>::size()));
        }
        
        const float* addr[4];
        size_t ch;
        for (ch = 0; ch < partial; ++ch)
        {
            addr[ch] = buffer.getReadPointer(i*SIMDRegister<float>::size() + ch);
        }
        for (; ch < SIMDRegister<float>::size(); ++ch)
        {
            addr[ch] = zero.getChannelPointer(ch);
        }
        AudioDataConverters::interleaveSamples(addr,
                                               reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)), L,
                                               static_cast<int> (SIMDRegister<float>::size()));
    }
    
    
    
    
    int currNumRefl = roundToInt(*numRefl);
    int workingNumRefl = (currNumRefl < _numRefl) ? _numRefl : currNumRefl;
    
    
    // calculating reflection coefficients (only if parameter changed)
    float reflCoeffGain = Decibels::decibelsToGain(*reflCoeff);
    if (powReflCoeff[1] != reflCoeffGain)
    {
        powReflCoeff[0] = 1;
        powReflCoeff[1] = reflCoeffGain;
        for (int i = 2; i<=maxOrderImgSrc; ++i )
        {
            powReflCoeff[i] = powReflCoeff[i-1] * reflCoeffGain;
        }
    }
    
    // ======================================== LIMIT MOVING SPEED OF SOURCE AND LISTENER
    float maxDist = 30.0 / sampleRate * L; // 30 meters per second
    {
        Vector3D<float> posDiff;
        float posDiffLength;
        posDiff = Vector3D<float>(*sourceX, *sourceY, *sourceZ) - sourcePos;
        posDiffLength = posDiff.length();
        
        if (posDiffLength > maxDist)
        {
            posDiff *= maxDist/posDiffLength;
            sourcePos += posDiff;
        }
        else
        {
            sourcePos = Vector3D<float>(*sourceX, *sourceY, *sourceZ);
        }
        
        posDiff = Vector3D<float>(*listenerX, *listenerY, *listenerZ) - listenerPos;
        posDiffLength = posDiff.length();
        
        if (posDiffLength > maxDist)
        {
            posDiff *= maxDist/posDiffLength;
            listenerPos += posDiff;
        }
        else
        {
            listenerPos = Vector3D<float>(*listenerX, *listenerY, *listenerZ);
        }
    }
    
    // prevent division by zero when source is as listener's position
    if ((listenerPos-sourcePos).lengthSquared() < 0.0001) {sourcePos = listenerPos + Vector3D<float>(0.01f, 0.0f, 0.0f);}
    
    
    float* pMonoBufferWrite = monoBuffer.getWritePointer(0);
    
    t = *roomX;
    b = *roomY;
    h = *roomZ;
    
    for (int q = 0; q<nImgSrc; ++q) //process all, because we can (and it avoids artefacts when adding img sources)
    {
        int m = reflList[q][0];
        int n = reflList[q][1];
        int o = reflList[q][2];
        mx[q] = m*t + mSig[m&1]*sourcePos.x - listenerPos.x;
        my[q] = n*b + mSig[n&1]*sourcePos.y - listenerPos.y;
        mz[q] = o*h + mSig[o&1]*sourcePos.z - listenerPos.z;
        
        mRadius[q] = sqrt(mx[q]*mx[q] + my[q]*my[q]+ mz[q]*mz[q]);
        mx[q] /= mRadius[q];
        my[q] /= mRadius[q];
        mz[q] /= mRadius[q];
        
        smx[q] = - mSig[m&1] * mx[q];
        smy[q] = - mSig[n&1] * my[q];
        smz[q] = - mSig[o&1] * mz[q];
    }
    
    
    for (int q=0; q<workingNumRefl+1; ++q) {
        if (q == 1) {
            for (int i = 0; i<nSIMDFilters; ++i)
            {
                lowShelfArray[i]->process(ProcessContextReplacing<SIMDRegister<float>> (*interleavedData[i]));
                highShelfArray[i]->process(ProcessContextReplacing<SIMDRegister<float>> (*interleavedData[i]));
            }
        }
        if (q == 7) {
            for (int i = 0; i<nSIMDFilters; ++i)
            {
                lowShelfArray2[i]->process(ProcessContextReplacing<SIMDRegister<float>> (*interleavedData[i]));
                highShelfArray2[i]->process(ProcessContextReplacing<SIMDRegister<float>> (*interleavedData[i]));
            }
        }
        
        // ========================================   CALCULATE SAMPLED MONO SIGNALS
        SIMDRegister<float> SHsample[16]; //TODO: can be smaller: (N+1)^2/SIMDRegister.size()
        SIMDRegister<float> SHsampleStep[16];
        FloatVectorOperations::clear((float *) &SHsample->value, 64);
        SHEval(directivityOrder, smx[q], smy[q], smz[q],(float *) &SHsample->value);
        
        interleavedData.getRawDataPointer();
        
        Array<SIMDRegister<float>*> interleavedDataPtr;
        interleavedDataPtr.resize(nSIMDFilters);
        SIMDRegister<float>** intrlvdDataArrayPtr = interleavedDataPtr.getRawDataPointer();
        //SIMDRegister<float>* interleavedDataPtr[nSIMDFilters];
        
        for (int i = 0; i<nSIMDFilters; ++i)
        {
            intrlvdDataArrayPtr[i] = reinterpret_cast<SIMDRegister<float>*> (interleavedData[i]->getChannelPointer (0));
            SHsampleStep[i] = SHsample[i]-SHsampleOld[q][i];
            SHsampleStep[i] *= oneOverL;
            SHsample[i] = SHsampleOld[q][i];
        }
        
        for (int smpl = 0; smpl < L; ++smpl)
        {
            SIMDRegister<float> SIMDTemp;
            SIMDTemp = 0.0f;
            
            for (int i = 0; i<nSIMDFilters; ++i)
            {
                SIMDTemp += SHsample[i] * *(intrlvdDataArrayPtr[i]++);
                SHsample[i] += SHsampleStep[i];
            }
            pBufferWrite[smpl] = SIMDTemp.sum();
        }
        
        // ============================================
        
        double delay, delayStep;
        int firstIdx, copyL;
        delay = mRadius[q]*dist2smpls; // dist2smpls also contains factor 128 for LUT
        delayStep = (delay - oldDelay[q])*oneOverL;
        
        //calculate firstIdx and copyL
        int startIdx = ((int)oldDelay[q])>>interpShift;
        int stopIdx = L-1 + (((int)(oldDelay[q] + delayStep * L-1))>>interpShift); // ((int)(startIdx + delayStep * L-1))>>7
        firstIdx = jmin(startIdx, stopIdx) - interpOffset;
        copyL = abs(stopIdx-startIdx) + interpLength;
        
        monoBuffer.clear(0,firstIdx, copyL); //TODO: optimization idea: resample input to match delay stretching
        
        float* tempWritePtr = pMonoBufferWrite; //reset writePtr as it gets increased during the next for loop
        const float* readPtr = pBufferRead;
        
        double tempDelay = oldDelay[q]; //start from oldDelay and add delayStep after each interation;
        
        //int interpCoeffIdx;
        for (int smplIdx = 0; smplIdx < L; ++smplIdx) {
            //int delayInt; = truncatePositiveToUnsignedInt(tempDelay); //(int)tempDelay;
            float integer;
            float fraction = modff(tempDelay, &integer);
            int delayInt = (int) integer;
            
            int interpCoeffIdx = delayInt&interpMask;
            delayInt = delayInt>>interpShift;
            int idx = delayInt-interpOffset;
            
            
            float* dest = tempWritePtr++ + idx;
            
            __m128 destSamples = _mm_loadu_ps(dest);
            
            __m128 srcSample = _mm_set1_ps(*readPtr++);
            
            __m128 interp = getInterpolatedLagrangeWeights(interpCoeffIdx, fraction);

            destSamples = _mm_add_ps(destSamples, _mm_mul_ps(interp, srcSample));
            //temp = _mm_add_ps(temp, (interpCoeffsSIMD[interpCoeffIdx] * *readPtr++).value);
            _mm_storeu_ps(dest, destSamples);
            
            tempDelay += delayStep;
        }
        
        const float* monoBufferReadPtrWithOffset = monoBuffer.getReadPointer(0) + firstIdx;
        firstIdx = firstIdx + readOffset;
        if (firstIdx >= bufferSize)
            firstIdx -= bufferSize;
        
        float SHcoeffs[64];
        float SHcoeffsStep[64];
        
        if (q<=currNumRefl)
        {
            SHEval(ambisonicOrder,mx[q], my[q], mz[q], SHcoeffs);
            if (*useSN3D > 0.5f)
            {
                FloatVectorOperations::multiply(SHcoeffs, SHcoeffs, n3d2sn3d, maxNChOut);
            }
        }
        else
            FloatVectorOperations::clear(SHcoeffs, 64);
        
        float gain = powReflCoeff[reflList[q][3]]/mRadius[q];
        allGains[q] = gain; // for reflectionVisualizer
        
        FloatVectorOperations::multiply(SHcoeffs, gain, maxNChOut);
        //FloatVectorOperations::multiply(SHcoeffs, mSig[reflList[q][3]&1]*gain, maxNChOut);
        FloatVectorOperations::subtract(SHcoeffsStep, SHcoeffs, SHcoeffsOld[q], maxNChOut);
        FloatVectorOperations::multiply(SHcoeffsStep, 1.0f/copyL, maxNChOut);
        
        if (firstIdx + copyL - 1 >= bufferSize)
        {
            int firstNumCopy = bufferSize - firstIdx;
            int secondNumCopy = copyL-firstNumCopy;
            
            for (int channel = 0; channel < maxNChOut; ++channel)
            {
                if (SHcoeffsOld[q][channel] != SHcoeffs[channel])
                {
#if defined(JUCE_USE_VDSP_FRAMEWORK) && defined(JUCE_MAC)
                    vDSP_vrampmuladd(monoBufferReadPtrWithOffset, 1, //input vector with stride
                                     &SHcoeffsOld[q][channel], //ramp start value (gets increased)
                                     &SHcoeffsStep[channel], //step value
                                     delayBufferWritePtrArray[channel] + firstIdx, 1,// output with stride
                                     (size_t) firstNumCopy //num
                                     );
                    vDSP_vrampmuladd(monoBufferReadPtrWithOffset+firstNumCopy, 1, //input vector with stride
                                     &SHcoeffsOld[q][channel], //ramp start value (gets increased)
                                     &SHcoeffsStep[channel], //step value
                                     delayBufferWritePtrArray[channel], 1,// output with stride
                                     (size_t) secondNumCopy //num
                                     );
#else
                    delayBuffer.addFromWithRamp(channel, firstIdx, monoBufferReadPtrWithOffset, firstNumCopy,
                                                SHcoeffsOld[q][channel], SHcoeffsOld[q][channel] + SHcoeffsStep[channel]*firstNumCopy);
                    delayBuffer.addFromWithRamp(channel, 0,        monoBufferReadPtrWithOffset+firstNumCopy, secondNumCopy,
                                                SHcoeffsOld[q][channel] + SHcoeffsStep[channel]*firstNumCopy, SHcoeffs[channel]);
#endif
                }
                else
                {
                    FloatVectorOperations::addWithMultiply(delayBufferWritePtrArray[channel] + firstIdx,
                                                           monoBufferReadPtrWithOffset,
                                                           SHcoeffs[channel], firstNumCopy);
                    FloatVectorOperations::addWithMultiply(delayBufferWritePtrArray[channel],
                                                           monoBufferReadPtrWithOffset + firstNumCopy,
                                                           SHcoeffs[channel], secondNumCopy);
                }
            }
        }
        else
        {
            for (int channel = 0; channel < maxNChOut; ++channel)
            {
                if (SHcoeffsOld[q][channel] != SHcoeffs[channel])
                {
#if defined(JUCE_USE_VDSP_FRAMEWORK) && defined(JUCE_MAC)
                    vDSP_vrampmuladd(monoBufferReadPtrWithOffset, 1, //input vector with stride
                                     &SHcoeffsOld[q][channel], //ramp start value (gets increased)
                                     &SHcoeffsStep[channel], //step value
                                     delayBufferWritePtrArray[channel] + firstIdx, 1,// output with stride
                                     (size_t) copyL //num
                                     );
#else
                    delayBuffer.addFromWithRamp(channel, firstIdx, monoBufferReadPtrWithOffset, copyL, SHcoeffsOld[q][channel], SHcoeffs[channel]);
#endif
                }
                else
                {
                    FloatVectorOperations::addWithMultiply(delayBufferWritePtrArray[channel] + firstIdx,
                                                           monoBufferReadPtrWithOffset,
                                                           SHcoeffs[channel], copyL);
                }
                
            }
        }
        
        FloatVectorOperations::copy(SHcoeffsOld[q], SHcoeffs, maxNChOut);
        FloatVectorOperations::copy((float *) &SHsampleOld[q]->value, (float *) &SHsample->value, maxNChIn);
        //oldDelay[q] = delay;
        oldDelay[q] = tempDelay;
    }
    
    //updating the remaining oldDelay values
    for (int q = workingNumRefl + 1; q < nImgSrc; ++q)
        oldDelay[q] = mRadius[q]*dist2smpls;
    
    // Read from buffer
    buffer.clear();
    for (int channel = 0; channel < maxNChOut; ++channel)
    {
        buffer.copyFrom(channel, 0, delayBufferWritePtrArray[channel] + readOffset, L);
        
        delayBuffer.clear(channel, readOffset, L);
    }
    
    _numRefl = currNumRefl;
    
    readOffset += L;
    if (readOffset >= bufferSize) readOffset = 0;
}

//==============================================================================
bool RoomEncoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* RoomEncoderAudioProcessor::createEditor()
{
    return new RoomEncoderAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void RoomEncoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RoomEncoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void RoomEncoderAudioProcessor::timerCallback()
{
    if (*syncChannel > 0.5f)
    {
        int ch = (int) *syncChannel;
        bool sRoom(*syncRoomSize>=0.5f);
        bool sListener(*syncListener>=0.5f);
        bool sReflections(*syncReflection>=0.5f);
        
        RoomParams& roomParam = sharedParams.get().rooms.getReference(ch);

        
        if (sRoom)
        {
            if (roomParam.validRoomData) {
                readingSharedParams = true;
                parameters.getParameterAsValue("roomX").setValue(roomParam.roomX);
                parameters.getParameterAsValue("roomY").setValue(roomParam.roomY);
                parameters.getParameterAsValue("roomZ").setValue(roomParam.roomZ);
                readingSharedParams = false;
            }
            else
            {
                roomParam.roomX = *roomX;
                roomParam.roomY = *roomY;
                roomParam.roomZ = *roomZ;
                roomParam.validRoomData = true;
            }
        }
        if (sListener)
        {
            if (roomParam.validListenerData) {
                readingSharedParams = true;
                parameters.getParameterAsValue("listenerX").setValue(roomParam.listenerX);
                parameters.getParameterAsValue("listenerY").setValue(roomParam.listenerY);
                parameters.getParameterAsValue("listenerZ").setValue(roomParam.listenerZ);
                readingSharedParams = false;
            }
            else
            {
                roomParam.listenerX = *listenerX;
                roomParam.listenerY = *listenerY;
                roomParam.listenerZ = *listenerZ;
                roomParam.validListenerData = true;
            }
        }
        if (sReflections)
        {
            if (roomParam.validReflectionData) {
                readingSharedParams = true;
                parameters.getParameterAsValue("reflCoeff").setValue(roomParam.reflCoeff);
                parameters.getParameterAsValue("numRefl").setValue(roomParam.numRefl);
                parameters.getParameterAsValue("lowShelfFreq").setValue(roomParam.lowShelfFreq);
                parameters.getParameterAsValue("lowShelfGain").setValue(roomParam.lowShelfGain);
                parameters.getParameterAsValue("highShelfFreq").setValue(roomParam.highShelfFreq);
                parameters.getParameterAsValue("highShelfGain").setValue(roomParam.highShelfGain);
                readingSharedParams = false;
            }
            else
            {
                roomParam.reflCoeff = *reflCoeff;
                roomParam.numRefl = *numRefl;
                roomParam.lowShelfFreq = *lowShelfFreq;
                roomParam.lowShelfGain = *lowShelfGain;
                roomParam.highShelfFreq = *highShelfFreq;
                roomParam.highShelfGain = *highShelfGain;
                roomParam.validReflectionData = true;
            }
        }
    }
}


void RoomEncoderAudioProcessor::checkOrderUpdateBuffers(int userSetDirectivityOrder, int userSetOutputOrder) {
    userChangedOrderSettings = false;
    //old values;
    _nChInput = nChInput;
    _nChOutput = nChOutput;
    _directivityOrder = directivityOrder;
    _ambisonicOrder = ambisonicOrder;
    
    
    maxPossibleOutputOrder = isqrt(getTotalNumOutputChannels())-1;
    maxPossibleInputOrder = isqrt(getTotalNumInputChannels())-1;
    
    // ================== CHECK INPUT
    if (userSetDirectivityOrder == -1 || userSetDirectivityOrder > maxPossibleInputOrder) directivityOrder = maxPossibleOutputOrder; // Auto setting or requested order exceeds highest possible order
    else directivityOrder = userSetDirectivityOrder;
    
    if (directivityOrder != _directivityOrder) {
        nChInput = squares[directivityOrder+1];
        DBG(nChInput << " input channels ");
    }
    
    
    // ================== CHECK OUTPUT
    if (userSetOutputOrder == -1 || userSetOutputOrder > maxPossibleOutputOrder) ambisonicOrder = maxPossibleOutputOrder; // Auto setting or requested order exceeds highest possible order
    else ambisonicOrder = userSetOutputOrder;
    
    if (ambisonicOrder != _ambisonicOrder) {
        nChOutput = squares[ambisonicOrder+1];
        delayBuffer.setSize(nChOutput, bufferSize);
        delayBuffer.clear();
        delayBufferWritePtrArray = delayBuffer.getArrayOfWritePointers();
        DBG("Used order has changed! Order: " << ambisonicOrder << ", numCH: " << nChOutput);
        DBG("Now updating filters and buffers.");
    }
}

void RoomEncoderAudioProcessor::setFilterVisualizer(FilterVisualizer* newFv)
{
    editorFv = newFv;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RoomEncoderAudioProcessor();
}
