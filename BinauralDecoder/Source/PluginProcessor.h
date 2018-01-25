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

using namespace dsp;
//==============================================================================
/**
 Use the IOHelper to detect which amount of channels or which Ambisonic order is possible with the current bus layout.
 The available IOTypes are:
    - AudioChannels<maxChannelCount>
    - Ambisonics<maxOrder> (can also be used for directivity signals)
 You can leave `maxChannelCount` and `maxOrder` empty for default values (64 channels and 7th order)
*/
class BinauralDecoderAudioProcessor  : public AudioProcessor,
                                        public AudioProcessorValueTreeState::Listener,
                                        public IOHelper<IOTypes::Ambisonics<>, IOTypes::AudioChannels<2>>
{
    
    
public:
    //==============================================================================
    BinauralDecoderAudioProcessor();
    ~BinauralDecoderAudioProcessor();

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
    
    const StringArray headphoneEQs = {
        "AKG-K141MK2", "AKG-K240DF", "AKG-K240MK2", "AKG-K271MK2", "AKG-K271STUDIO", "AKG-K601", "AKG-K701", "AKG-K702", "AKG-K1000-Closed", "AKG-K1000-Open", "AudioTechnica-ATH-M50", "Beyerdynamic-DT250", "Beyerdynamic-DT770PRO-250Ohms", "Beyerdynamic-DT880", "Beyerdynamic-DT990PRO", "Presonus-HD7", "Sennheiser-HD430", "Sennheiser-HD480", "Sennheiser-HD560ovationII", "Sennheiser-HD565ovation", "Sennheiser-HD600", "Sennheiser-HD650", "SHURE-SRH940"
    };
    
private:
    // ====== parameters
    AudioProcessorValueTreeState parameters;
    
    // list of used audio parameters
    float* inputOrderSetting;
    float* useSN3D;
    float* applyHeadphoneEq;
    
    OwnedArray<Convolution> engines[7];
    Convolution EQ;
    
    AudioBuffer<float> stereoSum, stereoTemp;
    AudioBuffer<float> irs[7];
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BinauralDecoderAudioProcessor)
};
