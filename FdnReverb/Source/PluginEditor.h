/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Sebastian Grill
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
#include "PluginProcessor.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/FilterVisualizer.h"
#include "T60Visualizer.h"

using namespace juce::dsp;

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================

class FdnReverbAudioProcessorEditor  : public AudioProcessorEditor, private Timer, private Button::Listener, private Slider::Listener
{
public:
    FdnReverbAudioProcessorEditor (FdnReverbAudioProcessor&, AudioProcessorValueTreeState&);
    ~FdnReverbAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void buttonClicked (Button* button) override;
    void sliderValueChanged (Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FdnReverbAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    // Layout stuff (footers, headers, logos, etc.)
    TitleBar<NoIOWidget, NoIOWidget> title;
    Footer footer;
    LaF globalLaF;
    void timerCallback() override;

    SimpleLabel lbDelay, lbTime, lbDryWet, lbHighCutoff, lbHighQ, lbHighGain, lbLowCutoff, lbLowQ, lbLowGain;

    // Functional stuff (sliders, Indicators, OpenGL Voodoo magic, etc.)
    // Groups
    GroupComponent delayGroup, highsGroup, lowsGroup, t60Group, gainGroup;

    // Sliders
    ReverseSlider delayLengthSlider, revTimeSlider, dryWetSlider, highCutoffSlider, highQSlider, highGainSlider, lowCutoffSlider, lowQSlider, lowGainSlider;

    // Pointers for value tree state
    ScopedPointer<SliderAttachment> delayAttachment, feedbackAttachment, dryWetAttachment, highCutoffAttachment, highQAttachment, highGainAttachment, lowCutoffAttachment, lowQAttachment, lowGainAttachment;

    // Buttons
    ToggleButton networkOrder, freezeMode;
    ScopedPointer<ButtonAttachment> networkOrderAttachment;
    
    // filter visualization
    T60Visualizer tv;
    FilterVisualizer fv;

    IIR::Coefficients<float>::Ptr highpassCoeffs;
    IIR::Coefficients<float>::Ptr lowpassCoeffs;

    int maxPossibleChannels = 64;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FdnReverbAudioProcessorEditor)
};
