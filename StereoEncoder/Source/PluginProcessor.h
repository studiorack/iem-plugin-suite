/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://www.iem.at
 
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

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/Quaternion.h"
#include "../../resources/efficientSHvanilla.h"
#include "../../resources/ambisonicTools.h"




//==============================================================================
/**
*/
class StereoEncoderAudioProcessor  : public AudioProcessor,
                                                public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    StereoEncoderAudioProcessor();
    ~StereoEncoderAudioProcessor();

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
    
    
    Vector3D<float> posC, posL, posR;
    
    Atomic<bool> updatedPositionData;
    
    float *orderSetting;
    float *useSN3D;
    float *qw;
    float *qx;
    float *qy;
    float *qz;
    float *yaw;
    float *pitch;
    float *roll;
    float *width;
    
    
    // -- variable order --
    int maxPossibleOrder = -1;
    int ambisonicOrder = -1;
    int _ambisonicOrder = -1;
    int nChannels = 0;
    int _nChannels = 0;
    
    bool userChangedOrderSettings = false;
    void checkOrderUpdateBuffers(int userSetOutputOrder);
    // --------------------
    
    int  useInput = 1; //0: quaternion, 1: ypr, 2: cartesian
    bool yprInput;
    iem::Quaternion<float> quat,quatL, quatR, quatLRot;
double phi, theta;

private:
    //==============================================================================

    bool processorUpdatingParams;
    AudioProcessorValueTreeState parameters;
    float ypr[3];
  
    float xyz[3];
    float xyzL[3];
    float xyzR[3];
    
    float SHL[64];
    float SHR[64];
    float _SHL[64];
    float _SHR[64];
    
    //float gains[4];
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoEncoderAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
