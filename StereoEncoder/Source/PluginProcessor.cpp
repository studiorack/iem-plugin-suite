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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//==============================================================================
StereoEncoderAudioProcessor::StereoEncoderAudioProcessor()

#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                 .withInput("Input", AudioChannelSet::stereo(), true)
#endif
                 .withOutput("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                 ),
#endif
posC(1.0f, 0.0f, 0.0f),
posL(1.0f, 0.0f, 0.0f),
posR(1.0f, 0.0f, 0.0f),
updatedPositionData(true),
parameters(*this, nullptr)
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
    
    parameters.createAndAddParameter("qw", "Quaternion W", "",
                                     NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 1.0,
                                     [](float value) { return String(value); }, nullptr);
    parameters.createAndAddParameter("qx", "Quaternion X", "",
                                     NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
                                     [](float value) { return String(value); }, nullptr);
    parameters.createAndAddParameter("qy", "Quaternion Y", "",
                                     NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
                                     [](float value) { return String(value); }, nullptr);
    parameters.createAndAddParameter("qz", "Quaternion Z", "",
                                     NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
                                     [](float value) { return String(value); }, nullptr);
    parameters.createAndAddParameter("yaw", "Yaw angle", "deg",
                                     NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) { return String(value); }, nullptr);
    parameters.createAndAddParameter("pitch", "Pitch angle", "deg",
                                     NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) { return String(value); }, nullptr);
    parameters.createAndAddParameter("roll", "Roll angle", "deg",
                                     NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) { return String(value); }, nullptr);
    parameters.createAndAddParameter("width", "Stereo Width", "deg",
                                     NormalisableRange<float>(-360.0f, 360.0f, 0.01f), 0.0,
                                     [](float value) { return String(value); }, nullptr);
    
    parameters.createAndAddParameter("highQuality", "High-quality panning", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f,
                                     [](float value) { return value < 0.5f ? "OFF" : "ON"; }, nullptr);
    
    
    parameters.state = ValueTree(Identifier("StereoEncoder"));
    
    parameters.addParameterListener("qw", this);
    parameters.addParameterListener("qx", this);
    parameters.addParameterListener("qy", this);
    parameters.addParameterListener("qz", this);
    parameters.addParameterListener("yaw", this);
    parameters.addParameterListener("pitch", this);
    parameters.addParameterListener("roll", this);
    parameters.addParameterListener("orderSetting", this);
    
    orderSetting = parameters.getRawParameterValue("orderSetting");
    useSN3D = parameters.getRawParameterValue("useSN3D");
    qw = parameters.getRawParameterValue("qw");
    qx = parameters.getRawParameterValue("qx");
    qy = parameters.getRawParameterValue("qy");
    qz = parameters.getRawParameterValue("qz");
    yaw = parameters.getRawParameterValue("yaw");
    pitch = parameters.getRawParameterValue("pitch");
    roll = parameters.getRawParameterValue("roll");
    width = parameters.getRawParameterValue("width");
    highQuality = parameters.getRawParameterValue("highQuality");
    
    processorUpdatingParams = false;
    
    yprInput = true; //input from ypr
    
    
    FloatVectorOperations::clear(SHL, 64);
    FloatVectorOperations::clear(SHR, 64);
}

StereoEncoderAudioProcessor::~StereoEncoderAudioProcessor()
= default;

//==============================================================================
const String StereoEncoderAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool StereoEncoderAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool StereoEncoderAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double StereoEncoderAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int StereoEncoderAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int StereoEncoderAudioProcessor::getCurrentProgram() {
    return 0;
}

void StereoEncoderAudioProcessor::setCurrentProgram(int index) {
}

const String StereoEncoderAudioProcessor::getProgramName(int index) {
    return String();
}

void StereoEncoderAudioProcessor::changeProgramName(int index, const String &newName) {
}

//==============================================================================
void StereoEncoderAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    checkInputAndOutput(this, 2, *orderSetting, true);
    
    bufferCopy.setSize(2, samplesPerBlock);
    
    smoothYawL.setValue(*yaw / 180.0f * (float) M_PI);
    smoothPitchL.setValue(*pitch / 180.0f * (float) M_PI);
    
    smoothYawR.setValue(*yaw / 180.0f * (float) M_PI);
    smoothPitchR.setValue(*pitch / 180.0f * (float) M_PI);
    
    
    smoothYawL.reset(1, samplesPerBlock);
    smoothPitchL.reset(1, samplesPerBlock);
    smoothYawR.reset(1, samplesPerBlock);
    smoothPitchR.reset(1, samplesPerBlock);
}

void StereoEncoderAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations

bool StereoEncoderAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    return true;
}

#endif

void StereoEncoderAudioProcessor::processBlock(AudioSampleBuffer &buffer, MidiBuffer &midiMessages) {
    checkInputAndOutput(this, 2, *orderSetting);
    
    const int nChOut = jmin(buffer.getNumChannels(), output.getNumberOfChannels());
    const int L = buffer.getNumSamples();
    const int totalNumInputChannels = getTotalNumInputChannels() < 2 ? 1 : 2;
    const int ambisonicOrder = output.getOrder();
    
    for (int i = 0; i < totalNumInputChannels; ++i)
        bufferCopy.copyFrom(i, 0, buffer.getReadPointer(i), buffer.getNumSamples());
    buffer.clear();
    
    
    FloatVectorOperations::copy(_SHL, SHL, nChOut);
    FloatVectorOperations::copy(_SHR, SHR, nChOut);
    
    if (yprInput) {
        ypr[0] = *yaw / 180 * (float) M_PI;
        ypr[1] = *pitch / 180 * (float) M_PI;
        ypr[2] = *roll / 180 *(float) M_PI;
        
        //updating not active params
        quat.fromYPR(ypr);
        processorUpdatingParams = true;
        parameters.getParameter("qw")->setValue(parameters.getParameterRange("qw").convertTo0to1(quat.w));
        parameters.getParameter("qx")->setValue(parameters.getParameterRange("qx").convertTo0to1(quat.x));
        parameters.getParameter("qy")->setValue(parameters.getParameterRange("qy").convertTo0to1(quat.y));
        parameters.getParameter("qz")->setValue(parameters.getParameterRange("qz").convertTo0to1(quat.z));
        processorUpdatingParams = false;
    } else {
        quat = iem::Quaternion<float>(*qw, *qx, *qy, *qz);
        quat.normalize();
        quat.toYPR(ypr);
        
        //updating not active params
        processorUpdatingParams = true;
        parameters.getParameter("yaw")->setValue(
                                                 parameters.getParameterRange("yaw").convertTo0to1(ypr[0] / (float) M_PI * 180));
        parameters.getParameter("pitch")->setValue(
                                                   parameters.getParameterRange("pitch").convertTo0to1(ypr[1] / (float) M_PI * 180));
        parameters.getParameter("roll")->setValue(
                                                  parameters.getParameterRange("roll").convertTo0to1(ypr[2] / (float) M_PI * 180));
        processorUpdatingParams = false;
    }
    
    quat.toCartesian(xyz);
    
    quatLRot = iem::Quaternion<float>(cos(*width / 4 / 180 * M_PI), 0.0f, 0.0f, sin(*width / 4 / 180 * M_PI));
    quatL = quat * quatLRot;
    quatR = quat * (quatLRot.getConjugate());
    
    quatL.toCartesian(xyzL);
    quatR.toCartesian(xyzR);
    
    //TODO: refactor into inline functions
    float yawL, yawR, pitchL, pitchR, hypxy;
    hypxy = sqrt(xyzL[0] * xyzL[0] + xyzL[1] * xyzL[1]);
    yawL = atan2(xyzL[1], xyzL[0]);
    pitchL = atan2(hypxy, xyzL[2])-M_PI/2;
    
    hypxy = sqrt(xyzR[0] * xyzR[0] + xyzR[1] * xyzR[1]);
    yawR = atan2(xyzR[1], xyzR[0]);
    pitchR = atan2(hypxy, xyzR[2])-M_PI/2;
    
    
    
    if (*highQuality < 0.5f)
    {
//        smoothYawL.setValue(yawL, true);
//        smoothPitchL.setValue(pitchL, true);
//        smoothYawR.setValue(yawR, true);
//        smoothPitchR.setValue(pitchR, true);
        
        
        SHEval(ambisonicOrder, xyzL[0], xyzL[1], xyzL[2], SHL);
        SHEval(ambisonicOrder, xyzR[0], xyzR[1], xyzR[2], SHR);
        
        if (*useSN3D > 0.5f) {
            FloatVectorOperations::multiply(SHL, SHL, n3d2sn3d, nChOut);
            FloatVectorOperations::multiply(SHR, SHR, n3d2sn3d, nChOut);
        }
        
        const float *leftIn = bufferCopy.getReadPointer(0);
        const float *rightIn = bufferCopy.getReadPointer(1);
        for (int i = 0; i < nChOut; ++i) {
            buffer.copyFromWithRamp(i, 0, leftIn, buffer.getNumSamples(), _SHL[i], SHL[i]);
            buffer.addFromWithRamp(i, 0, rightIn, buffer.getNumSamples(), _SHR[i], SHR[i]);
        }
    }
    else // high-quality sampling
    {
        if (smoothYawL.getTargetValue() - yawL > M_PI)
        {
            smoothYawL.setValue(smoothYawL.getTargetValue() - 2.0f * M_PI);
            smoothYawL.reset(1,L);
        }
        else if (yawL - smoothYawL.getTargetValue() > M_PI)
        {
            smoothYawL.setValue(smoothYawL.getTargetValue() + 2.0f * M_PI);
            smoothYawL.reset(1,L);
        }
        
        if (smoothPitchL.getTargetValue() - pitchL > M_PI)
        {
            smoothPitchL.setValue(smoothPitchL.getTargetValue() - 2.0f * M_PI);
            smoothPitchL.reset(1,L);
        }
        else if (pitchL - smoothPitchL.getTargetValue() > M_PI)
        {
            smoothPitchL.setValue(smoothPitchL.getTargetValue() + 2.0f * M_PI);
            smoothPitchL.reset(1,L);
        }
        
        if (smoothYawR.getTargetValue() - yawR > M_PI)
        {
            smoothYawR.setValue(smoothYawR.getTargetValue() - 2.0f * M_PI);
            smoothYawR.reset(1,L);
        }
        else if (yawR - smoothYawR.getTargetValue() > M_PI)
        {
            smoothYawR.setValue(smoothYawR.getTargetValue() + 2.0f * M_PI);
            smoothYawR.reset(1,L);
        }
        
        if (smoothPitchR.getTargetValue() - pitchR > M_PI)
        {
            smoothPitchR.setValue(smoothPitchR.getTargetValue() - 2.0f * M_PI);
            smoothPitchR.reset(1,L);
        }
        else if (pitchR - smoothPitchR.getTargetValue() > M_PI)
        {
            smoothPitchR.setValue(smoothPitchR.getTargetValue() + 2.0f * M_PI);
            smoothPitchR.reset(1,L);
        }
        
        
        smoothYawL.setValue(yawL);
        smoothPitchL.setValue(pitchL);
        smoothYawR.setValue(yawR);
        smoothPitchR.setValue(pitchR);
        
        for (int i = 0; i < L; ++i)
        {
            const float yaw = smoothYawL.getNextValue();
            const float pitch = smoothPitchL.getNextValue();
            const float cosPitch = cos(pitch);
            float sample = bufferCopy.getSample(0, i);
            SHEval(ambisonicOrder, cosPitch * cos(yaw), cosPitch * sin(yaw), sin(-1.0f * pitch), SHL);
            
            for (int ch = 0; ch < nChOut; ++ch) {
                buffer.setSample(ch, i, sample * SHL[ch]);
            }
        }
        
        for (int i = 0; i < L; ++i)
        {
            const float yaw = smoothYawR.getNextValue();
            const float pitch = smoothPitchR.getNextValue();
            const float cosPitch = cos(pitch);
            float sample = bufferCopy.getSample(1, i);
            SHEval(ambisonicOrder, cosPitch * std::cos(yaw), cosPitch * sin(yaw), sin(-1.0f * pitch), SHR);
            
            for (int ch = 0; ch < nChOut; ++ch) {
                buffer.addSample(ch, i, sample * SHR[ch]);
                
            }
        }
        
        if (*useSN3D > 0.5f) {
            for (int ch = 0; ch < nChOut; ++ch) {
                buffer.applyGain(ch, 0, L, n3d2sn3d[ch]);
            }
            FloatVectorOperations::multiply(SHL, SHL, n3d2sn3d, nChOut);
            FloatVectorOperations::multiply(SHR, SHR, n3d2sn3d, nChOut);
        }
    }

    
    // update LCR position information for GUI
    posC = Vector3D<float>(xyz[0], xyz[1], xyz[2]);
    posL = Vector3D<float>(xyzL[0], xyzL[1], xyzL[2]);
    posR = Vector3D<float>(xyzR[0], xyzR[1], xyzR[2]);
    
    updatedPositionData = true;
}

//==============================================================================
bool StereoEncoderAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor *StereoEncoderAudioProcessor::createEditor() {
    return new StereoEncoderAudioProcessorEditor(*this, parameters);
}

void StereoEncoderAudioProcessor::parameterChanged(const String &parameterID, float newValue) {
    if (!processorUpdatingParams) {
        if (parameterID == "qw" || parameterID == "qx" || parameterID == "qy" || parameterID == "qz") yprInput = false;
        else if (parameterID == "yaw" || parameterID == "pitch" || parameterID == "roll") yprInput = true;
    }
    if (parameterID == "orderSetting") userChangedIOSettings = true;
}


//==============================================================================
void StereoEncoderAudioProcessor::getStateInformation(MemoryBlock &destData) {
    //MemoryOutputStream (destData, true).writeFloat (*qw);
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void StereoEncoderAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    //*qw = MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat();
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new StereoEncoderAudioProcessor();
}


