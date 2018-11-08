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
#include "../../resources/FilterVisualizerHelper.h"

// ===== OSC ====
#include "../../resources/OSCParameterInterface.h"
#include "../../resources/OSCReceiverPlus.h"

#define numFilterBands 6
using namespace juce::dsp;

#if JUCE_USE_SIMD
# define IIRfloat juce::dsp::SIMDRegister<float>
# define IIRfloat_elements() IIRfloat::SIMDNumElements
#else /* !JUCE_USE_SIMD */
# define IIRfloat float
# define IIRfloat_elements() 1
#endif /* JUCE_USE_SIMD */

//==============================================================================
/**
 Use the IOHelper to detect which amount of channels or which Ambisonic order is possible with the current bus layout.
 The available IOTypes are:
    - AudioChannels<maxChannelCount>
    - Ambisonics<maxOrder> (can also be used for directivity signals)
 You can leave `maxChannelCount` and `maxOrder` empty for default values (64 channels and 7th order)
*/
class MultiEQAudioProcessor  : public AudioProcessor,
                                        public AudioProcessorValueTreeState::Listener,
                                        public IOHelper<IOTypes::AudioChannels<64>, IOTypes::AudioChannels<64>>,
                                        public VSTCallbackHandler,
                                        private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
    //==============================================================================
    MultiEQAudioProcessor();
    ~MultiEQAudioProcessor();



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

    void updateFilterCoefficients (double sampleRate);
    void copyFilterCoefficientsToProcessor();

    IIR::Coefficients<double>::Ptr getCoefficientsForGui (const int filterIndex) { return guiCoefficients[filterIndex]; };
    void updateGuiCoefficients();
    
    // FV repaint flag
    Atomic<bool> repaintFV = true;
    
private:

    enum class RegularFilterType
    {
        FirstOrderHighPass, SecondOrderHighPass, LowShelf, PeakFilter, HighShelf, FirstOrderLowPass, SecondOrderLowPass, NothingToDo
    };

    enum class SpecialFilterType
    {
        FirstOrderHighPass, SecondOrderHighPass, LinkwitzRileyHighPass, LowShelf, FirstOrderLowPass, SecondOrderLowPass, LinkwitzRileyLowPass, HighShelf
    };

    void createLinkwitzRileyFilter (const bool isUpperBand);
    void createFilterCoefficients (const int filterIndex, const double sampleRate);


    inline dsp::IIR::Coefficients<float>::Ptr createFilterCoefficients (const RegularFilterType type, const double sampleRate, const float frequency, const float Q, const float gain);

    inline dsp::IIR::Coefficients<double>::Ptr createFilterCoefficientsForGui (const RegularFilterType type, const double sampleRate, const float frequency, const float Q, const float gain);

    // filter dummy for GUI
    IIR::Coefficients<double>::Ptr guiCoefficients[numFilterBands];

    IIR::Coefficients<float>::Ptr processorCoefficients[numFilterBands];
    IIR::Coefficients<float>::Ptr additionalProcessorCoefficients[2];

    IIR::Coefficients<float>::Ptr tempCoefficients[numFilterBands];
    IIR::Coefficients<float>::Ptr additionalTempCoefficients[2];

    // data for interleaving audio
    HeapBlock<char> interleavedBlockData[16], zeroData; //todo: dynamically?
    OwnedArray<AudioBlock<IIRfloat>> interleavedData;
    AudioBlock<float> zero;


    // ====== parameters
    AudioProcessorValueTreeState parameters;
    OSCParameterInterface oscParams;
    OSCReceiverPlus oscReceiver;
    
    // list of used audio parameters
    float *inputChannelsSetting;
    float* filterEnabled[numFilterBands];
    float* filterType[numFilterBands];
    float* filterFrequency[numFilterBands];
    float* filterQ[numFilterBands];
    float* filterGain[numFilterBands];

    // filters for processing
    OwnedArray<IIR::Filter<IIRfloat>> filterArrays[numFilterBands];
    OwnedArray<IIR::Filter<IIRfloat>> additionalFilterArrays[2];

    Atomic<bool> userHasChangedFilterSettings = true;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiEQAudioProcessor)
};
