/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
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

//Plugin Design Essentials
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"

//Custom Components
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/MailBox.h"
#include "../../resources/customComponents/RoundButton.h"
#include "../../resources/LabelAttachment.h"


typedef ReverseSlider::SliderAttachment SliderAttachment; // all ReverseSliders will make use of the parameters' valueToText() function
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
 */
class DistanceCompensatorAudioProcessorEditor  : public AudioProcessorEditor, private Timer, private Button::Listener
{
public:
    DistanceCompensatorAudioProcessorEditor (DistanceCompensatorAudioProcessor&, AudioProcessorValueTreeState&);
    ~DistanceCompensatorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void timerCallback() override;
    void buttonClicked (Button* button) override;
    void buttonStateChanged (Button* button) override;

private:
    // ====================== begin essentials ==================
    LaF globalLaF;

    DistanceCompensatorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    TitleBar<AudioChannelsIOWidget<64, true>, NoIOWidget> title;
    OSCFooter footer;
    // ====================== end essentials ====================


    void updateEnableSetting (const int ch);
    void showControls (const int nCh);

    std::unique_ptr<ComboBoxAttachment> cbInputChannelsSettingAttachment;


    Label lbSpeedOfSound;
    std::unique_ptr<LabelAttachment> lbSpeedOfSoundAttachment;
    SimpleLabel slbSpeedOfSound;

    Label lbDistanceExponent;
    std::unique_ptr<LabelAttachment> lbDistanceExponentAttachment;
    SimpleLabel slbDistanceExponent;

    ComboBox cbGainNormalization;
    SimpleLabel slbGainNormalization;
    std::unique_ptr<ComboBoxAttachment> cbGainNormalizationAttachment;

    Label lbReferenceX, lbReferenceY, lbReferenceZ;
    std::unique_ptr<LabelAttachment> lbReferenceXAttachment, lbReferenceYAttachment, lbReferenceZAttachment;
    SimpleLabel slbReference, slbReferenceX, slbReferenceY, slbReferenceZ;


    TooltipWindow toolTipWin;

    // load
    GroupComponent gcLayout;
    TextButton btLoadFile;
    TextButton btReference;

    // buttons
    GroupComponent gcCompensation;
    ToggleButton tbEnableGains;
    ToggleButton tbEnableDelays;
    std::unique_ptr<ButtonAttachment> tbEnableGainsAttachment;
    std::unique_ptr<ButtonAttachment> tbEnableDelaysAttachment;
    std::unique_ptr<ButtonAttachment> tbEnableFiltersAttachment;

    int maxNumInputs = -1;
    int lastSetNumChIn = -1;

    // distances
    GroupComponent gcDistances;

    OwnedArray<RoundButton> tbEnableCompensation;
    OwnedArray<ButtonAttachment> tbEnableCompensationAttachment;

    OwnedArray<Label> slDistance;
    OwnedArray<LabelAttachment> slDistanceAttachment;
    OwnedArray<SimpleLabel> lbDistance;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistanceCompensatorAudioProcessorEditor)
};
