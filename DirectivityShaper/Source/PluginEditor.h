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


typedef ReverseSlider::SliderAttachment SliderAttachment;
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
    LaF globalLaF;

    DirectivityShaperAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    ShapeAndOrderXyPad xyPad;

    float weights[numberOfBands][8];

    bool addedCoefficients = false;
    void timerCallback() override;

    TitleBar<AudioChannelsIOWidget<1,false>, DirectivityIOWidget> title;
    OSCFooter footer;

    int maxPossibleOrder = -1;
    int ambisonicOrder = -1;

    ComboBox cbFilterType[numberOfBands];
    ReverseSlider slFilterFrequency[numberOfBands];
    ReverseSlider slFilterQ[numberOfBands];
    ReverseSlider slFilterGain[numberOfBands];
    ReverseSlider slOrder[numberOfBands];
    ReverseSlider slShape[numberOfBands];
    ReverseSlider slAzimuth[numberOfBands];
    ReverseSlider slElevation[numberOfBands];
    ReverseSlider slProbeAzimuth;
    ReverseSlider slProbeElevation;
    ReverseSlider slProbeRoll;
    ComboBox cbDirectivityNormalization;
    ScopedPointer<ComboBoxAttachment> cbDirectivityNormalizationAttachment; // on-axis, energy


    SpherePanner sphere;
    ScopedPointer<SpherePanner::AziumuthElevationParameterElement> sphereElements[numberOfBands];
    SpherePanner::AziumuthElevationParameterElement probeElement;

    SimpleLabel lbAzimuth, lvElevation, lbOrder, lbShape;
    SimpleLabel lbProbeAzimuth, lbProbeElevation, lbProbeRoll;
    SimpleLabel lbNormalization;

    GroupComponent gcFilterBands, gcOrderAndShape, gcPanning, gcSettings;

    ToggleButton tbProbeLock;
    ScopedPointer<ComboBoxAttachment> cbFilterTypeAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slFilterFrequencyAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slFilterQAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slFilterGainAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slOrderAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slShapeAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slAzimuthAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slElevationAttachment[numberOfBands];
    ScopedPointer<SliderAttachment> slProbeAzimuthAttachment;
    ScopedPointer<SliderAttachment> slProbeElevationAttachment;
    ScopedPointer<SliderAttachment> slProbeRollAttachment;
    ScopedPointer<ButtonAttachment> tbProbeLockAttachment;
    DirectivityVisualizer dv;
    FilterVisualizer<float> fv;

    ScopedPointer<SliderAttachment> slParam1Attachment, slParam2Attachment, slParam3Attachment;
    ScopedPointer<ComboBoxAttachment> cbOrderSettingAttachment;
    ScopedPointer<ComboBoxAttachment> cbNormalizationAttachment; // n3d, sn3d

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectivityShaperAudioProcessorEditor)
};
