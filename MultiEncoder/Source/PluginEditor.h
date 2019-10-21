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
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/MuteSoloButton.h"
#include "../../resources/customComponents/SpherePanner.h"
#include "MasterControlWithText.h"
#include "EncoderList.h"



typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class MultiEncoderAudioProcessorEditor  : public AudioProcessorEditor,
private Timer,
private SpherePanner::Listener
{
public:
    MultiEncoderAudioProcessorEditor (MultiEncoderAudioProcessor&, AudioProcessorValueTreeState&);
    ~MultiEncoderAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void importLayout();
private:
    LaF globalLaF;
    TitleBar<AudioChannelsIOWidget<maxNumberOfInputs>, AmbisonicIOWidget<>> title;
    OSCFooter footer;

    void timerCallback() override;
    void mouseWheelOnSpherePannerMoved (SpherePanner* sphere, const MouseEvent &event, const MouseWheelDetails &wheel) override;

    MultiEncoderAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    GroupComponent masterGroup, encoderGroup;
    TextButton tbImport;

    ReverseSlider slMasterAzimuth, slMasterElevation, slMasterRoll;

    ToggleButton tbLockedToMaster;
    ComboBox inputChooser;

    SpherePanner sphere;
    SpherePanner::AzimuthElevationParameterElement masterElement;

    std::unique_ptr<SliderAttachment> slMasterAzimuthAttachment;
    std::unique_ptr<SliderAttachment> slMasterElevationAttachment;
    std::unique_ptr<SliderAttachment> slMasterRollAttachment;
    std::unique_ptr<ButtonAttachment> tbLockedToMasterAttachment;

    std::unique_ptr<ComboBoxAttachment> cbNumInputChannelsAttachment, cbNormalizationAtachment;
    std::unique_ptr<ComboBoxAttachment> cbOrderAtachment;

    Viewport viewport;
    EncoderList encoderList;

    TooltipWindow tooltipWin;

    int maxPossibleOrder = -1;
    int maxNumInputs = -1;
    int lastSetNumChIn = -1;

    // labels
    SimpleLabel lbNum;
    MasterControlWithText lbAzimuth, lbElevation, lbGain;
    SimpleLabel lbMasterAzimuth, lbMasterElevation, lbMasterRoll;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiEncoderAudioProcessorEditor)
};
