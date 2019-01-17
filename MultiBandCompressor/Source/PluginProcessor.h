/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Markus Huber
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
#include <set>
#include <vector>
#include "../../resources/IOHelper.h"
#include "../../resources/FilterVisualizerHelper.h"
#include "../../resources/Compressor.h"

// ===== OSC ====
#include "../../resources/OSCParameterInterface.h"
#include "../../resources/OSCReceiverPlus.h"


#if JUCE_USE_SIMD
# define MUCO_FLOAT_TYPE juce::dsp::SIMDRegister<float>
# define MUCO_FLOAT_ELEMENTS juce::dsp::SIMDRegister<float>::size()
#else /* !JUCE_USE_SIMD */
# define MUCO_FLOAT_TYPE float
# define MUCO_FLOAT_ELEMENTS 1
#endif /* JUCE_USE_SIMD */


using namespace juce::dsp;
typedef AudioProcessorValueTreeState::ParameterLayout ParameterLayout;

static const int numFilterBands {4};

//==============================================================================
/**
 Use the IOHelper to detect which amount of channels or which Ambisonic order is possible with the current bus layout.
 The available IOTypes are:
    - AudioChannels<maxChannelCount>
    - Ambisonics<maxOrder> (can also be used for directivity signals)
 You can leave `maxChannelCount` and `maxOrder` empty for default values (64 channels and 7th order)
*/
class MultiBandCompressorAudioProcessor  : public AudioProcessor,
                                           public AudioProcessorValueTreeState::Listener,
                                           public IOHelper<IOTypes::AudioChannels<64>, IOTypes::Ambisonics<7>>,
                                           public VSTCallbackHandler,
                                           private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
    //==============================================================================
    MultiBandCompressorAudioProcessor();
    ~MultiBandCompressorAudioProcessor();

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
  
    enum class FilterBands
    {
        Low, MidLow, MidHigh, High
    };
  
    enum class FilterType
    {
        LowPass, HighPass, AllPass
    };

    double const& getCurrentSampleRate() const { return lastSampleRate; };
    IIR::Coefficients<double>::Ptr lowPassLRCoeffs[numFilterBands-1];
    IIR::Coefficients<double>::Ptr highPassLRCoeffs[numFilterBands-1];
  
    Atomic<bool> repaintFilterVisualization = false;
  
    Compressor* getCompressor(const int i) {return &compressors[i];};
    Compressor compressors[numFilterBands];


private:
    // ====== parameters
    AudioProcessorValueTreeState parameters;
    OSCParameterInterface oscParams;
    OSCReceiverPlus oscReceiver;
  
    ParameterLayout createParameterLayout();
    void calculateCoefficients(const int index);
    void copyCoeffsToProcessor();
  
    double lastSampleRate {48000};
    int numChannels;
    const int maxNumFilters;
  
    // list of used audio parameters
    float* inputChannelsSetting, *outputOrderSetting,
           *cutoffs[numFilterBands-1],
           *threshold[numFilterBands], *knee[numFilterBands],
           *makeUpGain[numFilterBands], *ratio[numFilterBands],
           *attack[numFilterBands], *release[numFilterBands],
           *maxGR[numFilterBands], *maxRMS[numFilterBands],
           *compressionEnabled[numFilterBands];
  
    std::set<int> soloEnabledArray;


    // filter coefficients
    IIR::Coefficients<float>::Ptr iirLPCoefficients[numFilterBands-1], iirHPCoefficients[numFilterBands-1],
                                  iirAPCoefficients[numFilterBands-1],
                                  iirTempLPCoefficients[numFilterBands-1],
                                  iirTempHPCoefficients[numFilterBands-1], iirTempAPCoefficients[numFilterBands-1];
  
    // filters (cascaded butterworth/linkwitz-riley filters  + allpass)
    OwnedArray<IIR::Filter<MUCO_FLOAT_TYPE>> iirLP[numFilterBands-1], iirHP[numFilterBands-1],
                                             iirLP2[numFilterBands-1], iirHP2[numFilterBands-1],
                                             iirAP[numFilterBands-1];
  
    OwnedArray<AudioBlock<MUCO_FLOAT_TYPE>> interleaved, freqBands[numFilterBands];
    AudioBlock<float> zero, temp, gains;
    AudioBuffer<float> tempBuffer;
    float* gainChannelPointer;
  
    std::vector<HeapBlock<char>> interleavedBlockData,  freqBandsBlocks[numFilterBands];
    HeapBlock<char> zeroData, tempData, gainData;
    HeapBlock<const float*> channelPointers { input.getMaxSize() };
  
    Atomic<bool> userChangedFilterSettings = true;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiBandCompressorAudioProcessor)
};
