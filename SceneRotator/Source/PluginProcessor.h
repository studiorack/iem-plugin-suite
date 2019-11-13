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

#include "../../resources/Conversions.h"
#include "../../resources/Quaternion.h"
#include "../../resources/ReferenceCountedMatrix.h"

#define ProcessorClass SceneRotatorAudioProcessor

//==============================================================================
class SceneRotatorAudioProcessor  : public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::Ambisonics<>, true>,
                                    private MidiMessageCollector,
                                    private Timer
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    SceneRotatorAudioProcessor();
    ~SceneRotatorAudioProcessor();

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

    //======= OSC ==================================================================
    inline const bool interceptOSCMessage (OSCMessage &message) override;

    //==============================================================================
    inline void updateQuaternions();
    inline void updateEuler();

    void rotateBuffer (AudioBuffer<float>* bufferToRotate, const int nChannels, const int samples);
    void calcRotationMatrix (const int order);

    //======= MIDI Connection ======================================================
    enum class MidiScheme
    {
        none = 0,
        mrHeadTrackerYprDir,
        mrHeadTrackerYprInv,
        mrHeadTrackerQuaternions
    };

    const StringArray midiSchemeNames
    {
        "none (link only)",
        "MrHT YPR Direct",
        "MrHT YPR Inverse",
        "MrHT Quaternions"
    };

    const Identifier midiSchemeIdentifieres[4]
    {
        "none",
        "MrHT_YprDir",
        "MrHT_YprInv",
        "MrHT_Quat"
    };

    String getCurrentMidiDeviceName();
    void openMidiInput (String midiDeviceName, bool forceUpdatingCurrentMidiDeviceName = false);
    void closeMidiInput();

    const StringArray getMidiSchemes() { return midiSchemeNames; };
    MidiScheme getCurrentMidiScheme() { return currentMidiScheme; };
    void setMidiScheme (MidiScheme newMidiScheme);

    //==============================================================================
    // Flags for editor
    Atomic<bool> deviceHasChanged = false;
    Atomic<bool> showMidiOpenError = false;
    Atomic<bool> schemeHasChanged = false;

private:
    //==============================================================================

    Atomic<bool> usingYpr = true;

    // list of used audio parameters
    float* orderSetting;
    float* useSN3D;

    float* yaw;
    float* pitch;
    float* roll;
    float* qw;
    float* qx;
    float* qy;
    float* qz;
    float* invertYaw;
    float* invertPitch;
    float* invertRoll;
    float* invertQuaternion;
    float* rotationSequence;

    Atomic<bool> updatingParams {false};
    Atomic<bool> rotationParamsHaveChanged {true};

    AudioBuffer<float> copyBuffer;

    OwnedArray<Matrix<float>> orderMatrices;
    OwnedArray<Matrix<float>> orderMatricesCopy;

    double P (int i, int l, int a, int b, Matrix<float>& R1, Matrix<float>& Rlm1);
    double U (int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1);
    double V (int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1);
    double W (int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1);

    void timerCallback() override;

    // ============ MIDI Device Connection ======================
    // MrHeadTracker 14-bit MIDI Data
    int yawLsb = 0, pitchLsb = 0, rollLsb = 0;
    int qwLsb = 0, qxLsb = 0, qyLsb = 0, qzLsb = 0;

    std::unique_ptr<MidiInput> midiInput;
    String currentMidiDeviceName = "";
    MidiScheme currentMidiScheme = MidiScheme::none;
    CriticalSection changingMidiDevice;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneRotatorAudioProcessor)
};
