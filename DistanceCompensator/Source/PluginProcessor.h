/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
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
using namespace juce::dsp;

#include "../../resources/AudioProcessorBase.h"
#include "../../resources/customComponents/MailBox.h"

#define CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS 1
#include "../../resources/ConfigurationHelper.h"

#include "../../resources/Conversions.h"
#include "../../resources/MultiChannelGain.h"
#include "../../resources/MultiChannelDelay.h"

#define ProcessorClass DistanceCompensatorAudioProcessor

//==============================================================================
class DistanceCompensatorAudioProcessor  : public AudioProcessorBase<IOTypes::AudioChannels<64>, IOTypes::AudioChannels<64>>
{
    struct PositionAndChannel
    {
        Vector3D<float> position;
        int channel;
    };

public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    DistanceCompensatorAudioProcessor();
    ~DistanceCompensatorAudioProcessor();

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
    inline const bool processNotYetConsumedOSCMessage (const OSCMessage &message) override;


    //==============================================================================
    void setLastDir (File newLastDir);
    File getLastDir() {return lastDir;};

    void loadConfiguration (const File& presetFile);

    float distanceToGainInDecibels (const float distance);
    float distanceToDelayInSeconds (const float distance);

    void updateDelays();
    void updateGains();
    void updateParameters();

    bool updateMessage = false;

    MailBox::Message messageToEditor;

private:
    //==============================================================================
    Atomic<bool> updatingParameters = false;

    // list of used audio parameters
    float *inputChannelsSetting;
    float *speedOfSound;
    float *distanceExponent;
    float *gainNormalization;
    float *referenceX;
    float *referenceY;
    float *referenceZ;
    float *enableGains;
    float *enableDelays;

    float *enableCompensation[64];
    float *distance[64];

    // ===== last directory loaded from
    File lastDir;
    std::unique_ptr<PropertiesFile> properties;

    Array<float> tempValues;

    Array<PositionAndChannel> loadedLoudspeakerPositions;

    // processors
    MultiChannelGain<float> gain;
    MultiChannelDelay<float> delay;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistanceCompensatorAudioProcessor)
};
