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
#include "../../resources/Quaternion.h"
#include "../../resources/efficientSHvanilla.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/AudioProcessorBase.h"
#include "../../resources/Conversions.h"

#define CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS 1
#include "../../resources/ConfigurationHelper.h"

#define ProcessorClass MultiEncoderAudioProcessor

constexpr int maxNumberOfInputs = 64;
constexpr int startNnumberOfInputs = 5;

//==============================================================================
/**
*/
class MultiEncoderAudioProcessor  : public AudioProcessorBase<IOTypes::AudioChannels<maxNumberOfInputs>, IOTypes::Ambisonics<>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    MultiEncoderAudioProcessor();
    ~MultiEncoderAudioProcessor();

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
    Result loadConfiguration (const File& configFile);
    void setLastDir (File newLastDir);
    File getLastDir() { return lastDir; };

    float xyzGrab[3];
    float xyz[maxNumberOfInputs][3];

    float *azimuth[maxNumberOfInputs];
    float *elevation[maxNumberOfInputs];
    float *gain[maxNumberOfInputs];
    float *mute[maxNumberOfInputs];
    float *solo[maxNumberOfInputs];

    BigInteger muteMask;
    BigInteger soloMask;

    float *masterAzimuth;
    float *masterElevation;
    float *masterRoll;
    float *lockedToMaster;

    float *inputSetting;
    float *orderSetting;
    float *useSN3D;


    bool yprInput;
    double phi, theta;

    bool updateColours = false;
    bool updateSphere = true;
    bool soloMuteChanged = true;

    Colour elementColours[maxNumberOfInputs];

    void updateBuffers() override;
    void updateQuaternions();

private:
    //==============================================================================
    File lastDir;
    std::unique_ptr<PropertiesFile> properties;

    bool processorUpdatingParams;

    float masterYpr[3];

    iem::Quaternion<float> quats[maxNumberOfInputs];

    float dist[maxNumberOfInputs];

    bool wasLockedBefore;
    bool locked = false;
    bool moving = false;

    float SH[maxNumberOfInputs][64];
    float _SH[maxNumberOfInputs][64];
    float _gain[maxNumberOfInputs];

    AudioBuffer<float> bufferCopy;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiEncoderAudioProcessor)
};
