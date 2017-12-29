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


#define deg2rad M_PI/180.0
#define rad2deg 180.0/M_PI
//==============================================================================
AmbisonicCompressorAudioProcessor::AmbisonicCompressorAudioProcessor()
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
    parameters.createAndAddParameter("orderSetting", "Ambisonics Order", "",
                                     NormalisableRange<float>(0.0f, 8.0f, 1.0f), 0.0f,
                                     [](float value) {
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
    parameters.createAndAddParameter("useSN3D", "Normalization", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "SN3D";
                                         else return "N3D";
                                     }, nullptr);
    
    parameters.createAndAddParameter("preGain", "Input Gain ", "dB",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.1f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    // compressor 1
    parameters.createAndAddParameter("c1Enabled", "Compressor 1", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0,
                                     [](float value)
                                     {
                                         if (value >= 0.5f) return "ON";
                                         else return "OFF";
                                     }, nullptr);
    
    parameters.createAndAddParameter("c1DrivingSignal", "Compressor 1 Driving Signal", "",
                                     NormalisableRange<float> (0.0f, 2.0f, 1.0f), 1.0,
                                     [](float value)
                                     {
                                         if (value >= 0.5f && value < 1.5f) return "Masked";
                                         else if (value >= 1.5f) return "Unmasked";
                                         else return "Full";
                                     }, nullptr);
    parameters.createAndAddParameter("c1Apply", "Apply compression 1 to", "",
                                     NormalisableRange<float> (0.0f, 2.0f, 1.0f), 1.0,
                                     [](float value)
                                     {
                                         if (value >= 0.5f && value < 1.5f) return "Masked";
                                         else if (value >= 1.5f) return "Unmasked";
                                         else return "Full";
                                     }, nullptr);
    parameters.createAndAddParameter("c1Threshold", "Threshold 1", "dB",
                                     NormalisableRange<float> (-50.0f, 0.0f, 0.1f), -10.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("c1Attack", "Attack Time 1", "ms",
                                     NormalisableRange<float> (1.0f, 100.0f, 0.1f), 30.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("c1Release", "Release Time 1", "ms",
                                     NormalisableRange<float> (1.0f, 500.0f, 0.1f), 150.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("c1Ratio", "Ratio 1", "",
                                     NormalisableRange<float> (1.0f, 10.0f, .1f), 4.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("c1Makeup", "MakeUp Gain 1", "dB",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.10f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    
    // compressor 2
    parameters.createAndAddParameter("c2Enabled", "Compressor 2", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0,
                                     [](float value)
                                     {
                                         if (value >= 0.5f) return "ON";
                                         else return "OFF";
                                     }, nullptr);
    parameters.createAndAddParameter("c2DrivingSignal", "Compressor 2 Driving Signal", "",
                                     NormalisableRange<float> (0.0f, 2.0f, 1.0f), 1.0,
                                     [](float value)
                                     {
                                         if (value >= 0.5f && value < 1.5f) return "Masked";
                                         else if (value >= 1.5f) return "Unmasked";
                                         else return "Full";
                                     }, nullptr);
    parameters.createAndAddParameter("c2Apply", "Apply compression 2 to", "",
                                     NormalisableRange<float> (0.0f, 2.0f, 1.0f), 1.0,
                                     [](float value)
                                     {
                                         if (value >= 0.5f && value < 1.5f) return "Masked";
                                         else if (value >= 1.5f) return "Unmasked";
                                         else return "Full";
                                     }, nullptr);
    parameters.createAndAddParameter("c2Threshold", "Threshold 2", "dB",
                                     NormalisableRange<float> (-50.0f, 0.0f, 0.1f), -10.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("c2Attack", "Attack Time 2", "ms",
                                     NormalisableRange<float> (1.0f, 100.0f, 0.1f), 30.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("c2Release", "Release Time 2", "ms",
                                     NormalisableRange<float> (1.0f, 500.0f, 0.1f), 150.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("c2Ratio", "Ratio 2", "",
                                     NormalisableRange<float> (1.0f, 10.0f, .1f), 4.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("c2Makeup", "MakeUp Gain 2", "dB",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.10f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    
    
    
    parameters.createAndAddParameter("yaw", "Yaw", "deg",
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("pitch", "Pitch", "deg",
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("width", "Width", "deg",
                                     NormalisableRange<float> (10.0f, 180.0f, 0.01f), 40.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("listen", "Listen to", "",
                                     NormalisableRange<float> (0.0f, 2.0f, 1.0f), 0.0,
                                     [](float value)
                                     {
                                         if (value >= 0.5f && value < 1.5f) return "Masked";
                                         else if (value >= 1.5f) return "Unmasked";
                                         else return "Full";
                                     }, nullptr);
    
    
    parameters.state = ValueTree (Identifier ("DirectionalCompressor"));
    
    
    parameters.addParameterListener ("yaw", this);
    parameters.addParameterListener ("pitch", this);
    parameters.addParameterListener ("width", this);
    parameters.addParameterListener ("orderSetting", this);
    
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    preGain = parameters.getRawParameterValue ("preGain");
    
    c1Enabled = parameters.getRawParameterValue ("c1Enabled");
    c1DrivingSignal = parameters.getRawParameterValue ("c1DrivingSignal");
    c1Apply = parameters.getRawParameterValue ("c1Apply");
    c1Threshold = parameters.getRawParameterValue ("c1Threshold");
    c1Attack = parameters.getRawParameterValue ("c1Attack");
    c1Release = parameters.getRawParameterValue ("c1Release");
    c1Ratio = parameters.getRawParameterValue ("c1Ratio");
    c1Makeup = parameters.getRawParameterValue ("c1Makeup");
 
    c2Enabled = parameters.getRawParameterValue ("c2Enabled");
    c2DrivingSignal = parameters.getRawParameterValue ("c2DrivingSignal");
    c2Apply = parameters.getRawParameterValue ("c2Apply");
    c2Threshold = parameters.getRawParameterValue ("c2Threshold");
    c2Attack = parameters.getRawParameterValue ("c2Attack");
    c2Release = parameters.getRawParameterValue ("c2Release");
    c2Ratio = parameters.getRawParameterValue ("c2Ratio");
    c2Makeup = parameters.getRawParameterValue ("c2Makeup");
    
    yaw = parameters.getRawParameterValue ("yaw");
    pitch = parameters.getRawParameterValue ("pitch");
    width = parameters.getRawParameterValue ("width");
    listen = parameters.getRawParameterValue ("listen");
    
    c1MaxGR = 0.0f;
    c2MaxGR = 0.0f;
    c1GR = 0.0f;
    c2GR = 0.0f;
    
    // calc Y and YH
    for (int point=0; point<tDesignN; ++point)
    {
        SHEval(7, tDesignX[point], tDesignY[point], tDesignZ[point], Y.data()+point*64);
        //FloatVectorOperations::multiply(Y.data()+point*64, Y.data()+point*64, sn3d2n3d, 64); //expecting sn3d normalization -> converting it to n3d
    }
    
    Y *= sqrt(4*M_PI/tDesignN) / correction(7); // reverting 7th order correction
    //Y.transposeInPlace();
    YH = Y.transpose();
}


AmbisonicCompressorAudioProcessor::~AmbisonicCompressorAudioProcessor()
{
    
}

//==============================================================================
const String AmbisonicCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AmbisonicCompressorAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AmbisonicCompressorAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double AmbisonicCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AmbisonicCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AmbisonicCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmbisonicCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const String AmbisonicCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void AmbisonicCompressorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

void AmbisonicCompressorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "yaw" || parameterID == "pitch" || parameterID == "width") paramChanged = true;
    else if (parameterID == "orderSetting") userChangedIOSettings = true;
}

//==============================================================================
void AmbisonicCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *orderSetting, *orderSetting, true);
    
    omniW.setSize(1, samplesPerBlock);
    
    
    c1RMS.resize(samplesPerBlock);
    c1Gains.resize(samplesPerBlock);
    c1GRarray.resize(samplesPerBlock);
    
    c2RMS.resize(samplesPerBlock);
    c2Gains.resize(samplesPerBlock);
    c2GRarray.resize(samplesPerBlock);

    
    const double A = exp(-1.0/(sampleRate*0.01)); //multiplicated value after sampleRate is rms time
    c1MeanSqrFilter.setCoefficients(juce::IIRCoefficients(1.0 - A,0.0,0.0,1.0,- A,0.0));
    c2MeanSqrFilter.setCoefficients(juce::IIRCoefficients(1.0 - A,0.0,0.0,1.0,- A,0.0));
    calcParams();
}

void AmbisonicCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AmbisonicCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void AmbisonicCompressorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *orderSetting, *orderSetting);
    if (paramChanged) calcParams();
    
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int sampleRate = this->getSampleRate();
    const int bufferSize = buffer.getNumSamples();
    const int ambisonicOrder = input.getOrder();
    
    drivingPointers[0] = maskBuffer.getReadPointer(0);
    drivingPointers[1] = buffer.getReadPointer(0);
    drivingPointers[2] = omniW.getReadPointer(0);
    
    
    
    const int numCh = jmin(input.getNumberOfChannels(), buffer.getNumChannels());
    
    // preGain - can be tweaked by adding gain to compressor gains
    float preGainLinear = Decibels::decibelsToGain(*preGain);
    
    if (*useSN3D >= 0.5f)
        for (int i = 0; i < numCh; ++i)
            buffer.applyGain(i, 0, bufferSize, sn3d2n3d[i]*preGainLinear);
    else
        buffer.applyGain(Decibels::decibelsToGain(*preGain));
    
    
    // --------- make copys of buffer
    omniW.copyFrom(0, 0, buffer, 0, 0, bufferSize);

    maskBuffer.clear();
    maskBuffer.setSample(0, 0, 0.0f);
    for (int chIn = 0; chIn < numCh; ++chIn)
    {
        const float* readPtr = buffer.getReadPointer(chIn);
        for (int chOut = 0; chOut < numCh; ++chOut)
        {
            maskBuffer.addFrom(chOut, 0, readPtr, bufferSize, P1(chOut, chIn));
        }
    }
    /* This makes the buffer containing the negative mask */
    for (int chIn = 0; chIn < numCh; ++chIn)
        FloatVectorOperations::subtract(buffer.getWritePointer(chIn), maskBuffer.getReadPointer(chIn), bufferSize);
    
    // ------- clear not needed channels
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    

    // =============== COMPRESSOR 1 ====================
    {
        // needed variables
        float envSample, attackGain, releaseGain, ratioFactor;
        // parameters
        attackGain = expf(-1000.0f / sampleRate / *c1Attack);
        releaseGain = expf(-1000.0f / sampleRate / *c1Release);
        ratioFactor = -1.0f * (1 - 1/ *c1Ratio);
        
        // set compressor driving signal
        const float* drivingSignalPtr = [this] () -> const float* {
            if (*c1DrivingSignal >= 0.5f && *c1DrivingSignal < 1.5f) return drivingPointers[0];
            else if (*c1DrivingSignal >= 1.5f) return drivingPointers[1];
            else return drivingPointers[2];
        }();
        
        // calculate RMS
        FloatVectorOperations::multiply(c1RMS.getRawDataPointer(), drivingSignalPtr, drivingSignalPtr, bufferSize);
        c1MeanSqrFilter.processSamples(c1RMS.getRawDataPointer(), bufferSize);
        for (int i = 0; i<bufferSize; ++i) c1RMS.setUnchecked(i, Decibels::gainToDecibels(ambisonicOrder+1.0f) + 0.5f * Decibels::gainToDecibels(c1RMS[i],-120.0f));

        c1MaxRMS = FloatVectorOperations::findMaximum(c1RMS.getRawDataPointer(), bufferSize); // for GUI
        
        FloatVectorOperations::add(c1RMS.getRawDataPointer(), -1.0 * *c1Threshold, bufferSize); // difference between RMS and threshold
        
        for (int i = 0; i<bufferSize; ++i)
        {
            envSample = c1RMS[i]>0.0 ? c1RMS[i] : 0.0f; //clipping, can maybe optimized bei FVO::clip(...)
            if (c1GR < envSample) {
                c1GR = envSample + attackGain * (c1GR - envSample);
            } else {
                c1GR = envSample + releaseGain * (c1GR - envSample);
            }
            c1GRarray.setUnchecked(i, c1GR * ratioFactor);
            c1Gains.setUnchecked(i,Decibels::decibelsToGain(c1GRarray[i] + *c1Makeup));
        }
        c1MaxGR = FloatVectorOperations::findMaximum(c1GRarray.getRawDataPointer(), bufferSize);
    }
    
    // =============== COMPRESSOR 2 ====================
    {
        // needed variables
        float envSample, attackGain, releaseGain, ratioFactor;
        // parameters
        attackGain = expf(-1000.0f / sampleRate / *c2Attack);
        releaseGain = expf(-1000.0f / sampleRate / *c2Release);
        ratioFactor = -1.0f * (1 - 1/ *c2Ratio);
        
        // set compressor driving signal
        const float* drivingSignalPtr = [this] () -> const float* {
            if (*c2DrivingSignal >= 0.5f && *c2DrivingSignal < 1.5f) return drivingPointers[0];
            else if (*c2DrivingSignal >= 1.5f) return drivingPointers[1];
            else return drivingPointers[2];
        }();
        
        // calculate RMS
        FloatVectorOperations::multiply(c2RMS.getRawDataPointer(), drivingSignalPtr, drivingSignalPtr, bufferSize);
        c2MeanSqrFilter.processSamples(c2RMS.getRawDataPointer(), bufferSize);
        for (int i = 0; i<bufferSize; ++i) c2RMS.setUnchecked(i, Decibels::gainToDecibels(ambisonicOrder+1.0f) + 0.5f * Decibels::gainToDecibels(c2RMS[i],-120.0f));
        c2MaxRMS = FloatVectorOperations::findMaximum(c2RMS.getRawDataPointer(), bufferSize); // for GUI
        
        FloatVectorOperations::add(c2RMS.getRawDataPointer(), -1.0 * *c2Threshold, bufferSize); // difference between RMS and threshold
        
        for (int i = 0; i<bufferSize; ++i)
        {
            envSample = c2RMS[i]>0.0 ? c2RMS[i] : 0.0f; //clipping, can maybe optimized bei FVO::clip(...)
            if (c2GR < envSample) {
                c2GR = envSample + attackGain * (c2GR - envSample);
            } else {
                c2GR = envSample + releaseGain * (c2GR - envSample);
            }
            c2GRarray.setUnchecked(i, c2GR * ratioFactor);
            c2Gains.setUnchecked(i, Decibels::decibelsToGain(c2GRarray[i] + *c2Makeup));
        }
        c2MaxGR = FloatVectorOperations::findMaximum(c2GRarray.getRawDataPointer(), bufferSize);
    }
    

    // =============== OUTPUT CALCULATIONS ====================
    // REMEMBER: buffer contains negative mask content
    
    // apply gains from compressor 1
    if (*c1Enabled >= 0.5f)
    {
        if (*c1Apply >= 0.5f && *c1Apply < 1.5f) //mask
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = maskBuffer.getWritePointer(channel);
                FloatVectorOperations::multiply(writeData, c1Gains.getRawDataPointer(), bufferSize);
            }
        }
        else if (*c1Apply > 1.5f) //negative mask
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = buffer.getWritePointer(channel);
                FloatVectorOperations::multiply(writeData, c1Gains.getRawDataPointer(), bufferSize);
            }
        }
        else
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = maskBuffer.getWritePointer(channel);
                FloatVectorOperations::multiply(writeData, c1Gains.getRawDataPointer(), bufferSize);
                writeData = buffer.getWritePointer(channel);
                FloatVectorOperations::multiply(writeData, c1Gains.getRawDataPointer(), bufferSize);
            }
        }
    }
    
    // apply gains from compressor 2
    if (*c2Enabled >= 0.5f)
    {
        if (*c2Apply >= 0.5f && *c2Apply < 1.5f) //mask
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = maskBuffer.getWritePointer(channel);
                FloatVectorOperations::multiply(writeData, c2Gains.getRawDataPointer(), bufferSize);
            }
        }
        else if (*c2Apply > 1.5f) //negative mask
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = buffer.getWritePointer(channel);
                FloatVectorOperations::multiply(writeData, c2Gains.getRawDataPointer(), bufferSize);
            }
        }
        else
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = maskBuffer.getWritePointer(channel);
                FloatVectorOperations::multiply(writeData, c2Gains.getRawDataPointer(), bufferSize);
                writeData = buffer.getWritePointer(channel);
                FloatVectorOperations::multiply(writeData, c2Gains.getRawDataPointer(), bufferSize);
            }
        }
    }
    
    //add channels or replace
    if (*listen >= 0.5f && *listen < 1.5f) //mask
    {
        FloatVectorOperations::copy(buffer.getWritePointer(0), maskBuffer.getReadPointer(0), numCh*bufferSize);
    }
    else if (*listen < 0.5f)
    {
        for (int chIn = 0; chIn < numCh; ++chIn)
            FloatVectorOperations::add(buffer.getWritePointer(chIn), maskBuffer.getReadPointer(chIn), bufferSize);
    }
    

    if (*useSN3D >= 0.5f)
        for (int i = 0; i < numCh; ++i)
            buffer.applyGain(i, 0, bufferSize, n3d2sn3d[i]);
}

void AmbisonicCompressorAudioProcessor::calcParams()
{
    paramChanged = false;
    
    // convert yaw and pitch to cartesian coordinates
    float cospitch = cosf(*pitch*deg2rad);
    xyz[0] = cospitch * cosf(*yaw*deg2rad);
    xyz[1] = cospitch * sinf(*yaw*deg2rad);
    xyz[2] = - sinf(*pitch*deg2rad);
    float norm = sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
    xyz[0] /= norm;
    xyz[1] /= norm;
    xyz[2] /= norm;
    //DBG("x: " << xyz[0] << " y: " << xyz[1] << " z: " << xyz[2]);
    
    
    
    for (int point=0; point<tDesignN; ++point)
    {
        //dist[point] = acosf(xyz[0]*tDesignX[point] + xyz[1]*tDesignY[point] + xyz[2]*tDesignZ[point]); // could yield nans
        dist[point] = xyz[0]*tDesignX[point] + xyz[1]*tDesignY[point] + xyz[2]*tDesignZ[point];
        dist[point] /= sqrt(tDesignX[point]*tDesignX[point] + tDesignY[point]*tDesignY[point] + tDesignZ[point]*tDesignZ[point]); // optimize by normalising tDesign on startup
        dist[point] = acosf(dist[point]);
    }
    
    float widthHalf = *width*deg2rad*0.25f; // it's actually width fourth (symmetric mask)
    widthHalf = jmax(widthHalf,FloatVectorOperations::findMinimum(dist, tDesignN));
    
    FloatVectorOperations::clip(dist, dist, widthHalf, 3*widthHalf, tDesignN);
    FloatVectorOperations::add(dist, - widthHalf, tDesignN);
    FloatVectorOperations::multiply(dist, 0.25f * M_PI / widthHalf, tDesignN);
    
    sumMaskWeights = 0.0f;
    for (int point=0; point<tDesignN; ++point)
    {
        float g = cosf(dist[point]);
        W.diagonal()[point] = g;
        sumMaskWeights += g;
    }
    
    tempMat = W*YH;
    P1 = Y*tempMat;
    
}
//==============================================================================
bool AmbisonicCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AmbisonicCompressorAudioProcessor::createEditor()
{
    return new AmbisonicCompressorAudioProcessorEditor (*this,parameters);
}

//==============================================================================
void AmbisonicCompressorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AmbisonicCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void AmbisonicCompressorAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
    const int nChannels = input.getNumberOfChannels();
    maskBuffer.setSize(nChannels, getBlockSize());
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmbisonicCompressorAudioProcessor();
}


