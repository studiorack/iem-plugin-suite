/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Copyright (c) 2017 - Daniel Rudrich
 www.iem.at
 
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

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//Plugin Design Essentials
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"

//Custom Components
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "ShapeAndOrderXyPad.h"
#include "DirectivityVisualizer.h"
#include "../../resources/customComponents/FilterVisualizer.h"
#include "../../resources/customComponents/SpherePanner.h"


typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class DirectivityShaperAudioProcessorEditor  : public AudioProcessorEditor, private Timer
{
public:
    DirectivityShaperAudioProcessorEditor (DirectivityShaperAudioProcessor&, AudioProcessorValueTreeState&);
    ~DirectivityShaperAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DirectivityShaperAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    ShapeAndOrderXyPad xyPad;
    
    float weights[numberOfBands][8];
    
    bool addedCoefficients = false;
    void timerCallback() override;
    LaF globalLaF;
    TitleBar<AudioChannelsIOWidget<1,false>, DirectivityIOWidget> title;
    Footer footer;

    int maxPossibleOrder = -1;
    int ambisonicOrder = -1;

    ComboBox cbFilterType[numberOfBands];
    Slider slFilterFrequency[numberOfBands];
    Slider slFilterQ[numberOfBands];
    Slider slFilterGain[numberOfBands];
    Slider slOrder[numberOfBands];
    Slider slShape[numberOfBands];
    ReverseSlider slYaw[numberOfBands];
    ReverseSlider slPitch[numberOfBands];
    ReverseSlider slMasterYaw;
    ReverseSlider slMasterPitch;
    ReverseSlider slMasterRoll;
    ComboBox cbNormalization;
    ScopedPointer<ComboBoxAttachment> cbNormalizationAttachment;
    
    SpherePanner sphere;
    SpherePanner::Element sphereElements[numberOfBands];
    SpherePanner::Element masterElement;
    
    SimpleLabel lbYaw, lbPitch, lbOrder, lbShape;
    SimpleLabel lbProbeYaw, lbProbePitch, lbProbeRoll;
    SimpleLabel lbNormalization;
    
    GroupComponent gcFilterBands, gcOrderAndShape, gcPanning, gcSettings;
    
    ToggleButton tbMasterToggle;
    ScopedPointer<ComboBoxAttachment> cbFilterTypeAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slFilterFrequencyAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slFilterQAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slFilterGainAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slOrderAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slShapeAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slYawAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slPitchAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slMasterYawAttachment;
    ScopedPointer<SliderAttachment> slMasterPitchAttachment;
    ScopedPointer<SliderAttachment> slMasterRollAttachment;
    ScopedPointer<ButtonAttachment> tbMasterToggleAttachment;
    DirectivityVisualizer dv;
    FilterVisualizer fv;
    
    ScopedPointer<SliderAttachment> slParam1Attachment, slParam2Attachment, slParam3Attachment;
    ScopedPointer<ComboBoxAttachment> cbOrderSettingAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectivityShaperAudioProcessorEditor)
};
