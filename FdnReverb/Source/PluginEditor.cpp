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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FdnReverbAudioProcessorEditor::FdnReverbAudioProcessorEditor (FdnReverbAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState (vts),
    tv (20.f, 20000.f, 0.1f, 60.f, 5.f),
    fv (20.f, 20000.f, -25.f, 5.f, 5.f, true)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize (500, 450);
    setLookAndFeel (&globalLaF);

    //networkOrder.addListener (this);
    freezeMode.addListener (this);
    
    addAndMakeVisible (&title);
    title.setTitle (String("FDN"), String("Reverb"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);

    addAndMakeVisible(&footer);
    
    addAndMakeVisible(&delayGroup);
    delayGroup.setText("Delay");
    delayGroup.setTextLabelPosition (Justification::centredLeft);
    delayGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    delayGroup.setColour (GroupComponent::textColourId, Colours::white);
    delayGroup.setVisible(true);

    addAndMakeVisible(&highsGroup);
    highsGroup.setText("High Shelf Filter");
    highsGroup.setTextLabelPosition (Justification::centredLeft);
    highsGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    highsGroup.setColour (GroupComponent::textColourId, Colours::blue);
    highsGroup.setVisible(true);

    addAndMakeVisible(&lowsGroup);
    lowsGroup.setText("Low Shelf Filter");
    lowsGroup.setTextLabelPosition (Justification::centredLeft);
    lowsGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    lowsGroup.setColour (GroupComponent::textColourId, Colours::red);
    lowsGroup.setVisible(true);

    addAndMakeVisible(&t60Group);
    t60Group.setText("Reverberation Time");
    t60Group.setTextLabelPosition (Justification::centredLeft);
    t60Group.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    t60Group.setColour (GroupComponent::textColourId, Colours::red);
    t60Group.setVisible(true);

    addAndMakeVisible(&gainGroup);
    gainGroup.setText("Filter Gain Magnitudes");
    gainGroup.setTextLabelPosition (Justification::centredLeft);
    gainGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gainGroup.setColour (GroupComponent::textColourId, Colours::red);
    gainGroup.setVisible(true);

    addAndMakeVisible (&delayLengthSlider);
    delayAttachment = new SliderAttachment (valueTreeState, "delayLength", delayLengthSlider);
    delayLengthSlider.setSliderStyle (Slider::Rotary);
    delayLengthSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
    delayLengthSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    delayLengthSlider.setTooltip("Room Size");

    addAndMakeVisible (&revTimeSlider);
    feedbackAttachment = new SliderAttachment (valueTreeState, "revTime", revTimeSlider);
    revTimeSlider.setSliderStyle (Slider::Rotary);
    revTimeSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
    revTimeSlider.setColour (Slider::rotarySliderOutlineColourId, Colours::white);
    revTimeSlider.setTooltip("Reverberation Time");
    revTimeSlider.setSkewFactorFromMidPoint(10.f);
    revTimeSlider.addListener(this);

    addAndMakeVisible (&dryWetSlider);
    dryWetAttachment = new SliderAttachment (valueTreeState, "dryWet", dryWetSlider);
    dryWetSlider.setSliderStyle (Slider::Rotary);
    dryWetSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
    dryWetSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    dryWetSlider.setTooltip("Dry/Wet");

    addAndMakeVisible (&lowCutoffSlider);
    lowCutoffAttachment = new SliderAttachment (valueTreeState, "lowCutoff", lowCutoffSlider);
    lowCutoffSlider.setSkewFactorFromMidPoint (2000.f);
    lowCutoffSlider.setSliderStyle (Slider::Rotary);
    lowCutoffSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
    lowCutoffSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    lowCutoffSlider.setTooltip("Low Shelf Cutoff Freq");
    lowCutoffSlider.addListener(this);

    addAndMakeVisible (&lowQSlider);
    lowQAttachment = new SliderAttachment (valueTreeState, "lowQ", lowQSlider);
    lowQSlider.setSliderStyle (Slider::Rotary);
    lowQSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
    lowQSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    lowQSlider.setTooltip("Low Shelf Q");
    lowQSlider.addListener(this);

    addAndMakeVisible (&lowGainSlider);
    lowGainAttachment = new SliderAttachment (valueTreeState, "lowGain", lowGainSlider);
    lowGainSlider.setSliderStyle (Slider::Rotary);
    lowGainSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
    lowGainSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    lowGainSlider.setTooltip("Low Shelf Gain");
    lowGainSlider.addListener(this);

    addAndMakeVisible (&highCutoffSlider);
    highCutoffAttachment = new SliderAttachment (valueTreeState, "highCutoff", highCutoffSlider);
    highCutoffSlider.setSkewFactorFromMidPoint (2000.f);
    highCutoffSlider.setSliderStyle (Slider::Rotary);
    highCutoffSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
    highCutoffSlider.setColour (Slider::rotarySliderOutlineColourId,
        globalLaF.ClWidgetColours[0]);;
    highCutoffSlider.setTooltip("High Shelf Cutoff Freq");
    highCutoffSlider.addListener(this);

    addAndMakeVisible (&highQSlider);
    highQAttachment = new SliderAttachment (valueTreeState, "highQ", highQSlider);
    highQSlider.setSliderStyle (Slider::Rotary);
    highQSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
    highQSlider.setColour (Slider::rotarySliderOutlineColourId,
        globalLaF.ClWidgetColours[0]);
    highQSlider.setTooltip("High Shelf Q");
    highQSlider.addListener(this);

    addAndMakeVisible (&highGainSlider);
    highGainAttachment = new SliderAttachment (valueTreeState, "highGain", highGainSlider);
    highGainSlider.setSliderStyle (Slider::Rotary);
    highGainSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
    highGainSlider.setColour (Slider::rotarySliderOutlineColourId,
        globalLaF.ClWidgetColours[0]);
    highGainSlider.setTooltip("High Shelf Gain");
    highGainSlider.addListener(this);

    //addAndMakeVisible (&networkOrder);
    //networkOrder.setButtonText ("big FDN");
    //networkOrderAttachment = new ButtonAttachment(valueTreeState, "fdnSize", networkOrder);
//    networkOrder.triggerClick();

    addAndMakeVisible (&freezeMode);
    freezeMode.setButtonText ("Freeze");

    startTimer(50);

    addAndMakeVisible(&lbDelay);
    lbDelay.setText("Room Size");
    addAndMakeVisible(&lbTime);
    lbTime.setText("Rev. Time");
    addAndMakeVisible(&lbDryWet);
    lbDryWet.setText("Dry/Wet");
    addAndMakeVisible(&lbHighCutoff);
    lbHighCutoff.setText("Freq.");
    addAndMakeVisible(&lbHighQ);
    lbHighQ.setText("Q");
    addAndMakeVisible(&lbHighGain);
    lbHighGain.setText("Gain");
    addAndMakeVisible(&lbLowCutoff);
    lbLowCutoff.setText("Freq.");
    addAndMakeVisible(&lbLowQ);
    lbLowQ.setText("Q");
    addAndMakeVisible(&lbLowGain);
    lbLowGain.setText("Gain");

    // left side
    addAndMakeVisible(&tv);
    
    lowpassCoeffs = IIR::Coefficients<float>::makeLowShelf(48000,
        lowCutoffSlider.getValue(), lowQSlider.getValue(), lowGainSlider.getValue());
    highpassCoeffs = IIR::Coefficients<float>::makeHighShelf(48000,
        highCutoffSlider.getValue(), highQSlider.getValue(), highGainSlider.getValue());

    tv.addCoefficients(lowpassCoeffs, Colours::orangered, &lowCutoffSlider,
        &lowGainSlider);
    tv.addCoefficients(highpassCoeffs, Colours::cyan, &highCutoffSlider,
        &highGainSlider);

	float gain = pow(10.0, -3.0 / revTimeSlider.getValue());
	tv.setOverallGain(gain);

    tv.repaint();

    addAndMakeVisible (&fv);
    fv.addCoefficients (&lowpassCoeffs, globalLaF.ClWidgetColours[3], &lowCutoffSlider,
        &lowGainSlider);
    fv.addCoefficients (&highpassCoeffs, globalLaF.ClWidgetColours[0], &highCutoffSlider,
        &highGainSlider);
    fv.repaint();
}

FdnReverbAudioProcessorEditor::~FdnReverbAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void FdnReverbAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (globalLaF.ClBackground);
}

void FdnReverbAudioProcessorEditor::timerCallback()
{
}

void FdnReverbAudioProcessorEditor::buttonClicked (Button* button)
{
//    if (button == &networkOrder)
//    {
//        if (networkOrder.getToggleState())
//            processor.setNetworkOrder(64);
//        else
//            processor.setNetworkOrder(32);
//    }

    if (button == &freezeMode)
    {
        if (freezeMode.getToggleState())
            processor.setFreezeMode (true);
        else
            processor.setFreezeMode (false);
    }
}

void FdnReverbAudioProcessorEditor::sliderValueChanged(Slider* slider) 
{   
    if (slider == &highCutoffSlider ||
        slider == &highQSlider ||
        slider == &highGainSlider)
    {   
        *highpassCoeffs = *IIR::Coefficients<float>::makeHighShelf(48000,
            highCutoffSlider.getValue(), highQSlider.getValue(), highGainSlider.getValue());

        tv.repaint();
        fv.repaint();
    }
    else if (slider == &lowCutoffSlider ||
             slider == &lowQSlider ||
             slider == &lowGainSlider)
    {   
        *lowpassCoeffs = *IIR::Coefficients<float>::makeLowShelf(48000,
            lowCutoffSlider.getValue(), lowQSlider.getValue(), lowGainSlider.getValue());

        tv.repaint();
        fv.repaint();
    }
    else if (slider == &revTimeSlider)
    {
        float gain = pow (10.0, -3.0 / revTimeSlider.getValue());
        fv.setOverallGain (gain);
        tv.setOverallGain (gain);

        fv.repaint();
        tv.repaint();
    }
}

void FdnReverbAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds(footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);

    Rectangle<int> sliderRow;
    Rectangle<int> highRow;
    Rectangle<int> lowRow;
    Rectangle<int> t60Row;
    Rectangle<int> gainRow;

    Rectangle<int> sideBarArea (area.removeFromRight(190));
    //const int sliderHeight = 15;
    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    //const int sliderSpacing = 3;
    const int rotSliderWidth = 55;
    const int labelHeight = 15;
    //const int labelWidth = 20;

    Rectangle<int> graphArea (area.removeFromLeft (230));

    Rectangle<int> t60Area (graphArea.removeFromTop (170));
    t60Group.setBounds (t60Area);
    t60Area.removeFromTop (25);

    Rectangle<int> gainArea (graphArea.removeFromTop (170));
    gainGroup.setBounds (gainArea);
    gainArea.removeFromTop (25);

    Rectangle<int> delayArea (sideBarArea.removeFromTop (50 + rotSliderHeight + labelHeight));
    delayGroup.setBounds (delayArea);
    delayArea.removeFromTop (25); //for box headline

    Rectangle<int> lowsArea (sideBarArea.removeFromTop (50 + rotSliderHeight + labelHeight));
    lowsGroup.setBounds (lowsArea);
    lowsArea.removeFromTop (25);

    Rectangle<int> highsArea (sideBarArea.removeFromTop (50 + rotSliderHeight + labelHeight));
    highsGroup.setBounds (highsArea);
    highsArea.removeFromTop (25);


//====================== LEFT SIDE VISUAL ELEMENTS =============================
    t60Row = t60Area.removeFromTop (170);
    tv.setBounds (t60Row);

    gainRow = gainArea.removeFromTop (170);
    fv.setBounds (gainRow);

//====================== DELAY SETTINGS GROUP ==================================
    sliderRow = delayArea.removeFromTop (rotSliderHeight);    delayLengthSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    revTimeSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    dryWetSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(3);
    //networkOrder.setBounds (sliderRow.removeFromLeft(70));
    sliderRow.removeFromLeft(rotSliderSpacing);

    lbDelay.setBounds (delayArea.removeFromLeft(rotSliderWidth));
    delayArea.removeFromLeft(rotSliderSpacing);
    lbTime.setBounds (delayArea.removeFromLeft(rotSliderWidth));
    delayArea.removeFromLeft(rotSliderSpacing);
    lbDryWet.setBounds (delayArea.removeFromLeft(rotSliderWidth));
    delayArea.removeFromLeft(3);
    // freezeMode.setBounds (delayArea.removeFromLeft(70));

//====================== LOW SHELF FILTER GROUP ================================
    lowRow = lowsArea.removeFromTop (rotSliderHeight);
    lowCutoffSlider.setBounds (lowRow.removeFromLeft (rotSliderWidth));
    lowRow.removeFromLeft(rotSliderSpacing);
    lowQSlider.setBounds (lowRow.removeFromLeft (rotSliderWidth));
    lowRow.removeFromLeft(rotSliderSpacing);
    lowGainSlider.setBounds (lowRow.removeFromLeft (rotSliderWidth));

    lbLowCutoff.setBounds (lowsArea.removeFromLeft (rotSliderWidth));
    lowsArea.removeFromLeft (rotSliderSpacing);
    lbLowQ.setBounds (lowsArea.removeFromLeft (rotSliderWidth));
    lowsArea.removeFromLeft (rotSliderSpacing);
    lbLowGain.setBounds (lowsArea.removeFromLeft (rotSliderWidth));

//====================== HIGH SHELF FILTER GROUP ===============================
    highRow = highsArea.removeFromTop (rotSliderHeight);
    highCutoffSlider.setBounds (highRow.removeFromLeft (rotSliderWidth));
    highRow.removeFromLeft(rotSliderSpacing);
    highQSlider.setBounds (highRow.removeFromLeft (rotSliderWidth));
    highRow.removeFromLeft(rotSliderSpacing);
    highGainSlider.setBounds (highRow.removeFromLeft (rotSliderWidth));

    lbHighCutoff.setBounds (highsArea.removeFromLeft (rotSliderWidth));
    highsArea.removeFromLeft (rotSliderSpacing);
    lbHighQ.setBounds (highsArea.removeFromLeft (rotSliderWidth));
    highsArea.removeFromLeft (rotSliderSpacing);
    lbHighGain.setBounds (highsArea.removeFromLeft (rotSliderWidth));

    sideBarArea.removeFromTop(20);
}
