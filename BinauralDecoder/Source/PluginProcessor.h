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
#include <fftw3.h>

#define ProcessorClass BinauralDecoderAudioProcessor

using namespace dsp;
class BinauralDecoderAudioProcessor  :  public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::AudioChannels<2>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 2;

    //==============================================================================
    BinauralDecoderAudioProcessor();
    ~BinauralDecoderAudioProcessor();

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
    void updateBuffers() override; // use this to implement a buffer update method


    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<RangedAudioParameter>> createParameterLayout();
    //==============================================================================


    static const StringArray headphoneEQs;

private:
    // list of used audio parameters
    float* inputOrderSetting;
    float* useSN3D;
    float* applyHeadphoneEq;

    Convolution EQ;

    int fftLength = -1;
    int irLength = 236;
    int irLengthMinusOne = irLength-1;
    float* in = nullptr;
    float* ifftOutputMid = nullptr;
    float* ifftOutputSide = nullptr;
    fftwf_complex* out = nullptr;
    fftwf_complex* accumMid = nullptr;
    fftwf_complex* accumSide = nullptr;

    fftwf_plan fftForward, fftBackwardMid, fftBackwardSide;
    bool fftwWasPlanned = false;

    AudioBuffer<float> stereoSum, stereoTemp;
    AudioBuffer<float> overlapBuffer;
    AudioBuffer<float> irs[7];

    AudioBuffer<float> irsFrequencyDomain;
    double irsSampleRate = 44100.0;
    //mapping between mid-channel index and channel index
    int mix2cix[36] = { 0, 2, 3, 6, 7, 8, 12, 13, 14, 15, 20, 21, 22, 23, 24, 30, 31, 32, 33, 34, 35, 42, 43, 44, 45, 46, 47, 48, 56, 57, 58, 59, 60, 61, 62, 63 };
    //mapping between side-channel index and channel index
    int six2cix[28] = { 1, 4, 5, 9, 10, 11, 16, 17, 18, 19, 25, 26, 27, 28, 29, 36, 37, 38, 39, 40, 41, 49, 50, 51, 52, 53, 54, 55 };
    int nMidCh;
    int nSideCh;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BinauralDecoderAudioProcessor)
};
