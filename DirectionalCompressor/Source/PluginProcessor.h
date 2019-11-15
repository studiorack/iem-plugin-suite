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
#include "../../resources/efficientSHvanilla.h"
#include "../../resources/tDesignN7.h"
#include <Eigen/Dense>
#include "../../resources/ambisonicTools.h"
#include "../../resources/AudioProcessorBase.h"
#include "../../resources/Compressor.h"
#include "../../resources/Conversions.h"

#define ProcessorClass DirectionalCompressorAudioProcessor

//==============================================================================
/**
*/
class DirectionalCompressorAudioProcessor  : public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::Ambisonics<>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    DirectionalCompressorAudioProcessor();
    ~DirectionalCompressorAudioProcessor();

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

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<RangedAudioParameter>> createParameterLayout();
    //==============================================================================

    void parameterChanged (const String &parameterID, float newValue) override;

    float c1MaxRMS;
    float c1MaxGR;
    float c2MaxRMS;
    float c2MaxGR;

    void calcParams();
    Atomic<bool> updatedPositionData;


private:
    //==============================================================================
    void updateBuffers() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectionalCompressorAudioProcessor)

    AudioBuffer<float> omniW;
    AudioBuffer<float> maskBuffer;

    Eigen::Matrix<float,64,tDesignN> Y;
    Eigen::Matrix<float,tDesignN,64> YH;

    Eigen::DiagonalMatrix<float, tDesignN> W;
    Eigen::Matrix<float,tDesignN,64> tempMat;
    Eigen::Matrix<float,64,64> P1;

    float dist[tDesignN];

    const float *drivingPointers[3];

    Array<float> c1Gains;
    Array<float> c2Gains;

    float c1GR;
    float c2GR;

    float sumMaskWeights;

    bool paramChanged = true;

    Compressor compressor1, compressor2;
    // == PARAMETERS ==
    // settings and mask
    float *orderSetting;
    float *useSN3D;
    float *preGain;
    float *azimuth;
    float *elevation;
    float *width;
    float *listen;
    // compressor 1
    float *c1Enabled;
    float *c1DrivingSignal;
    float *c1Apply;
    float *c1Threshold;
    float *c1Knee;
    float *c1Attack;
    float *c1Release;
    float *c1Ratio;
    float *c1Makeup;
    // compressor 2
    float *c2Enabled;
    float *c2DrivingSignal;
    float *c2Apply;
    float *c2Threshold;
    float *c2Knee;
    float *c2Attack;
    float *c2Release;
    float *c2Ratio;
    float *c2Makeup;

};
