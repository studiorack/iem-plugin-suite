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
#include "PluginProcessor.h"

#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/customComponents/LevelMeter.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/SpherePanner.h"

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class DirectionalCompressorAudioProcessorEditor  : public AudioProcessorEditor,
private Timer, private Button::Listener
{
public:
    DirectionalCompressorAudioProcessorEditor (DirectionalCompressorAudioProcessor&, AudioProcessorValueTreeState&);
    ~DirectionalCompressorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void buttonStateChanged (Button* button) override;
    void buttonClicked (Button* button) override {};

private:
    LaF globalLaF;

    DirectionalCompressorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    TitleBar<AmbisonicIOWidget<>, NoIOWidget> title;
    OSCFooter footer;

    SpherePanner sphere;
    SpherePanner::AzimuthElevationParameterElement sphereElem;

    int maxPossibleOrder = -1;
    std::unique_ptr<ComboBoxAttachment> cbNormalizationAtachement;
    std::unique_ptr<ComboBoxAttachment> cbOrderAtachement;

    void timerCallback() override;

    GroupComponent gcMask;
    GroupComponent gcSettings;
    GroupComponent gcC1;
    GroupComponent gcC2;

    ToggleButton tbC1;
    ToggleButton tbC2;

    ReverseSlider slPreGain, slAzimuth, slElevation, slWidth;
    ReverseSlider slC1Threshold, slC1Knee, slC1Ratio, slC1Attack, slC1Release, slC1Makeup;
    ReverseSlider slC2Threshold, slC2Knee, slC2Ratio, slC2Attack, slC2Release, slC2Makeup;

    ComboBox cbC1Driving, cbC1Apply;
    ComboBox cbC2Driving, cbC2Apply;
    ComboBox cbListen;

    std::unique_ptr<SliderAttachment> slPreGainAttachment, slAzimuthAttachment, slElevationAttachment, slWidthAttachment;
    std::unique_ptr<SliderAttachment> slC1ThresholdAttachment, slC1KneeAttachment, slC1RatioAttachment;
    std::unique_ptr<SliderAttachment> slC1AttackAttachment, slC1ReleaseAttachment, slC1MakeupAttachment;
    std::unique_ptr<SliderAttachment> slC2ThresholdAttachment, slC2KneeAttachment, slC2RatioAttachment;
    std::unique_ptr<SliderAttachment> slC2AttackAttachment, slC2ReleaseAttachment, slC2MakeupAttachment;

    std::unique_ptr<ComboBoxAttachment> cbC1DrivingAttachment, cbC1ApplyAttachment;
    std::unique_ptr<ComboBoxAttachment> cbC2DrivingAttachment, cbC2ApplyAttachment;
    std::unique_ptr<ComboBoxAttachment> cbListenAttachment;

    std::unique_ptr<ButtonAttachment> tbC1Attachment, tbC2Attachment;

    LevelMeter dbC1GRmeter, dbC1RMSmeter;
    LevelMeter dbC2GRmeter, dbC2RMSmeter;

    SimpleLabel lbPreGain, lbAzimuth, lbElevation, lbWidth;
    SimpleLabel lbC1Threshold, lbC1Knee, lbC1Ratio, lbC1Attack, lbC1Release, lbC1Makeup;
    SimpleLabel lbC2Threshold, lbC2Knee, lbC2Ratio, lbC2Attack, lbC2Release, lbC2Makeup;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectionalCompressorAudioProcessorEditor)
};
