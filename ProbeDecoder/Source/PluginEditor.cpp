/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//==============================================================================
ProbeDecoderAudioProcessorEditor::ProbeDecoderAudioProcessorEditor (ProbeDecoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), processor (p), valueTreeState(vts)//, sphere_opengl(nullptr)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 325);
    setLookAndFeel (&globalLaF);
    
    // ==== SPHERE AND ELEMENTS ===============
    addAndMakeVisible(&sphere);
    //sphere.addListener(this);
    
    probe.setColour(Colours::aqua);
    probe.setSliders(&slYaw, &slPitch);
    sphere.addElement(&probe);


    // ======================================
    
    
    addAndMakeVisible(&title);
    title.setTitle(String("Probe"),String("Decoder"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);
    
    addAndMakeVisible(&footer);
    
    toolTipWin.setMillisecondsBeforeTipAppears(500);
    

    cbNormalizationAtachement = new ComboBoxAttachment(valueTreeState,"useSN3D", *title.getInputWidgetPtr()->getNormCbPointer());
    cbOrderAtachement = new ComboBoxAttachment(valueTreeState,"orderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());
    
    
    // ======================== YAW PITCH ROLL GROUP
    ypGroup.setText("Yaw & Pitch");
    ypGroup.setTextLabelPosition (Justification::centredLeft);
    ypGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    ypGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible(&ypGroup);
    ypGroup.setVisible(true);
    
    addAndMakeVisible(&slYaw);
    slYawAttachment = new SliderAttachment(valueTreeState,"yaw", slYaw);
    slYaw.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slYaw.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slYaw.setReverse(true);
    slYaw.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slYaw.setRotaryParameters(M_PI, 3*M_PI, false);
    slYaw.setTooltip("Yaw angle");
    slYaw.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    
    addAndMakeVisible(&slPitch);
    slPitchAttachment = new SliderAttachment(valueTreeState,"pitch", slPitch);
    slPitch.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slPitch.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slPitch.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slPitch.setReverse(true);
    slPitch.setRotaryParameters(0.5*M_PI, 2.5*M_PI, false);
    slPitch.setTooltip("Pitch angle");
    slPitch.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    

    
    // ================ LABELS ===================
    addAndMakeVisible(&lbYaw);
    lbYaw.setText("Yaw");
    
    addAndMakeVisible(&lbPitch);
    lbPitch.setText("Pitch");

    startTimer(10);
}


//void ProbeDecoderAudioProcessorEditor::IEMSphereMouseWheelMoved(IEMSphere* sphere, const juce::MouseEvent &event, const MouseWheelDetails &wheel)
//{
//    if (event.mods.isCommandDown() && event.mods.isAltDown())
//        rollSlider.mouseWheelMove(event, wheel);
//    else if (event.mods.isShiftDown())
//        widthSlider.mouseWheelMove(event, wheel);
//    else if (event.mods.isAltDown())
//        pitchSlider.mouseWheelMove(event, wheel);
//    else if (event.mods.isCommandDown())
//        yawSlider.mouseWheelMove(event, wheel);
//}

ProbeDecoderAudioProcessorEditor::~ProbeDecoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void ProbeDecoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void ProbeDecoderAudioProcessorEditor::timerCallback()
{
    // check max possible order and update combobox in title
    if (processor.maxPossibleOrder != maxPossibleOrder)
    {
        maxPossibleOrder = processor.maxPossibleOrder;
        title.getInputWidgetPtr()->updateOrderCb(maxPossibleOrder);
    }
}

void ProbeDecoderAudioProcessorEditor::resized()
{
    
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds(footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop    (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    
    Rectangle<int> sliderRow;
    
    // ============== SIDEBAR RIGHT ====================
    // =================================================
    Rectangle<int> sideBarArea (area.removeFromRight(190));

    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;

    const int rotSliderWidth = 40;
    const int labelHeight = 15;
    

    // -------------- Yaw Pitch Roll ------------------
    Rectangle<int> yprArea (sideBarArea.removeFromTop(25+rotSliderHeight+labelHeight));
    ypGroup.setBounds (yprArea);
    yprArea.removeFromTop(25); //for box headline
    
    sliderRow = (yprArea.removeFromTop(rotSliderHeight));
    slYaw.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    slPitch.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
   
    
    
    lbYaw.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    yprArea.removeFromLeft(rotSliderSpacing);
    lbPitch.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    yprArea.removeFromLeft(rotSliderSpacing);
    lbRoll.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    yprArea.removeFromLeft(rotSliderSpacing);
    lblWidth.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    
    sideBarArea.removeFromTop(20);


    // ============== SIDEBAR LEFT ====================

    area.removeFromRight(10); // spacing
    sphere.setBounds(area.getX(), area.getY(),area.getWidth()-20,area.getWidth()-20);
    
    
}

