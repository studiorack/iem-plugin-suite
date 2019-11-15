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
#include "../../resources/AudioProcessorBase.h"
#include "../../resources/AmbisonicDecoder.h"

#define CONFIGURATIONHELPER_ENABLE_DECODER_METHODS 1
#include "../../resources/ConfigurationHelper.h"

#include "../../resources/ReferenceCountedDecoder.h"
#include "../../resources/FilterVisualizerHelper.h"

#define ProcessorClass SimpleDecoderAudioProcessor

using namespace dsp;
//==============================================================================
class SimpleDecoderAudioProcessor  :   public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::AudioChannels<>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    static const StringArray weightsStrings;

    //==============================================================================
    SimpleDecoderAudioProcessor();
    ~SimpleDecoderAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

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


    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<RangedAudioParameter>> createParameterLayout();

    //======= OSC ==================================================================
    inline const bool processNotYetConsumedOSCMessage (const OSCMessage &message) override;

    //==============================================================================
    File getLastDir() {return lastDir;}
    void setLastDir(File newLastDir);
    void loadConfiguration(const File& presetFile);

    Atomic<bool> updateDecoderInfo = true;
    Atomic<bool> messageChanged {true};
    String getMessageForEditor() {return messageForEditor;}

    ReferenceCountedDecoder::Ptr getCurrentDecoderConfig()
    {
        return decoderConfig;
    }

    IIR::Coefficients<double>::Ptr cascadedHighPassCoeffs, cascadedLowPassCoeffs;
    Atomic<bool> guiUpdateLowPassCoefficients = true;
    Atomic<bool> guiUpdateHighPassCoefficients = true;
    Atomic<bool> guiUpdateLowPassGain = true;
    Atomic<bool> guiUpdateSampleRate = true;

private:
    //==============================================================================
    void updateLowPassCoefficients (double sampleRate, float frequency);
    void updateHighPassCoefficients (double sampleRate, float frequency);

    // list of used audio parameters
    float *inputOrderSetting, *useSN3D;
    float *lowPassFrequency, *lowPassGain;
    float *highPassFrequency;

    float *swMode;
    float *swChannel;
    float *weights;

    // =========================================

    float omniGain = 0.0f;

    File lastDir;
    File lastFile;
    std::unique_ptr<PropertiesFile> properties;

    AudioBuffer<float> swBuffer;


    // processors
    std::unique_ptr<IIR::Filter<float>> lowPass1;
    std::unique_ptr<IIR::Filter<float>> lowPass2;
    IIR::Coefficients<float>::Ptr highPassCoeffs;
    IIR::Coefficients<float>::Ptr lowPassCoeffs;

    ProcessorDuplicator<IIR::Filter<float>, IIR::Coefficients<float>> highPass1;
    ProcessorDuplicator<IIR::Filter<float>, IIR::Coefficients<float>> highPass2;

    dsp::Gain<float> masterGain;

    ProcessSpec highPassSpecs {48000, 0, 0};

    AmbisonicDecoder decoder;

    ReferenceCountedDecoder::Ptr decoderConfig {nullptr};
    String messageForEditor {""};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDecoderAudioProcessor)
};
