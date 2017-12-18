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
#ifndef M_PI
#define M_PI 3.141592654
#endif

#include "../JuceLibraryCode/JuceHeader.h"
//#include "../../utils/tDesignN10.h"
#include "../hammerAitovSample.h"
#include "../../resources/Eigen/Dense"
#include "../../resources/efficientSHvanilla.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/MaxRE.h"



//==============================================================================
/**
*/
class EnergyVisualizerAudioProcessor  : public AudioProcessor,
                                        public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    EnergyVisualizerAudioProcessor();
    ~EnergyVisualizerAudioProcessor();

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
    AudioProcessorValueTreeState parameters;
    
    Array<float> rms;
    
    void checkOrderUpdateBuffers(int userSetInputOrder);
    
    int maxPossibleOrder = -1;
private:
    
    // -- variable order --
    
    int ambisonicOrder = -1;
    int _ambisonicOrder = -1;
    int nChannels = 0;
    int _nChannels = 0;
    
    bool userChangedOrderSettings = false;
    
    Eigen::DiagonalMatrix<float, 64> maxReWeights;
    
    float timeConstant;
    // parameter
    float *orderSetting, *useSN3D, *peakLevel;
    
    
    Eigen::Matrix<float,nSamplePoints,64,Eigen::ColMajor> YH;
    Eigen::Matrix<float,nSamplePoints,64,Eigen::ColMajor> workingMatrix;
    AudioSampleBuffer sampledSignals;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnergyVisualizerAudioProcessor)
};
