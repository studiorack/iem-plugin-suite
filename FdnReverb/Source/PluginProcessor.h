/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Sebastian Grill
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
#include "../../resources/FeedbackDelayNetwork.h"
#include "../../resources/AudioProcessorBase.h"

#define ProcessorClass FdnReverbAudioProcessor

//==============================================================================
/**
*/
class FdnReverbAudioProcessor  : public AudioProcessorBase<IOTypes::Nothing, IOTypes::Nothing>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
//==============================================================================
    FdnReverbAudioProcessor();
    ~FdnReverbAudioProcessor();

//==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;

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


    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<RangedAudioParameter>> createParameterLayout();



    int maxPossibleChannels = 64;

    void setFreezeMode (bool freezeState);
    void updateFilterParameters ();
    void getT60ForFrequencyArray (double* frequencies, double* t60Data, size_t numSamples);

private:
    //==============================================================================
	AudioBuffer<float> copyBuffer;

    // parameters (from GUI)
    float *revTime;
	float *fadeInTime;
    float *delayLength;

    float *highCutoff;
    float *highQ;
    float *highGain;
    float *lowCutoff;
    float *lowQ;
    float *lowGain;
    float *wet;

    FeedbackDelayNetwork fdn, fdnFade;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FdnReverbAudioProcessor)
};
