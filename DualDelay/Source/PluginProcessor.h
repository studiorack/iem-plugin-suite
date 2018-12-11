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
#include "../../resources/ambisonicTools.h"
#include "../../resources/interpLagrangeWeights.h"
#include "../../resources/IOHelper.h"

// ===== OSC ====
#include "../../resources/OSCParameterInterface.h"
#include "../../resources/OSCReceiverPlus.h"

//==============================================================================
/**
*/
class DualDelayAudioProcessor  : public AudioProcessor,
                                public AudioProcessorValueTreeState::Listener,
                                public IOHelper<IOTypes::Ambisonics<>, IOTypes::Ambisonics<>, true>,
                                public VSTCallbackHandler,
                                private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
    //==============================================================================
    DualDelayAudioProcessor();
    ~DualDelayAudioProcessor();

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

    void parameterChanged (const String &parameterID, float newValue) override;
    void updateBuffers() override;

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

    //======= Parameters ===========================================================
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //==============================================================================

private:
    //==============================================================================
    OSCParameterInterface oscParams;
    OSCReceiverPlus oscReceiver;
    AudioProcessorValueTreeState parameters;
    
    // parameters
    float* dryGain;
    float* wetGainL;
    float* wetGainR;
    float* delayTimeL;
    float* delayTimeR;
    float* rotationL;
    float* rotationR;
    float* LPcutOffL;
    float* LPcutOffR;
    float* HPcutOffL;
    float* HPcutOffR;
    float* feedbackL;
    float* feedbackR;
    float* xfeedbackL;
    float* xfeedbackR;
    float* lfoRateL;
    float* lfoRateR;
    float* lfoDepthL;
    float* lfoDepthR;
    float* orderSetting;

    float _delayL, _delayR;

    AudioBuffer<float> AudioIN;

    AudioBuffer<float> delayBufferLeft;
    AudioBuffer<float> delayBufferRight;
    AudioBuffer<float> delayOutLeft;
    AudioBuffer<float> delayOutRight;
    AudioBuffer<float> delayInLeft;
    AudioBuffer<float> delayInRight;

    AudioBuffer<float> delayTempBuffer;


    Array<float> delay;
    Array<int> interpCoeffIdx;
    Array<int> idx;


    dsp::Oscillator<float> LFOLeft, LFORight;

    int writeOffsetLeft;
    int writeOffsetRight;
    int readOffsetLeft;
    int readOffsetRight;

    float* readPointer;
    Array<float> sin_z;
    Array<float> cos_z;

    void calcParams(float phi);
    void rotateBuffer(AudioBuffer<float>* bufferToRotate, const int nChannels, const int samples);
    float feedback = 0.8f;

    OwnedArray<IIRFilter> lowPassFiltersLeft;
    OwnedArray<IIRFilter> lowPassFiltersRight;
    OwnedArray<IIRFilter> highPassFiltersLeft;
    OwnedArray<IIRFilter> highPassFiltersRight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualDelayAudioProcessor)
};
