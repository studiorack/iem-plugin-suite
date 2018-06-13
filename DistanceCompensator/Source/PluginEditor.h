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
    Footer footer;
    // ====================== end essentials ====================

    ScopedPointer<ComboBoxAttachment> cbInputChannelsSettingAttachment;


    RoundButton tbDelay;

    // load
    GroupComponent gcLayout;
    TextButton btLoadFile;

    // buttons
    GroupComponent gcCompensation;
    ToggleButton tbEnableGains;
    ToggleButton tbEnableDelays;
    ScopedPointer<ButtonAttachment> tbEnableGainsAttachment;
    ScopedPointer<ButtonAttachment> tbEnableDelaysAttachment;
    ScopedPointer<ButtonAttachment> tbEnableFiltersAttachment;


    // distances
    GroupComponent gcDistances;
    OwnedArray<ReverseSlider> slDistance;
    OwnedArray<SliderAttachment> slDistanceAttachment;
    OwnedArray<SimpleLabel> lbDistance;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistanceCompensatorAudioProcessorEditor)
};
