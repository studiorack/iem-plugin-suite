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

//Plugin Design Essentials
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"

//Custom Components
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"


typedef ReverseSlider::SliderAttachment SliderAttachment; // all ReverseSliders will make use of the parameters' valueToText() function
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class SceneRotatorAudioProcessorEditor  : public AudioProcessorEditor, private Timer
{
public:
    SceneRotatorAudioProcessorEditor (SceneRotatorAudioProcessor&, AudioProcessorValueTreeState&);
    ~SceneRotatorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;


    void timerCallback() override;

private:
    // ====================== begin essentials ==================
    // lookAndFeel class with the IEM plug-in suite design
    LaF globalLaF;

    // stored references to the AudioProcessor and ValueTreeState holding all the parameters
    SceneRotatorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;


    // title and footer component
    TitleBar<AmbisonicIOWidget<>, NoIOWidget> title;
    OSCFooter footer;
    // =============== end essentials ============

    // Attachments to create a connection between IOWidgets comboboxes
    // and the associated parameters
    ScopedPointer<ComboBoxAttachment> cbNormalizationAttachement;
    ScopedPointer<ComboBoxAttachment> cbOrderAttachement;


    ReverseSlider slYaw, slPitch, slRoll, slQW, slQX, slQY, slQZ;

    ScopedPointer<SliderAttachment> slYawAttachment;
    ScopedPointer<SliderAttachment> slPitchAttachment;
    ScopedPointer<SliderAttachment> slRollAttachment;
    ScopedPointer<SliderAttachment> slQWAttachment;
    ScopedPointer<SliderAttachment> slQXAttachment;
    ScopedPointer<SliderAttachment> slQYAttachment;
    ScopedPointer<SliderAttachment> slQZAttachment;


    // Labels and Groups
    SimpleLabel lbYaw, lbPitch, lbRoll, lbQW, lbQX, lbQY, lbQZ;
    GroupComponent quatGroup, yprGroup;

    ToggleButton tbYawFlip, tbPitchFlip, tbRollFlip;
    ScopedPointer<ButtonAttachment> tbYawFlipAttachment, tbPitchFlipAttachment, tbRollFlipAttachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneRotatorAudioProcessorEditor)
};
