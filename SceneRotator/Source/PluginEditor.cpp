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

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
SceneRotatorAudioProcessorEditor::SceneRotatorAudioProcessorEditor (SceneRotatorAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState (vts), footer (p.getOSCReceiver())
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits (450, 220, 800, 500); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (String ("Scene"), String ("Rotator"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================


    // create the connection between title component's comboBoxes and parameters
    cbOrderAttachement = new ComboBoxAttachment (valueTreeState, "orderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());
    cbNormalizationAttachement = new ComboBoxAttachment (valueTreeState, "useSN3D", *title.getInputWidgetPtr()->getNormCbPointer());



    // ======================== YAW, PITCH, ROLL GROUP
    yprGroup.setText ("Yaw, Pitch & Roll");
    yprGroup.setTextLabelPosition (Justification::centredLeft);
    yprGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    yprGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible (&yprGroup);

    addAndMakeVisible (&slYaw);
    slYawAttachment = new SliderAttachment (valueTreeState, "yaw", slYaw);
    slYaw.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slYaw.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slYaw.setReverse (true);
    slYaw.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slYaw.setRotaryParameters (M_PI, 3 * M_PI, false);
    slYaw.setTooltip ("Yaw angle");
    slYaw.setTextValueSuffix (CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible (&slPitch);
    slPitchAttachment = new SliderAttachment (valueTreeState, "pitch", slPitch);
    slPitch.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slPitch.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slPitch.setReverse (true);
    slPitch.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slPitch.setRotaryParameters (0.5 * M_PI, 2.5 * M_PI, false);
    slPitch.setTooltip ("Pitch angle");
    slPitch.setTextValueSuffix (CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible (&slRoll);
    slRollAttachment = new SliderAttachment (valueTreeState, "roll", slRoll);
    slRoll.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slRoll.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slRoll.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slRoll.setReverse (false);
    slRoll.setRotaryParameters (M_PI, 3 * M_PI, false);
    slRoll.setTooltip ("Roll angle");
    slRoll.setTextValueSuffix (CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible (tbYawFlip);
    tbYawFlipAttachment = new ButtonAttachment (valueTreeState, "yawFlip", tbYawFlip);
    tbYawFlip.setColour (ToggleButton::ColourIds::tickColourId, globalLaF.ClWidgetColours[0]);
    tbYawFlip.setButtonText ("Flip");

    addAndMakeVisible (tbPitchFlip);
    tbPitchFlipAttachment = new ButtonAttachment (valueTreeState, "pitchFlip", tbPitchFlip);
    tbPitchFlip.setColour (ToggleButton::ColourIds::tickColourId, globalLaF.ClWidgetColours[1]);
    tbPitchFlip.setButtonText ("Flip");

    addAndMakeVisible (tbRollFlip);
    tbRollFlipAttachment = new ButtonAttachment (valueTreeState, "rollFlip", tbRollFlip);
    tbRollFlip.setColour (ToggleButton::ColourIds::tickColourId, globalLaF.ClWidgetColours[2]);
    tbRollFlip.setButtonText ("Flip");



    // ====================== QUATERNION GROUP
    quatGroup.setText ("Quaternions");
    quatGroup.setTextLabelPosition (Justification::centredLeft);
    quatGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    quatGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible (&quatGroup);

    addAndMakeVisible (&slQW);
    slQWAttachment = new SliderAttachment (valueTreeState, "qw", slQW);
    slQW.setSliderStyle (Slider::LinearHorizontal);
    slQW.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    slQW.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&slQX);
    slQXAttachment = new SliderAttachment (valueTreeState, "qx", slQX);
    slQX.setSliderStyle (Slider::LinearHorizontal);
    slQX.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    slQX.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&slQY);
    slQYAttachment = new SliderAttachment (valueTreeState, "qy", slQY);
    slQY.setSliderStyle (Slider::LinearHorizontal);
    slQY.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    slQY.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&slQZ);
    slQZAttachment = new SliderAttachment (valueTreeState, "qz", slQZ);
    slQZ.setSliderStyle (Slider::LinearHorizontal);
    slQZ.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    slQZ.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);


    // ================ LABELS ===================
    addAndMakeVisible (&lbYaw);
    lbYaw.setText ("Yaw");

    addAndMakeVisible (&lbPitch);
    lbPitch.setText("Pitch");

    addAndMakeVisible (&lbRoll);
    lbRoll.setText("Roll");

    addAndMakeVisible (&lbQW);
    lbQW.setText("W");

    addAndMakeVisible (&lbQX);
    lbQX.setText("X");

    addAndMakeVisible (&lbQY);
    lbQY.setText("Y");

    addAndMakeVisible (&lbQZ);
    lbQZ.setText("Z");



    // start timer after everything is set up properly
    startTimer (20);
}

SceneRotatorAudioProcessorEditor::~SceneRotatorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void SceneRotatorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void SceneRotatorAudioProcessorEditor::resized()
{
    // ============ BEGIN: header and footer ============
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());

    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);
    area.removeFromBottom (5);
    // =========== END: header and footer =================


    const int sliderHeight = 17;
    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    const int sliderSpacing = 6;
    const int rotSliderWidth = 40;
    const int labelHeight = 15;
    const int labelWidth = 20;

    auto topArea (area.removeFromTop (150));

    // -------------- Yaw Pitch Roll ------------------
    auto yprArea (topArea.removeFromLeft (160));
    yprGroup.setBounds (yprArea);
    yprArea.removeFromTop (25); //for box headline

    auto sliderRow (yprArea.removeFromTop (rotSliderHeight));
    slYaw.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    slPitch.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    slRoll.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));

    sliderRow = yprArea.removeFromTop (20);
    lbYaw.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    lbPitch.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    lbRoll.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));

    sliderRow = yprArea.removeFromTop (20);
    tbYawFlip.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    tbPitchFlip.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    tbRollFlip.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));


    // ------------- Quaternions ------------------------
    auto quatArea (topArea.removeFromRight (190));
    quatGroup.setBounds (quatArea);
    quatArea.removeFromTop (25); //for box headline

    quatArea.removeFromTop (5);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    slQW.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbQW.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    slQX.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbQX.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    slQY.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbQY.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    slQZ.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbQZ.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

}

void SceneRotatorAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize (maxInSize, maxOutSize);
    title.setMaxSize (maxInSize, maxOutSize);
    // ==========================================

    // insert stuff you want to do be done at every timer callback
}
