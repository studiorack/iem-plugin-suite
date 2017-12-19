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
DualDelayAudioProcessor::DualDelayAudioProcessor()
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
parameters(*this,nullptr), LFOLeft([] (float phi) { return std::sin(phi);}), LFORight([] (float phi) { return std::sin(phi);})
{
    parameters.createAndAddParameter ("orderSetting", "Ambisonics Order", "",
                                      NormalisableRange<float> (0.0f, 7.0f, 1.0f), 0.0f,
                                      [](float value)
                                      {
                                          if (value >= 0.5f && value < 1.5f) return "1st";
                                          else if (value >= 1.5f && value < 2.5f) return "2nd";
                                          else if (value >= 2.5f && value < 3.5f) return "3rd";
                                          else if (value >= 3.5f && value < 4.5f) return "4th";
                                          else if (value >= 4.5f && value < 5.5f) return "5th";
                                          else if (value >= 5.5f && value < 6.5f) return "6th";
                                          else if (value >= 6.5f) return "7th";
                                          else return "Auto";
                                      }, nullptr);
    
    parameters.createAndAddParameter ("useSN3D", "Normalization", "",
                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                      [](float value)
                                      {
                                          if (value >= 0.5f ) return "SN3D";
                                          else return "N3D";
                                      }, nullptr);
    
    
    parameters.createAndAddParameter("dryGain", "Dry amount", "dB",
                                     NormalisableRange<float> (-60.0f, 0.0f, 0.1f), 0.0f,
                                     [](float value) {return (value >= -59.9f) ? String(value) : "-inf";}, nullptr);
    parameters.createAndAddParameter("wetGainL", "Wet amount left", "dB",
                                     NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -6.0f,
                                     [](float value) {return (value >= -59.9f) ? String(value) : "-inf";}, nullptr);
    parameters.createAndAddParameter("wetGainR", "Wet amount right", "dB",
                                     NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -6.0f,
                                     [](float value) {return (value >= -59.9f) ? String(value) : "-inf";}, nullptr);
    
    parameters.createAndAddParameter("delayTimeL", "delay time left", "ms",
                                     NormalisableRange<float> (10.0f, 500.0f, 0.1f), 500.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("delayTimeR", "delay time right", "ms",
                                     NormalisableRange<float> (10.0f, 500.0f, 0.1f), 375.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("rotationL", "rotation left", "degree",
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.1f), 10.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("rotationR", "rotation right", "degree",
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.1f), -7.5f,
                                     [](float value) {return String(value);}, nullptr);
    
    
    parameters.createAndAddParameter("LPcutOffL", "lowpass frequency left", "Hz",
                                     NormalisableRange<float> (20.0f, 20000.0f, 1.0f,0.2), 100.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("LPcutOffR", "lowpass frequency right", "Hz",
                                     NormalisableRange<float> (20.0f, 20000.0f, 1.0f,0.2), 100.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("HPcutOffL", "highpass frequency left", "Hz",
                                     NormalisableRange<float> (20.0f, 20000.0f, 1.0f,0.2), 20000.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("HPcutOffR", "highpass frequency right", "Hz",
                                     NormalisableRange<float> (20.0f, 20000.0f, 1.0f,0.2), 20000.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    
    parameters.createAndAddParameter("feedbackL", "feedback left", "dB",
                                     NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -8.0f,
                                     [](float value) {return (value >= -59.9f) ? String(value) : "-inf";}, nullptr);
    parameters.createAndAddParameter("feedbackR", "feedback right", "dB",
                                     NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -8.0f,
                                     [](float value) {return (value >= -59.9f) ? String(value) : "-inf";}, nullptr);
    
    parameters.createAndAddParameter("xfeedbackL", "cross feedback left", "dB",
                                     NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -20.0f,
                                     [](float value) {return (value >= -59.9f) ? String(value) : "-inf";}, nullptr);
    parameters.createAndAddParameter("xfeedbackR", "cross feedback right", "dB",
                                     NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -20.0f,
                                     [](float value) {return (value >= -59.9f) ? String(value) : "-inf";}, nullptr);
    
    parameters.createAndAddParameter("lfoRateL", "LFO left rate", "Hz",
                                     NormalisableRange<float> (0.0f, 10.0f, 0.01f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("lfoRateR", "LFO right rate", "Hz",
                                     NormalisableRange<float> (0.0f, 10.0f, 0.01f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("lfoDepthL", "LFO left depth", "ms",
                                     NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("lfoDepthR", "LFO right depth", "ms",
                                     NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    
    
    
    parameters.state = ValueTree (Identifier ("DualDelay"));
    
    dryGain = parameters.getRawParameterValue("dryGain");
    wetGainL = parameters.getRawParameterValue("wetGainL");
    wetGainR = parameters.getRawParameterValue("wetGainR");
    delayTimeL = parameters.getRawParameterValue("delayTimeL");
    delayTimeR = parameters.getRawParameterValue("delayTimeR");
    rotationL = parameters.getRawParameterValue("rotationL");
    rotationR = parameters.getRawParameterValue("rotationR");
    HPcutOffL = parameters.getRawParameterValue("HPcutOffL");
    HPcutOffR = parameters.getRawParameterValue("HPcutOffR");
    LPcutOffL = parameters.getRawParameterValue("LPcutOffL");
    LPcutOffR = parameters.getRawParameterValue("LPcutOffR");
    feedbackL = parameters.getRawParameterValue("feedbackL");
    feedbackR = parameters.getRawParameterValue("feedbackR");
    xfeedbackL = parameters.getRawParameterValue("xfeedbackL");
    xfeedbackR = parameters.getRawParameterValue("xfeedbackR");
    lfoRateL = parameters.getRawParameterValue("lfoRateL");
    lfoRateR = parameters.getRawParameterValue("lfoRateR");
    lfoDepthL = parameters.getRawParameterValue("lfoDepthL");
    lfoDepthR = parameters.getRawParameterValue("lfoDepthR");
    orderSetting = parameters.getRawParameterValue("orderSetting");
    parameters.addParameterListener("orderSetting", this);

    
    
    
    cos_z.resize(8);
    sin_z.resize(8);
    cos_z.set(0, 1.f);
    sin_z.set(0, 0.f);
}

DualDelayAudioProcessor::~DualDelayAudioProcessor()
{
    for (int i=0; i<lowPassFiltersLeft.size(); ++i)
    {
        delete lowPassFiltersLeft[i];
        delete lowPassFiltersRight[i];
        delete highPassFiltersLeft[i];
        delete highPassFiltersRight[i];
    }
}

//==============================================================================
const String DualDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DualDelayAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DualDelayAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double DualDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DualDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int DualDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DualDelayAudioProcessor::setCurrentProgram (int index)
{
}

const String DualDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void DualDelayAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DualDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkOrderUpdateBuffers(roundFloatToInt(*orderSetting - 1));
    
    
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;
    spec.maximumBlockSize = samplesPerBlock;
    LFOLeft.prepare(spec);
    LFORight.prepare(spec);
    
    
    delayBufferLeft.clear();
    delayBufferRight.clear();
    
    writeOffsetLeft = 0;
    writeOffsetRight = 0;
    readOffsetLeft = 0;
    readOffsetRight = 0;
    
    delay.resize(samplesPerBlock);
    interpCoeffIdx.resize(samplesPerBlock);
    idx.resize(samplesPerBlock);
    
    
    _delayL = *delayTimeL * sampleRate / 1000.0 * 128;
    _delayR = *delayTimeR * sampleRate / 1000.0 * 128;
}

void DualDelayAudioProcessor::releaseResources() { }

#ifndef JucePlugin_PreferredChannelConfigurations
bool DualDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void DualDelayAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    if (userChangedOrderSettings) checkOrderUpdateBuffers(roundFloatToInt(*orderSetting - 1));
    
    const int totalNumInputChannels  =  getTotalNumInputChannels();
    
    const int workingOrder = jmin(isqrt(buffer.getNumChannels())-1, ambisonicOrder);
    const int nCh = squares[workingOrder+1];
    
    
    const int delayBufferLength = getSampleRate(); // not necessarily samplerate
    const int fs = getSampleRate();
    
    const float msToFractSmpls = getSampleRate() / 1000.0 * 128.0;
    const int spb = buffer.getNumSamples();
    
    //clear not used channels
    for (int channel = nChannels; channel<totalNumInputChannels; ++channel)
        buffer.clear(channel, 0, spb);
    
    
    LFOLeft.setFrequency(*lfoRateL);
    LFORight.setFrequency(*lfoRateR);
    
    
    for (int i=0; i<nChannels; ++i)
    {
        lowPassFiltersLeft[i]->setCoefficients(IIRCoefficients::makeLowPass(fs,*LPcutOffL));
        lowPassFiltersRight[i]->setCoefficients(IIRCoefficients::makeLowPass(fs,*LPcutOffR));
        highPassFiltersLeft[i]->setCoefficients(IIRCoefficients::makeHighPass(fs,*HPcutOffL));
        highPassFiltersRight[i]->setCoefficients(IIRCoefficients::makeHighPass(fs,*HPcutOffR));
    }
    
    // ==================== MAKE COPY OF INPUT BUFFER==============================
    for (int channel = 0; channel < nCh; ++channel)
    {
        AudioIN.copyFrom(channel, 0, buffer, channel, 0, spb);
    }
    
    // ==================== READ FROM DELAYLINE AND GENERTE OUTPUT SIGNAL ===========
    // LEFT CHANNEL
    if (readOffsetLeft + spb >= delayBufferLength) { // overflow
        int nFirstRead = delayBufferLength - readOffsetLeft;
        
        for (int channel = 0; channel < nCh; ++channel)
        {
            delayOutLeft.copyFrom(channel, 0, delayBufferLeft, channel, readOffsetLeft, nFirstRead);
            delayOutLeft.copyFrom(channel, nFirstRead, delayBufferLeft, channel, 0, spb-nFirstRead);
        }
        delayBufferLeft.clear(readOffsetLeft, nFirstRead);
        delayBufferLeft.clear(0, spb-nFirstRead);
        
        readOffsetLeft += spb;
        readOffsetLeft -= delayBufferLength;
        
    }
    else { //noverflow
        for (int channel = 0; channel < nCh; ++channel)
        {
            delayOutLeft.copyFrom(channel, 0, delayBufferLeft, channel, readOffsetLeft, spb);
        }
        delayBufferLeft.clear(readOffsetLeft, spb);
        readOffsetLeft += spb;
    }
    
    // RIGHT CHANNEL
    if (readOffsetRight + spb >= delayBufferLength) { // overflow
        int nFirstRead = delayBufferLength - readOffsetRight;
        
        for (int channel = 0; channel < nCh; ++channel)
        {
            delayOutRight.copyFrom(channel, 0, delayBufferRight, channel, readOffsetRight, nFirstRead);
            delayOutRight.copyFrom(channel, nFirstRead, delayBufferRight, channel, 0, spb-nFirstRead);
        }
        delayBufferRight.clear(readOffsetRight, nFirstRead);
        delayBufferRight.clear(0, spb-nFirstRead);
        
        readOffsetRight += spb;
        readOffsetRight -= delayBufferLength;
        
    }
    else { //noverflow
        for (int channel = 0; channel < nCh; ++channel)
        {
            delayOutRight.copyFrom(channel, 0, delayBufferRight, channel, readOffsetRight, spb);
        }
        delayBufferRight.clear(readOffsetRight, spb);
        readOffsetRight += spb;
    }
    
    // ========== OUTPUT
    buffer.applyGain(Decibels::decibelsToGain(*dryGain,-59.91f)); //dry signal
    for (int channel = 0; channel < nCh; ++channel)
    {
        buffer.addFrom(channel, 0, delayOutLeft, channel, 0, spb, Decibels::decibelsToGain(*wetGainL,-59.91f)); //wet signal
        buffer.addFrom(channel, 0, delayOutRight, channel, 0, spb, Decibels::decibelsToGain(*wetGainR,-59.91f)); //wet signal
    }
    
    // ================ ADD INPUT AND FED BACK OUTPUT WITH PROCESSING ===========
    
    for (int channel = 0; channel < nCh; ++channel) // should be optimizable with SIMD
    {
        delayInLeft.copyFrom(channel, 0, AudioIN.getReadPointer(channel), spb); // input
        delayInLeft.addFrom(channel, 0, delayOutLeft.getReadPointer(channel), spb, Decibels::decibelsToGain(*feedbackL,-59.91f) ); // feedback gain
        delayInLeft.addFrom(channel, 0, delayOutRight.getReadPointer(channel),  spb, Decibels::decibelsToGain(*xfeedbackR,-59.91f) ); // feedback bleed gain
        lowPassFiltersLeft[channel]->processSamples(delayInLeft.getWritePointer(channel), spb); //filter
        highPassFiltersLeft[channel]->processSamples(delayInLeft.getWritePointer(channel), spb); //filter
        
        delayInRight.copyFrom(channel, 0, AudioIN.getReadPointer(channel), spb); // input
        delayInRight.addFrom(channel, 0, delayOutRight.getReadPointer(channel), spb,  Decibels::decibelsToGain(*feedbackR,-59.91f) ); // feedback gain
        delayInRight.addFrom(channel, 0, delayOutLeft.getReadPointer(channel), spb, Decibels::decibelsToGain(*xfeedbackL,-59.91f) ); // feedback bleed gain
        lowPassFiltersRight[channel]->processSamples(delayInRight.getWritePointer(channel), spb); //filter
        highPassFiltersRight[channel]->processSamples(delayInRight.getWritePointer(channel), spb); //filter
    }
    
    // left delay rotation
    calcParams(*rotationL/180.0f*M_PI);
    rotateBuffer(&delayInLeft, spb);
    
    // right delay rotation
    calcParams(*rotationR/180.0f*M_PI);
    rotateBuffer(&delayInRight, spb);
    
    
    // =============== UPDATE DELAY PARAMETERS =====
    float delayL = *delayTimeL * msToFractSmpls;
    float delayR = *delayTimeR * msToFractSmpls;

    int firstIdx, copyL;
    
    // ============= WRITE INTO DELAYLINE ========================
    // ===== LEFT CHANNEL

    
    float delayStep = (delayL - _delayL)/spb;
    //calculate firstIdx and copyL
    for (int i=0; i<spb; ++i) {
        delay.set(i, i*128 + _delayL + i*delayStep + *lfoDepthL * msToFractSmpls * LFOLeft.processSample(1.0f));
    }
    firstIdx = (((int) *std::min_element(delay.getRawDataPointer(),delay.getRawDataPointer()+spb)) >> interpShift) - interpOffset;
    int lastIdx =  (((int) *std::max_element(delay.getRawDataPointer(),delay.getRawDataPointer()+spb)) >> interpShift) - interpOffset;
    copyL = abs(firstIdx - lastIdx) + interpLength;
    

    
    delayTempBuffer.clear(0, copyL);
    //delayTempBuffer.clear();
    const float** readPtrArr = delayInLeft.getArrayOfReadPointers();
    
    for (int i=0; i<spb; ++i) {
        
        float integer;
        float fraction = modff(delay[i], &integer);
        int delayInt = (int) integer;
        
        int interpCoeffIdx = delayInt&interpMask;
        delayInt = delayInt>>interpShift;
        int idx = delayInt-interpOffset - firstIdx;
        
        __m128 interp = getInterpolatedLagrangeWeights(interpCoeffIdx, fraction);
        
        for (int ch = 0; ch < nCh; ++ch)
        {
            float* dest = delayTempBuffer.getWritePointer(ch, idx);
            
            __m128 destSamples = _mm_loadu_ps(dest);
            __m128 srcSample = _mm_set1_ps(readPtrArr[ch][i]);
            destSamples = _mm_add_ps(destSamples, _mm_mul_ps(interp, srcSample));
            _mm_storeu_ps(dest, destSamples);
        }
    }
    writeOffsetLeft = readOffsetLeft + firstIdx;
    if (writeOffsetLeft >= delayBufferLength)
        writeOffsetLeft -= delayBufferLength;
    
    if (writeOffsetLeft + copyL >= delayBufferLength) { // overflow
        int firstNumCopy = delayBufferLength - writeOffsetLeft;
        int secondNumCopy = copyL-firstNumCopy;
        
        for (int channel = 0; channel < nCh; ++channel)
        {
            delayBufferLeft.addFrom(channel, writeOffsetLeft, delayTempBuffer, channel, 0, firstNumCopy);
            delayBufferLeft.addFrom(channel, 0, delayTempBuffer, channel, firstNumCopy, secondNumCopy);
        }
    }
    else { // no overflow
        for (int channel = 0; channel < nCh; ++channel)
        {
            delayBufferLeft.addFrom(channel, writeOffsetLeft, delayTempBuffer, channel, 0 , copyL);
        }
    }
    
    // ===== Right CHANNEL
    
    
    delayStep = (delayR - _delayR)/spb;
    //calculate firstIdx and copyL
    for (int i=0; i<spb; ++i) {
        delay.set(i, i*128 + _delayR + i*delayStep + *lfoDepthR * msToFractSmpls * LFORight.processSample(1.0f));
    }
    firstIdx = (((int) *std::min_element(delay.getRawDataPointer(),delay.getRawDataPointer()+spb)) >> interpShift) - interpOffset;
    lastIdx =  (((int) *std::max_element(delay.getRawDataPointer(),delay.getRawDataPointer()+spb)) >> interpShift) - interpOffset;
    copyL = abs(firstIdx - lastIdx) + interpLength;
    
    
    
    delayTempBuffer.clear(0, copyL);

    const float** readPtrArrR = delayInRight.getArrayOfReadPointers();
    
    for (int i=0; i<spb; ++i) {
        float integer;
        float fraction = modff(delay[i], &integer);
        int delayInt = (int) integer;
        
        int interpCoeffIdx = delayInt&interpMask;
        delayInt = delayInt>>interpShift;
        int idx = delayInt-interpOffset - firstIdx;
        
        __m128 interp = getInterpolatedLagrangeWeights(interpCoeffIdx, fraction);
        
        for (int ch = 0; ch < nCh; ++ch)
        {
            float* dest = delayTempBuffer.getWritePointer(ch, idx);
            
            __m128 destSamples = _mm_loadu_ps(dest);
            __m128 srcSample = _mm_set1_ps(readPtrArrR[ch][i]);
            destSamples = _mm_add_ps(destSamples, _mm_mul_ps(interp, srcSample));
            _mm_storeu_ps(dest, destSamples);
        }
    }
    writeOffsetRight = readOffsetRight + firstIdx;
    if (writeOffsetRight >= delayBufferLength)
        writeOffsetRight -= delayBufferLength;
    
    if (writeOffsetRight + copyL >= delayBufferLength) { // overflow
        int firstNumCopy = delayBufferLength - writeOffsetRight;
        int secondNumCopy = copyL-firstNumCopy;
        
        for (int channel = 0; channel < nCh; ++channel)
        {
            delayBufferRight.addFrom(channel, writeOffsetRight, delayTempBuffer, channel, 0, firstNumCopy);
            delayBufferRight.addFrom(channel, 0, delayTempBuffer, channel, firstNumCopy, secondNumCopy);
        }
    }
    else { // no overflow
        for (int channel = 0; channel < nCh; ++channel)
        {
            delayBufferRight.addFrom(channel, writeOffsetRight, delayTempBuffer, channel, 0 , copyL);
        }
    }
    
    // =============== UPDATE DELAY PARAMETERS =====
    _delayL = delayL;
    _delayR = delayR;
    
}

//==============================================================================
bool DualDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DualDelayAudioProcessor::createEditor()
{
    return new DualDelayAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void DualDelayAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}

void DualDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.state = ValueTree::fromXml (*xmlState);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DualDelayAudioProcessor();
}

void DualDelayAudioProcessor::calcParams(float phi)
{
    // use mathematical negative angles!
    cos_z.set(1, cosf(phi));
    sin_z.set(1, sinf(phi));
    
    // chebyshev recursion
    for (int i = 2; i < 8; i++) {
        cos_z.set(i, 2 * cos_z[1] * cos_z[i-1] - cos_z[i-2]);
        sin_z.set(i, 2 * cos_z[1] * sin_z[i-1] - sin_z[i-2]);
    }
}

void DualDelayAudioProcessor::rotateBuffer(AudioBuffer<float>* bufferToRotate, int samples)
{
    AudioBuffer<float> tempBuffer;
    tempBuffer.makeCopyOf(*bufferToRotate);
    bufferToRotate->clear();
    
    int nCh = jmin(nChannels, bufferToRotate->getNumChannels());
    
    for (int acn_out = 0; acn_out < nCh; ++acn_out)
    {
        int l_out = 0;
        int m_out = 0;
        
        ACNtoLM(acn_out, l_out, m_out);
        
        for (int acn_in = 0; acn_in < nCh; ++acn_in)
        {
            int l_in=0; // degree 0, 1, 2, 3, 4, ......
            int m_in=0; // order ...., -2, -1, 0 , 1, 2, ...
            
            ACNtoLM(acn_in, l_in, m_in);
            
            if (abs(m_out) == abs (m_in) && l_in == l_out) { // if degree and order match  do something
                
                if (m_out == 0 && m_in == 0) {
                    // gain 1 -> no interpolation needed
                    bufferToRotate->copyFrom(acn_out, 0, tempBuffer, acn_in, 0, samples);
                }
                else  if (m_in < 0 && m_out < 0)
                {
                    bufferToRotate->addFrom(acn_out, 0, tempBuffer.getReadPointer(acn_in), samples, cos_z[-m_out]); //
                }
                else  if (m_in < 0 && m_out > 0)
                {
                    bufferToRotate->addFrom(acn_out, 0, tempBuffer.getReadPointer(acn_in), samples, -sin_z[m_out]);
                }
                else  if (m_in > 0 && m_out > 0)
                {
                    bufferToRotate->addFrom(acn_out, 0, tempBuffer.getReadPointer(acn_in), samples,  cos_z[m_out]);
                }
                else  if (m_in > 0 && m_out < 0)
                {
                    bufferToRotate->addFrom(acn_out, 0, tempBuffer.getReadPointer(acn_in), samples, sin_z[m_in]);
                }
                
            }
            
        }
        
    }
}

void DualDelayAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "orderSetting") userChangedOrderSettings = true;
}

void DualDelayAudioProcessor::checkOrderUpdateBuffers(int userSetInputOrder) {
    const int sampleRate = getSampleRate();
    const int samplesPerBlock = getBlockSize();
    
    //old values;
    _nChannels = nChannels;
    _ambisonicOrder = ambisonicOrder;
    
    maxPossibleOrder = isqrt(getTotalNumInputChannels())-1;
    if (userSetInputOrder == -1 || userSetInputOrder > maxPossibleOrder) ambisonicOrder = maxPossibleOrder; // Auto setting or requested order exceeds highest possible order
    else ambisonicOrder = userSetInputOrder;
    
    
    if (ambisonicOrder != _ambisonicOrder) {
        nChannels = squares[ambisonicOrder+1];
        DBG("Used order has changed! Order: " << ambisonicOrder << ", numCH: " << nChannels);
        DBG("Now updating filters and buffers.");
        
        
        if (nChannels > _nChannels)
        {
            lowPassFiltersLeft.resize(nChannels);
            lowPassFiltersRight.resize(nChannels);
            highPassFiltersLeft.resize(nChannels);
            highPassFiltersRight.resize(nChannels);
            for (int i=_nChannels; i<nChannels; ++i)
            {
                lowPassFiltersLeft.set(i, new IIRFilter());
                lowPassFiltersRight.set(i, new IIRFilter());
                highPassFiltersLeft.set(i, new IIRFilter());
                highPassFiltersRight.set(i, new IIRFilter());
                lowPassFiltersLeft[i]->reset();
                lowPassFiltersRight[i]->reset();
                highPassFiltersLeft[i]->reset();
                highPassFiltersRight[i]->reset();
            }
        }
        else {
            for (int i=nChannels; i<highPassFiltersRight.size(); ++i)
            {
                delete lowPassFiltersLeft[i];
                delete lowPassFiltersRight[i];
                delete highPassFiltersLeft[i];
                delete highPassFiltersRight[i];
            }
            lowPassFiltersLeft.resize(nChannels);
            lowPassFiltersRight.resize(nChannels);
            highPassFiltersLeft.resize(nChannels);
            highPassFiltersRight.resize(nChannels);
        }
        

        
        
        AudioIN.setSize(nChannels,samplesPerBlock);
        AudioIN.clear();
        
        delayBufferLeft.setSize(nChannels, 50000);
        delayBufferRight.setSize(nChannels, 50000);
        delayBufferLeft.clear();
        delayBufferRight.clear();
        
        int maxLfoDepth = (int) ceilf(parameters.getParameterRange("lfoDepthL").getRange().getEnd()*sampleRate/500.0f);
        delayTempBuffer.setSize(nChannels,samplesPerBlock+interpOffset-1+maxLfoDepth+sampleRate*0.5);
        
        delayOutLeft.setSize(nChannels, samplesPerBlock);
        delayOutRight.setSize(nChannels, samplesPerBlock);
        delayOutLeft.clear();
        delayOutRight.clear();
        
        delayInLeft.setSize(nChannels,samplesPerBlock);
        delayInRight.setSize(nChannels,samplesPerBlock);
        delayInLeft.clear();
        delayInRight.clear();
    }
}

