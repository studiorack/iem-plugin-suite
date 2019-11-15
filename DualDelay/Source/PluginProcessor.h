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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/interpLagrangeWeights.h"
#include "../../resources/AudioProcessorBase.h"

#define ProcessorClass DualDelayAudioProcessor

//==============================================================================
/**
*/
class DualDelayAudioProcessor  : public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::Ambisonics<>, true>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    DualDelayAudioProcessor();
    ~DualDelayAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged (const String &parameterID, float newValue) override;
    void updateBuffers() override;


    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<RangedAudioParameter>> createParameterLayout();
    //==============================================================================

private:
    //==============================================================================
    // parameters
    float* dryGain;
    float* wetGainL;
    float* wetGainR;
    float* delayTimeL;
    float* delayTimeR;
    float* rotationL;
    float* rotationR;
    float* LPcutOffL;
    float* LPcutOffR;
    float* HPcutOffL;
    float* HPcutOffR;
    float* feedbackL;
    float* feedbackR;
    float* xfeedbackL;
    float* xfeedbackR;
    float* lfoRateL;
    float* lfoRateR;
    float* lfoDepthL;
    float* lfoDepthR;
    float* orderSetting;

    float _delayL, _delayR;

    AudioBuffer<float> AudioIN;

    AudioBuffer<float> delayBufferLeft;
    AudioBuffer<float> delayBufferRight;
    AudioBuffer<float> delayOutLeft;
    AudioBuffer<float> delayOutRight;
    AudioBuffer<float> delayInLeft;
    AudioBuffer<float> delayInRight;

    AudioBuffer<float> delayTempBuffer;


    Array<float> delay;
    Array<int> interpCoeffIdx;
    Array<int> idx;


    dsp::Oscillator<float> LFOLeft, LFORight;

    int writeOffsetLeft;
    int writeOffsetRight;
    int readOffsetLeft;
    int readOffsetRight;

    float* readPointer;
    Array<float> sin_z;
    Array<float> cos_z;

    void calcParams(float phi);
    void rotateBuffer(AudioBuffer<float>* bufferToRotate, const int nChannels, const int samples);
    float feedback = 0.8f;

    OwnedArray<IIRFilter> lowPassFiltersLeft;
    OwnedArray<IIRFilter> lowPassFiltersRight;
    OwnedArray<IIRFilter> highPassFiltersLeft;
    OwnedArray<IIRFilter> highPassFiltersRight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualDelayAudioProcessor)
};
