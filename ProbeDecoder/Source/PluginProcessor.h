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
#include "../../resources/AudioProcessorBase.h"
#include "../../resources/Conversions.h"

#define ProcessorClass ProbeDecoderAudioProcessor

//==============================================================================
/**
*/
class ProbeDecoderAudioProcessor  : public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::AudioChannels<1>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 1;
    //==============================================================================
    ProbeDecoderAudioProcessor();
    ~ProbeDecoderAudioProcessor();

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

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<RangedAudioParameter>> createParameterLayout();

    //==============================================================================
    float *orderSetting;
    float *useSN3D;

    Atomic<bool> updatedPositionData {true};

private:
    //==============================================================================

    float *azimuth;
    float *elevation;

    float previousSH[64];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProbeDecoderAudioProcessor)
};
