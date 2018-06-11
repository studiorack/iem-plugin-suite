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
OmniCompressorAudioProcessorEditor::OmniCompressorAudioProcessorEditor (OmniCompressorAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts), characteristic(processor.compressor)
{
    setSize (330, 500);
    setLookAndFeel(&globalLaF);

    addAndMakeVisible(&title);
    title.setTitle(String("Omni"),String("Compressor"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);
    addAndMakeVisible(&footer);

    addAndMakeVisible(characteristic);

    addAndMakeVisible(&tbLookAhead);
    tbLookAheadAttachment = new ButtonAttachment(valueTreeState, "lookAhead", tbLookAhead);
    tbLookAhead.setButtonText("Look ahead (5ms)");
    tbLookAhead.setColour (ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible(&sliderKnee);
    KnAttachment = new SliderAttachment(valueTreeState,"knee", sliderKnee);
    sliderKnee.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sliderKnee.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    sliderKnee.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    sliderKnee.setTextValueSuffix(" dB");

    cbNormalizationAtachement = new ComboBoxAttachment(valueTreeState,"useSN3D", *title.getInputWidgetPtr()->getNormCbPointer());
    cbOrderAtachement = new ComboBoxAttachment(valueTreeState,"orderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());

    addAndMakeVisible(&sliderThreshold);
    ThAttachment = new SliderAttachment(valueTreeState,"threshold", sliderThreshold);
    sliderThreshold.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sliderThreshold.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    sliderThreshold.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    sliderThreshold.setTextValueSuffix(" dB");

    addAndMakeVisible(&sliderRatio);
    RaAttachment = new SliderAttachment(valueTreeState,"ratio", sliderRatio);
    sliderRatio.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sliderRatio.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    sliderRatio.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
//    sliderRatio.setTextValueSuffix("");

    addAndMakeVisible(&sliderAttackTime);
    ATAttachment = new SliderAttachment(valueTreeState,"attack", sliderAttackTime);
    sliderAttackTime.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sliderAttackTime.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    sliderAttackTime.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    sliderAttackTime.setTextValueSuffix(" ms");

    addAndMakeVisible(&sliderReleaseTime);
    RTAttachment = new SliderAttachment(valueTreeState,"release", sliderReleaseTime);
    sliderReleaseTime.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sliderReleaseTime.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    sliderReleaseTime.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    sliderReleaseTime.setTextValueSuffix(" ms");

    addAndMakeVisible(&sliderMakeupGain);
    MGAttachment = new SliderAttachment(valueTreeState,"outGain", sliderMakeupGain);
    sliderMakeupGain.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sliderMakeupGain.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    sliderMakeupGain.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    sliderMakeupGain.setTextValueSuffix(" dB");



    addAndMakeVisible(&dbGRmeter);
    dbGRmeter.setMinLevel(-25.0f);
    dbGRmeter.setColour(Colours::red.withMultipliedAlpha(0.8f));
    dbGRmeter.setGainReductionMeter(true);

    addAndMakeVisible(&inpMeter);
    inpMeter.setMinLevel(-60.0f);
    inpMeter.setColour(Colours::green.withMultipliedAlpha(0.8f));
    inpMeter.setGainReductionMeter(false);



    // ===== LABELS =====
    addAndMakeVisible(&lbKnee);
    lbKnee.setText("Knee");

    addAndMakeVisible(&lbThreshold);
    lbThreshold.setText("Threshold");

    addAndMakeVisible(&lbOutGain);
    lbOutGain.setText("Makeup");

    addAndMakeVisible(&lbRatio);
    lbRatio.setText("Ratio");

    addAndMakeVisible(&lbAttack);
    lbAttack.setText("Attack");

    addAndMakeVisible(&lbRelease);
    lbRelease.setText("Release");


    startTimer(50);
}

OmniCompressorAudioProcessorEditor::~OmniCompressorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void OmniCompressorAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (globalLaF.ClBackground);


}

void OmniCompressorAudioProcessorEditor::timerCallback()
{
    characteristic.setMarkerLevels(processor.maxRMS, processor.maxGR);
    characteristic.updateCharacteristic();
    characteristic.repaint();

    inpMeter.setLevel(processor.maxRMS);
    dbGRmeter.setLevel(processor.maxGR);

    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    // ==========================================
}


void OmniCompressorAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int sliderHeight = 70;
    const int labelHeight = 15;
    const int sliderSpacing = 20;
    const int sliderWidth = 55;
    Rectangle<int> area (getLocalBounds());

    Rectangle<int> footerArea (area.removeFromBottom(footerHeight));
    footer.setBounds(footerArea);

    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    area.removeFromBottom(5);



    Rectangle<int> ctrlPlane = area.removeFromBottom(180);
    ctrlPlane.setWidth(270);
    ctrlPlane.setCentre(area.getCentreX(), ctrlPlane.getCentreY());

    inpMeter.setBounds(ctrlPlane.removeFromLeft(20));
    ctrlPlane.removeFromLeft(10);
    dbGRmeter.setBounds(ctrlPlane.removeFromRight(20));
    ctrlPlane.removeFromRight(10);

    Rectangle<int> sliderRow;

    sliderRow = ctrlPlane.removeFromTop(sliderHeight);

    sliderThreshold.setBounds(sliderRow.removeFromLeft(sliderWidth));
    sliderRow.removeFromLeft(sliderSpacing);
    sliderKnee.setBounds(sliderRow.removeFromLeft(sliderWidth));
    sliderRow.removeFromLeft(sliderSpacing);
    sliderMakeupGain.setBounds(sliderRow.removeFromLeft(sliderWidth));

    sliderRow = ctrlPlane.removeFromTop(labelHeight);
    lbThreshold.setBounds(sliderRow.removeFromLeft(sliderWidth));
    sliderRow.removeFromLeft(sliderSpacing);
    lbKnee.setBounds(sliderRow.removeFromLeft(sliderWidth));
    sliderRow.removeFromLeft(sliderSpacing);
    lbOutGain.setBounds(sliderRow.removeFromLeft(sliderWidth));


    ctrlPlane.removeFromTop(10);

    sliderRow = ctrlPlane.removeFromTop(sliderHeight);

    sliderRatio.setBounds(sliderRow.removeFromLeft(sliderWidth));
    sliderRow.removeFromLeft(sliderSpacing);
    sliderAttackTime.setBounds(sliderRow.removeFromLeft(sliderWidth));
    sliderRow.removeFromLeft(sliderSpacing);
    sliderReleaseTime.setBounds(sliderRow.removeFromLeft(sliderWidth));

    sliderRow = ctrlPlane.removeFromTop(labelHeight);
    lbRatio.setBounds(sliderRow.removeFromLeft(sliderWidth));
    sliderRow.removeFromLeft(sliderSpacing);
    lbAttack.setBounds(sliderRow.removeFromLeft(sliderWidth));
    sliderRow.removeFromLeft(sliderSpacing);
    lbRelease.setBounds(sliderRow.removeFromLeft(sliderWidth));

    area.removeFromBottom(10);
    tbLookAhead.setBounds(area.removeFromBottom(20).removeFromLeft(130));
    area.removeFromBottom(10);
    characteristic.setBounds(area);


}
