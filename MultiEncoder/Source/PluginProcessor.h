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

#define maxNumberOfInputs 64
#define startNnumberOfInputs 5


#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/Quaternion.h"
#include "../../resources/efficientSHvanilla.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/IOHelper.h"
#include "../../resources/Conversions.h"

// ===== OSC ====
#include "../../resources/OSCParameterInterface.h"
#include "../../resources/OSCReceiverPlus.h"


//==============================================================================
/**
*/
class MultiEncoderAudioProcessor  : public AudioProcessor,
                                    public AudioProcessorValueTreeState::Listener,
                                    public IOHelper<IOTypes::AudioChannels<maxNumberOfInputs>, IOTypes::Ambisonics<>>,
                                    public VSTCallbackHandler,
                                    private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
    //==============================================================================
    MultiEncoderAudioProcessor();
    ~MultiEncoderAudioProcessor();

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
    OSCParameterInterface oscParams;
    OSCReceiverPlus oscReceiver;
    AudioProcessorValueTreeState parameters;

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
