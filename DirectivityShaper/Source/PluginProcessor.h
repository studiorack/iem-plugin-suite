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
#define numberOfBands 4
using namespace juce::dsp;

//==============================================================================
/**
*/
class DirectivityShaperAudioProcessor  : public AudioProcessor,
                                        public AudioProcessorValueTreeState::Listener,
                        public IOHelper<IOTypes::AudioChannels<1>, IOTypes::Ambisonics<>>
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
    const float maxRe[8][8] =
    {
        {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 5.7754104119288496e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 7.7520766107019334e-01f, 4.0142037667287966e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 8.6155075887658639e-01f, 6.1340456518123299e-01f, 3.0643144179936538e-01f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 9.0644136637224459e-01f, 7.3245392600617265e-01f, 5.0224998490808703e-01f, 2.4736484001129033e-01f, 0.0f, 0.0f, 0.0f},
        {1.0f, 9.3263709143129281e-01f, 8.0471791647013236e-01f, 6.2909156744472861e-01f, 4.2321128963220900e-01f, 2.0719132924646289e-01f, 0.0f, 0.0f},
        {1.0f, 9.4921830632793713e-01f, 8.5152308960211620e-01f, 7.1432330396679700e-01f, 5.4794300713180655e-01f, 3.6475291657556469e-01f, 1.7813609450688817e-01f, 0.0f},
        {1.0f, 9.6036452263662697e-01f, 8.8345002450861454e-01f, 7.7381375334313540e-01f, 6.3791321433685355e-01f, 4.8368159255186721e-01f, 3.2000849790781744e-01f, 1.5616185043093761e-01f}
    };

    const float inPhase[8][8] =
    {
        {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 3.3333333333333331e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 5.0000000000000000e-01f, 1.0000000000000001e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 5.9999999999999998e-01f, 2.0000000000000001e-01f, 2.8571428571428571e-02f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 6.6666666666666663e-01f, 2.8571428571428570e-01f,  7.1428571428571425e-02f, 7.9365079365079361e-03f,  0.0f, 0.0f, 0.0f},
        {1.0f, 7.1428571428571430e-01f, 3.5714285714285715e-01f, 1.1904761904761904e-01f, 2.3809523809523808e-02f,  2.1645021645021645e-03f, 0.0f, 0.0f},
        {1.0f, 7.5000000000000000e-01f, 4.1666666666666669e-01f, 1.6666666666666666e-01f, 4.5454545454545456e-02f, 7.5757575757575760e-03f, 5.8275058275058275e-04f, 0.0f},
        {1.0f, 7.7777777777777779e-01f, 4.6666666666666667e-01f, 2.1212121212121213e-01f, 7.0707070707070704e-02f, 1.6317016317016316e-02f, 2.3310023310023310e-03f, 1.5540015540015540e-04f}
    };
    const float basic[8][8] =
    {
        {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    };



    AudioSampleBuffer filteredBuffer;

    iem::Quaternion<float> quats[numberOfBands];

    bool changeWeights = true;
    bool probeChanged = true;

    bool toggled = false;
    bool moving = false;

    float shOld[numberOfBands][64];
    // parameters
    float *orderSetting;
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
