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
#include "../../resources/IOHelper.h"

// ===== OSC ====
#include "../../resources/OSCParameterInterface.h"
#include "../../resources/OSCReceiverPlus.h"

#include "../../resources/Conversions.h"
#include "../../resources/Quaternion.h"
#include "../../resources/ReferenceCountedMatrix.h"


//==============================================================================
/**
 Use the IOHelper to detect which amount of channels or which Ambisonic order is possible with the current bus layout.
 The available IOTypes are:
    - AudioChannels<maxChannelCount>
    - Ambisonics<maxOrder> (can also be used for directivity signals)
 You can leave `maxChannelCount` and `maxOrder` empty for default values (64 channels and 7th order)
*/
class SceneRotatorAudioProcessor  : public AudioProcessor,
                                        public AudioProcessorValueTreeState::Listener,
                                        public IOHelper<IOTypes::Ambisonics<>, IOTypes::Ambisonics<>, true>,
                                        public VSTCallbackHandler,
                                        private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
    //==============================================================================
    SceneRotatorAudioProcessor();
    ~SceneRotatorAudioProcessor();

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
    void updateBuffers() override; // use this to implement a buffer update method

    //======== PluginCanDo =========================================================
    pointer_sized_int handleVstManufacturerSpecific (int32 index, pointer_sized_int value,
                                                     void* ptr, float opt) override { return 0; };

    pointer_sized_int handleVstPluginCanDo (int32 index, pointer_sized_int value,
                                            void* ptr, float opt) override;
    //==============================================================================


    //======== OSC =================================================================
    void oscMessageReceived (const OSCMessage &message) override;
    void oscBundleReceived (const OSCBundle &bundle) override;
    OSCReceiverPlus& getOSCReceiver () { return oscReceiver; }
    //==============================================================================

    inline void updateQuaternions();
    inline void updateEuler();
    

    void rotateBuffer (AudioBuffer<float>* bufferToRotate, const int nChannels, const int samples);
    void calcRotationMatrix (const int order);

private:
    // ====== parameters
    AudioProcessorValueTreeState parameters;
    OSCParameterInterface oscParams;
    OSCReceiverPlus oscReceiver;

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
    float* rotationOrder;

    Atomic<bool> updatingParams {false};
    Atomic<bool> rotationParamsHaveChanged {true};

    AudioBuffer<float> copyBuffer;
    
    OwnedArray<Matrix<float>> orderMatrices;
    OwnedArray<Matrix<float>> orderMatricesCopy;

    double P (int i, int l, int a, int b, Matrix<float>& R1, Matrix<float>& Rlm1);
    double U (int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1);
    double V (int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1);
    double W (int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1);
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneRotatorAudioProcessor)
};
