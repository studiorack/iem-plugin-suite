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
#include "../../resources/ambisonicTools.h"
#include "../../resources/Quaternion.h"
#include "../../resources/IOHelper.h"
#include "../../resources/Weights.h"
#define numberOfBands 4
using namespace juce::dsp;

//==============================================================================
/**
*/
class DirectivityShaperAudioProcessor  : public AudioProcessor,
                                        public AudioProcessorValueTreeState::Listener,
                                        public IOHelper<IOTypes::AudioChannels<1>, IOTypes::Ambisonics<>>,
                                        public VSTCallbackHandler
{
public:
    //==============================================================================
    DirectivityShaperAudioProcessor();
    ~DirectivityShaperAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect () const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //======== PluginCanDo =========================================================
    pointer_sized_int handleVstManufacturerSpecific (int32 index, pointer_sized_int value,
                                                     void* ptr, float opt) override { return 0; };
    pointer_sized_int handleVstPluginCanDo (int32 index, pointer_sized_int value,
                                            void* ptr, float opt) override;

    //==============================================================================
    void parameterChanged (const String &parameterID, float newValue) override;
    AudioProcessorValueTreeState parameters;
    float weights[numberOfBands][8];

    inline dsp::IIR::Coefficients<float>::Ptr createFilterCoefficients(int type, double sampleRate, double frequency, double Q);

    IIR::Coefficients<float>::Ptr arrayOfCoefficients[4];
    IIR::Filter<float> filter[numberOfBands];

    float probeGains[numberOfBands];

    Atomic<bool> repaintDV = true;
    Atomic<bool> repaintXY = true;
    Atomic<bool> repaintFV = true;
    Atomic<bool> repaintSphere = true;

    void updateBuffers() override { repaintXY = true; };

private:
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
