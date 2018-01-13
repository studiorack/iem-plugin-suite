/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Sebastian Grill
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 http://www.iem.at
 
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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FdnReverbAudioProcessorEditor::FdnReverbAudioProcessorEditor (FdnReverbAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState (vts),
    tv (20.f, 20000.f, 0.1f, 25.f, 5.f),
    fv (20.f, 20000.f, -80.f, 5.f, 5.f, false)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setResizeLimits(600, 480, 1000, 950);
    setLookAndFeel (&globalLaF);

    //networkOrder.addListener (this);
    freezeMode.addListener (this);
    
    addAndMakeVisible (&title);
    title.setTitle (String("FDN"), String("Reverb"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);

    addAndMakeVisible(&footer);
    
    addAndMakeVisible(&delayGroup);
    delayGroup.setText("General Settings");
    delayGroup.setTextLabelPosition (Justification::centredLeft);
    delayGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    delayGroup.setColour (GroupComponent::textColourId, Colours::white);
    delayGroup.setVisible(true);

    addAndMakeVisible(&filterGroup);
    filterGroup.setText("Filter Settings");
//    highsGroup.setTextLabelPosition (Justification::centredLeft);
//    highsGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
//    highsGroup.setColour (GroupComponent::textColourId, Colours::blue);
//    highsGroup.setVisible(true);


    addAndMakeVisible(&t60Group);
    t60Group.setText("Reverberation Time");
    t60Group.setTextLabelPosition (Justification::centredLeft);
    t60Group.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    t60Group.setColour (GroupComponent::textColourId, Colours::red);
    t60Group.setVisible(true);

    addAndMakeVisible (&delayLengthSlider);
    delayAttachment = new SliderAttachment (valueTreeState, "delayLength", delayLengthSlider);
    delayLengthSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    delayLengthSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    delayLengthSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    delayLengthSlider.setTooltip("Room Size");

    addAndMakeVisible (&revTimeSlider);
    feedbackAttachment = new SliderAttachment (valueTreeState, "revTime", revTimeSlider);
    revTimeSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    revTimeSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    revTimeSlider.setColour (Slider::rotarySliderOutlineColourId, Colours::white);
    revTimeSlider.setTooltip("Reverberation Time");
    //revTimeSlider.setSkewFactorFromMidPoint(10.f);
    revTimeSlider.addListener(this);

    addAndMakeVisible (&dryWetSlider);
    dryWetAttachment = new SliderAttachment (valueTreeState, "dryWet", dryWetSlider);
    dryWetSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    dryWetSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    dryWetSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    dryWetSlider.setTooltip("Dry/Wet");

    addAndMakeVisible (&lowCutoffSlider);
    lowCutoffAttachment = new SliderAttachment (valueTreeState, "lowCutoff", lowCutoffSlider);
    lowCutoffSlider.setSkewFactorFromMidPoint (2000.f);
    lowCutoffSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    lowCutoffSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    lowCutoffSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    lowCutoffSlider.setTooltip("Low Shelf Cutoff Freq");
    lowCutoffSlider.addListener(this);

    addAndMakeVisible (&lowQSlider);
    lowQAttachment = new SliderAttachment (valueTreeState, "lowQ", lowQSlider);
    lowQSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    lowQSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    lowQSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    lowQSlider.setTooltip("Low Shelf Q");
    lowQSlider.addListener(this);

    addAndMakeVisible (&lowGainSlider);
    lowGainAttachment = new SliderAttachment (valueTreeState, "lowGain", lowGainSlider);
    lowGainSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    lowGainSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    lowGainSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    lowGainSlider.setTooltip("Low Shelf Gain");
    lowGainSlider.addListener(this);

    addAndMakeVisible (&highCutoffSlider);
    highCutoffAttachment = new SliderAttachment (valueTreeState, "highCutoff", highCutoffSlider);
    highCutoffSlider.setSkewFactorFromMidPoint (2000.f);
    highCutoffSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    highCutoffSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    highCutoffSlider.setColour (Slider::rotarySliderOutlineColourId,
        globalLaF.ClWidgetColours[0]);;
    highCutoffSlider.setTooltip("High Shelf Cutoff Freq");
    highCutoffSlider.addListener(this);

    addAndMakeVisible (&highQSlider);
    highQAttachment = new SliderAttachment (valueTreeState, "highQ", highQSlider);
    highQSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    highQSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    highQSlider.setColour (Slider::rotarySliderOutlineColourId,
        globalLaF.ClWidgetColours[0]);
    highQSlider.setTooltip("High Shelf Q");
    highQSlider.addListener(this);

    addAndMakeVisible (&highGainSlider);
    highGainAttachment = new SliderAttachment (valueTreeState, "highGain", highGainSlider);
    highGainSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    highGainSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
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

    //startTimer(50);

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
                                                           lowCutoffSlider.getValue(), lowQSlider.getValue(), Decibels::decibelsToGain(lowGainSlider.getValue()));
    highpassCoeffs = IIR::Coefficients<float>::makeHighShelf(48000,
                                                             highCutoffSlider.getValue(), highQSlider.getValue(), Decibels::decibelsToGain(highGainSlider.getValue()));

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
    fv.setOverallGain (gain);
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
                 highCutoffSlider.getValue(), highQSlider.getValue(), Decibels::decibelsToGain(highGainSlider.getValue()));

        tv.repaint();
        fv.repaint();
    }
    else if (slider == &lowCutoffSlider ||
             slider == &lowQSlider ||
             slider == &lowGainSlider)
    {   
        *lowpassCoeffs = *IIR::Coefficients<float>::makeLowShelf(48000,
            lowCutoffSlider.getValue(), lowQSlider.getValue(), Decibels::decibelsToGain(lowGainSlider.getValue()));

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
    //const int sliderHeight = 15;
    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    //const int sliderSpacing = 3;
    const int rotSliderWidth = 40;
    const int labelHeight = 20;
    //const int labelWidth = 20;
    
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds(footerArea);

    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    area.removeFromBottom(5);
    
    
    { //====================== DELAY SETTINGS GROUP ==================================
        const int rotSliderWidth = 55;
        
        Rectangle<int> settingsArea (area.removeFromRight(185));
        delayGroup.setBounds (settingsArea);
        settingsArea.removeFromTop (25); //for box headline
        
        Rectangle<int> sliderRow (settingsArea.removeFromTop(rotSliderHeight));
        
        delayLengthSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        revTimeSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        dryWetSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        //sliderRow.removeFromLeft(3);
        //networkOrder.setBounds (sliderRow.removeFromLeft(70));
        //sliderRow.removeFromLeft(rotSliderSpacing);
        
        sliderRow = settingsArea.removeFromTop(labelHeight);

        lbDelay.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        lbTime.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        lbDryWet.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        //delayArea.removeFromLeft(3);
        // freezeMode.setBounds (delayArea.removeFromLeft(70));
    }
    
    area.removeFromRight(10); //spacing

    const int height = (area.getHeight() - 10 - labelHeight - rotSliderHeight + 10) / 2;
    { //====================== T60 GROUP ==================================
        Rectangle<int> t60Area (area.removeFromTop(height));
        t60Group.setBounds (t60Area);
        t60Area.removeFromTop (25);
        tv.setBounds(t60Area);
    }
    
    area.removeFromTop(10); //spacing
    
    
    { //====================== FILTER GROUP ==================================
        Rectangle<int> filterArea(area);
        filterGroup.setBounds(filterArea);
        filterArea.removeFromTop(25);
        
        Rectangle<int> sliderRow(filterArea.removeFromBottom(labelHeight));
        lbLowCutoff.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lbLowQ.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lbLowGain.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        
        
        lbHighGain.setBounds (sliderRow.removeFromRight(rotSliderWidth));
        sliderRow.removeFromRight (rotSliderSpacing);
        lbHighQ.setBounds (sliderRow.removeFromRight(rotSliderWidth));
        sliderRow.removeFromRight (rotSliderSpacing);
        lbHighCutoff.setBounds (sliderRow.removeFromRight(rotSliderWidth));
        
        sliderRow = filterArea.removeFromBottom(rotSliderHeight-10);
        
        lowCutoffSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        lowQSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        lowGainSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        
        highGainSlider.setBounds (sliderRow.removeFromRight(rotSliderWidth));
        sliderRow.removeFromRight(rotSliderSpacing);
        highQSlider.setBounds (sliderRow.removeFromRight(rotSliderWidth));
        sliderRow.removeFromRight(rotSliderSpacing);
        highCutoffSlider.setBounds (sliderRow.removeFromRight(rotSliderWidth));
        
        fv.setBounds(filterArea);
    }
    
    
    

}
