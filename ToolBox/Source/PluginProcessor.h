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
#include "../../resources/ambisonicTools.h"
#include "../../resources/inPhase.h"
#include "../../resources/MaxRE.h"

// ===== OSC ====
#include "../../resources/OSCParameterInterface.h"
#include "../../resources/OSCReceiverPlus.h"

//==============================================================================
/**
 Use the IOHelper to detect which amount of channels or which Ambisonic order is possible with the current bus layout.
 The available IOTypes are:
    - AudioChannels<maxChannelCount>
    - Ambisonics<maxOrder> (can also be used for directivity signals)
 You can leave `maxChannelCount` and `maxOrder` empty for default values (64 channels and 7th order)
*/
class ToolBoxAudioProcessor  :  public AudioProcessor,
                                public AudioProcessorValueTreeState::Listener,
                                public IOHelper<IOTypes::Ambisonics<7>, IOTypes::Ambisonics<7>>,
                                public VSTCallbackHandler,
                                private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
    //==============================================================================
    ToolBoxAudioProcessor();
    ~ToolBoxAudioProcessor();

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

    //======= Parameters ===========================================================
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //==============================================================================


private:
    // ====== parameters
    OSCParameterInterface oscParams;
    OSCReceiverPlus oscReceiver;
    AudioProcessorValueTreeState parameters;
    
    // list of used audio parameters
    float* inputOrderSetting;
    float* outputOrderSetting;
    float* useSn3dInput;
    float* useSn3dOutput;
    float* flipX;
    float* flipY;
    float* flipZ;
    float* loaWeights;

    bool doFlipX, doFlipY, doFlipZ;
    // flips
    BigInteger flipXMask, flipYMask, flipZMask;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolBoxAudioProcessor)
};
