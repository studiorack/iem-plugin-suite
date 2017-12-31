/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 http://www.iem.at
 
 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#define maxNumberOfInputs 64
#define startNnumberOfInputs 5

#ifndef M_PI
#define M_PI 3.141592654
#endif

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/Quaternion.h"
#include "../../resources/efficientSHvanilla.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/IOHelper.h"




//==============================================================================
/**
*/
class MultiEncoderAudioProcessor  : public AudioProcessor,
                                                public AudioProcessorValueTreeState::Listener,
public IOHelper<IOTypes::AudioChannels<maxNumberOfInputs>, IOTypes::Ambisonics<>>
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
    
    
    float xyzGrab[3];
    float xyz[maxNumberOfInputs][3];
    
    float *yaw[maxNumberOfInputs];
    float *pitch[maxNumberOfInputs];
    float *gain[maxNumberOfInputs];
    float *mute[maxNumberOfInputs];
    float *solo[maxNumberOfInputs];
    
    BigInteger muteMask;
    BigInteger soloMask;
    
    float *masterYaw;
    float *masterPitch;
    float *masterRoll;
    float *lockedToMaster;
    
    float *inputSetting;
    float *orderSetting;
    float *useSN3D;
    

    bool yprInput;
    double phi, theta;
    
    bool updateColours = false;
    
    Colour elementColours[maxNumberOfInputs];
    
    void updateBuffers() override;
    void updateQuaternions();
    
private:
    //==============================================================================
//    iem::Quaternion quat;

    bool processorUpdatingParams;
    AudioProcessorValueTreeState parameters;
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
