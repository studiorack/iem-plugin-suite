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
#include "../../resources/AmbisonicDecoder.h"
#include "../../resources/DecoderHelper.h"
#include "../../resources/ReferenceCountedDecoder.h"
#include "../../resources/FilterVisualizerHelper.h"

using namespace dsp;
//==============================================================================
/**
 Use the IOHelper to detect which amount of channels or which Ambisonic order is possible with the current bus layout.
 The available IOTypes are:
    - AudioChannels<maxChannelCount>
    - Ambisonics<maxOrder> (can also be used for directivity signals)
 You can leave `maxChannelCount` and `maxOrder` empty for default values (64 channels and 7th order)
*/
class SimpleDecoderAudioProcessor  :    public AudioProcessor,
                                        public AudioProcessorValueTreeState::Listener,
                                        public IOHelper<IOTypes::Ambisonics<>, IOTypes::AudioChannels<>>,
                                        public VSTCallbackHandler
{
public:
    //==============================================================================
    SimpleDecoderAudioProcessor();
    ~SimpleDecoderAudioProcessor();

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

    File getLastDir() {return lastDir;}
    void setLastDir(File newLastDir);
    void loadPreset(const File& presetFile);

    bool messageChanged {true};
    String getMessageForEditor() {return messageForEditor;}

    ReferenceCountedDecoder::Ptr getCurrentDecoderConfig()
    {
        return decoderConfig;
    }

    IIR::Coefficients<double>::Ptr cascadedHighPassCoeffs, cascadedLowPassCoeffs;
    bool updateFv {true};

private:
    void updateLowPassCoefficients (double sampleRate, float frequency);
    void updateHighPassCoefficients (double sampleRate, float frequency);
    // ====== parameters
    AudioProcessorValueTreeState parameters;

    // list of used audio parameters
    float *inputOrderSetting, *useSN3D;
    float *lowPassFrequency, *lowPassGain;
    float *highPassFrequency;

    float *swMode;
    float *swChannel;

    // =========================================

    float omniGain = 0.0f;

    File lastDir;
    File lastFile;
    ScopedPointer<PropertiesFile> properties;

    AudioBuffer<float> swBuffer;


    // processors
    ScopedPointer<IIR::Filter<float>> lowPass1;
    ScopedPointer<IIR::Filter<float>> lowPass2;
    IIR::Coefficients<float>::Ptr highPassCoeffs;
    IIR::Coefficients<float>::Ptr lowPassCoeffs;

    ProcessorDuplicator<IIR::Filter<float>, IIR::Coefficients<float>> highPass1;
    ProcessorDuplicator<IIR::Filter<float>, IIR::Coefficients<float>> highPass2;

    ProcessSpec highPassSpecs {48000, 0, 0};

    AmbisonicDecoder decoder;

    ReferenceCountedDecoder::Ptr decoderConfig {nullptr};
    String messageForEditor {""};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDecoderAudioProcessor)
};
