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
#ifndef M_PI
#define M_PI 3.141592654
#endif

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/interpCoeffsSIMD.h"

//==============================================================================
/**
*/
class DualDelayAudioProcessor  : public AudioProcessor,
                                        public AudioProcessorValueTreeState::Listener
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
    
    int getOrder() {return ambisonicOrder;}
    float getleftLPValue() {return *LPcutOffL;}
    
    // -- variable order --
    int maxPossibleOrder = -1;
    int ambisonicOrder = -1;
    int _ambisonicOrder = -1;
    int nChannels = 0;
    int _nChannels = 0;
    
    bool userChangedOrderSettings = false;
    void checkOrderUpdateBuffers(int userSetInputOrder);
    // --------------------
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualDelayAudioProcessor)
    
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
    void rotateBuffer(AudioBuffer<float>* bufferToRotate, int samples);
    float feedback = 0.8f;
    
    Array<IIRFilter*> lowPassFiltersLeft;
    Array<IIRFilter*> lowPassFiltersRight;
    Array<IIRFilter*> highPassFiltersLeft;
    Array<IIRFilter*> highPassFiltersRight;

//    IIRFilter* filtersLeft[4];
//    IIRFilter* filtersRight[4];
//    Array<IIRFilter*> filters;
};
