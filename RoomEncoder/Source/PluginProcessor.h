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

#define JUCE_USE_VDSP_FRAMEWORK 1

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/Quaternion.h"
#include "../../resources/interpCoeffsSIMD.h"
#include "../../resources/efficientSHvanilla.h"
#include "reflections.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/IOHelper.h"
#include "../../resources/customComponents/FilterVisualizer.h"


#ifdef JUCE_MAC
#define VIMAGE_H // avoid namespace clashes
#include <Accelerate/Accelerate.h>
#endif

const int mSig[] = {1,-1};
using namespace juce::dsp;

struct RoomParams {
    bool validRoomData = false;
    bool validListenerData = false;
    bool validReflectionData = false;
    float roomX, roomY, roomZ;
    float listenerX, listenerY, listenerZ;
    float reflCoeff, lowShelfFreq, lowShelfGain, highShelfFreq, highShelfGain, numRefl;
};

struct SharedParams {
    SharedParams()
    {
        rooms.add(RoomParams());
        rooms.add(RoomParams());
        rooms.add(RoomParams());
        rooms.add(RoomParams());
    }
    Array<RoomParams> rooms;
};

//==============================================================================
/**
*/
class RoomEncoderAudioProcessor  : public AudioProcessor,
                                    public AudioProcessorValueTreeState::Listener,
                                    private Timer,
        public IOHelper<IOTypes::Ambisonics<>, IOTypes::Ambisonics<>>
{
public:
    //==============================================================================
    RoomEncoderAudioProcessor();
    ~RoomEncoderAudioProcessor();

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
    
    double oldDelay[nImgSrc];
    //float oldRGain[nImgSrc];
    float allGains[nImgSrc];
    //float* oldDelayPtr;
    
    
    //filter coefficients
    IIR::Coefficients<float>::Ptr lowShelfCoefficients;
    IIR::Coefficients<float>::Ptr highShelfCoefficients;
    
    void setFilterVisualizer(FilterVisualizer* newFv);
    bool userChangedFilterSettings = true;
    bool updateFv = false;
    
    void timerCallback() override;
    

    void updateFilterCoefficients(int sampleRate);

    float* numRefl;
    float mRadius[nImgSrc];
    
    void updateBuffers() override;
    
private:
    //==============================================================================
    AudioProcessorValueTreeState parameters;
    
    bool readingSharedParams = false;;
    
    double phi;
    double theta;
    
    FilterVisualizer* editorFv = nullptr;
    
    // Parameters
    float *directivityOrderSetting;
    float *orderSetting;
    float *useSN3D;
    
    float* roomX;
    float* roomY;
    float* roomZ;
    
    float* sourceX;
    float* sourceY;
    float* sourceZ;
    
    float* listenerX;
    float* listenerY;
    float* listenerZ;
    
    float* reflCoeff;
    
    float* lowShelfFreq;
    float* lowShelfGain;
    float* highShelfFreq;
    float* highShelfGain;
    
    float* syncChannel;
    float* syncRoomSize;
    float* syncReflection;
    float* syncListener;
    
    int _numRefl;
    
    SharedResourcePointer<SharedParams> sharedParams;
    
    //SIMD IIR Filter
    OwnedArray<IIR::Filter<SIMDRegister<float>>> lowShelfArray;
    OwnedArray<IIR::Filter<SIMDRegister<float>>> highShelfArray;
    OwnedArray<IIR::Filter<SIMDRegister<float>>> lowShelfArray2;
    OwnedArray<IIR::Filter<SIMDRegister<float>>> highShelfArray2;
    HeapBlock<char> interleavedBlockData[16], zeroData; //todo: dynamically?
    OwnedArray<AudioBlock<SIMDRegister<float>>> interleavedData;
    AudioBlock<float> zero;
    
    
    Vector3D<float> sourcePos, listenerPos;
    float h,b,t;
    
    float mx[nImgSrc];
    float my[nImgSrc];
    float mz[nImgSrc];
    float smx[nImgSrc];
    float smy[nImgSrc];
    float smz[nImgSrc];
    
    
    float hypxy;
    int bufferSize;
    int bufferReadIdx;
    int overflow;
    int temp;
    

    int readOffset;
    
    float powReflCoeff[maxOrderImgSrc+1];
    double dist2smpls;

    float *tempAddr;
    float SHcoeffsOld[nImgSrc][64];
    SIMDRegister<float> SHsampleOld[nImgSrc][16]; //TODO: can be smaller: (N+1)^2/SIMDRegister.size()
    float weightedSample;
    
    AudioBuffer<float> delayBuffer;
    AudioBuffer<float> monoBuffer;
    
    float** delayBufferWritePtrArray;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomEncoderAudioProcessor)
};
