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
#include "../../resources/AudioProcessorBase.h"

#include "../../resources/efficientSHvanilla.h"
#include "../../resources/Quaternion.h"
#include "../../resources/Weights.h"

#define ProcessorClass DirectivityShaperAudioProcessor

#define numberOfBands 4
using namespace juce::dsp;

//==============================================================================
/**
*/
class DirectivityShaperAudioProcessor  : public AudioProcessorBase<IOTypes::AudioChannels<1>, IOTypes::Ambisonics<>>
{
public:
    constexpr static int numberOfInputChannels = 1;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    DirectivityShaperAudioProcessor();
    ~DirectivityShaperAudioProcessor();

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

    //==============================================================================
    void parameterChanged (const String &parameterID, float newValue) override;

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<RangedAudioParameter>> createParameterLayout();
    //==============================================================================


    float weights[numberOfBands][8];

    inline dsp::IIR::Coefficients<float>::Ptr createFilterCoefficients(int type, double sampleRate, double frequency, double Q);

    IIR::Filter<float> filter[numberOfBands];

    float probeGains[numberOfBands];

    Atomic<bool> repaintDV = true;
    Atomic<bool> repaintXY = true;
    Atomic<bool> repaintFV = true;
    Atomic<bool> repaintSphere = true;

    void updateBuffers() override { repaintXY = true; };

private:
    //==============================================================================
    AudioSampleBuffer filteredBuffer;

    iem::Quaternion<float> quats[numberOfBands];

    bool changeWeights = true;
    bool probeChanged = true;

    bool toggled = false;
    bool moving = false;

    float shOld[numberOfBands][64];

    // parameters
    float *orderSetting;
    float *useSN3D;
    float* filterType[numberOfBands];
    float* filterFrequency[numberOfBands];
    float* filterQ[numberOfBands];
    float* filterGain[numberOfBands];
    float* probeAzimuth;
    float* probeElevation;
    float* probeRoll;
    float* probeLock;
    float* normalization;
    float* order[numberOfBands];
    float* shape[numberOfBands];
    float* azimuth[numberOfBands];
    float* elevation[numberOfBands];
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectivityShaperAudioProcessor)
};
