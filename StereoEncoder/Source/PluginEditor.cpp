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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//==============================================================================
StereoEncoderAudioProcessorEditor::StereoEncoderAudioProcessorEditor (StereoEncoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), processor (p), valueTreeState(vts)//, sphere_opengl(nullptr)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 325);
    setLookAndFeel (&globalLaF);
    
    // ==== SPHERE AND ELEMENTS ===============
    addAndMakeVisible(&sphere);
    sphere.addListener(this);
    
    leftElement.setBackgroundColour(Colours::aqua);
    sphere.addElement(&leftElement);
    leftElement.setLabel("L");
    
    rightElement.setBackgroundColour(Colours::red);
    sphere.addElement(&rightElement);
    rightElement.setLabel("R");
    
    centerElement.setBackgroundColour(Colours::white);
    sphere.addElement(&centerElement);
    centerElement.setLabel("C");
    centerElement.setGrabPriority(1);
    

    centerElement.setPosition(processor.posC);
    leftElement.setPosition(processor.posL);
    rightElement.setPosition(processor.posR);


    
    // ======================================
    
    
    addAndMakeVisible(&title);
    title.setTitle(String("Stereo"),String("Encoder"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);
    
    addAndMakeVisible(&footer);
    
    toolTipWin.setMillisecondsBeforeTipAppears(500);
    

    cbNormalizationAtachement = new ComboBoxAttachment(valueTreeState,"useSN3D", *title.getOutputWidgetPtr()->getNormCbPointer());
    cbOrderAtachement = new ComboBoxAttachment(valueTreeState,"orderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer());
    
    
    // ======================== YAW PITCH ROLL GROUP
    ypGroup.setText("Yaw, Pitch, Roll");
    ypGroup.setTextLabelPosition (Justification::centredLeft);
    ypGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    ypGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible(&ypGroup);
    ypGroup.setVisible(true);
    
    addAndMakeVisible(&yawSlider);
    yawAttachment = new SliderAttachment(valueTreeState,"yaw", yawSlider);
    yawSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    yawSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    yawSlider.setReverse(true);
    yawSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    yawSlider.setRotaryParameters(M_PI, 3*M_PI, false);
    yawSlider.setTooltip("Yaw angle");
    yawSlider.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    
    addAndMakeVisible(&pitchSlider);
    pitchAttachment = new SliderAttachment(valueTreeState,"pitch", pitchSlider);
    pitchSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    pitchSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    pitchSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    pitchSlider.setReverse(true);
    pitchSlider.setRotaryParameters(0.5*M_PI, 2.5*M_PI, false);
    pitchSlider.setTooltip("Pitch angle");
    pitchSlider.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    
    addAndMakeVisible(&rollSlider);
    rollAttachment = new SliderAttachment(valueTreeState,"roll", rollSlider);
    rollSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    rollSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    rollSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    rollSlider.setReverse(false);
    rollSlider.setRotaryParameters(M_PI, 3*M_PI, false);
    rollSlider.setTooltip("Roll angle");
    rollSlider.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    
    
    
    // ====================== QUATERNION GROUP
    quatGroup.setText("Quaternions");
    quatGroup.setTextLabelPosition (Justification::centredLeft);
    quatGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    quatGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible(&quatGroup);
    quatGroup.setVisible(true);
    
    addAndMakeVisible(&qwSlider);
    qwAttachment = new SliderAttachment(valueTreeState,"qw", qwSlider);
    qwSlider.setSliderStyle (Slider::LinearHorizontal);
    qwSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    qwSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    
    addAndMakeVisible(&qxSlider);
    qxAttachment = new SliderAttachment(valueTreeState,"qx", qxSlider);
    qxSlider.setSliderStyle (Slider::LinearHorizontal);
    qxSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    qxSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    
    addAndMakeVisible(&qySlider);
    qyAttachment = new SliderAttachment(valueTreeState,"qy", qySlider);
    qySlider.setSliderStyle (Slider::LinearHorizontal);
    qySlider.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    qySlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    
    addAndMakeVisible(&qzSlider);
    qzAttachment = new SliderAttachment(valueTreeState,"qz", qzSlider);
    qzSlider.setSliderStyle (Slider::LinearHorizontal);
    qzSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    qzSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    
    
    // =========================== SETTINGS GROUP
    addAndMakeVisible(&settingsGroup);
    settingsGroup.setText("Settings");
    settingsGroup.setTextLabelPosition (Justification::centredLeft);
    settingsGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    settingsGroup.setColour (GroupComponent::textColourId, Colours::white);
    settingsGroup.setVisible(true);
    
    addAndMakeVisible(&widthSlider);
    widthAttachment = new SliderAttachment(valueTreeState,"width", widthSlider);
    widthSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    widthSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    widthSlider.setReverse(false);
    widthSlider.setRotaryParameters(M_PI, 3*M_PI, false);
    widthSlider.setTooltip("Stereo Width");
    //widthSlider.setEnabled(*processor.inputMode >= 0.5f);
    
    
    // ================ LABELS ===================
    addAndMakeVisible(&lbYaw);
    lbYaw.setText("Yaw");
    
    addAndMakeVisible(&lbPitch);
    lbPitch.setText("Pitch");
    
    addAndMakeVisible(&lbRoll);
    lbRoll.setText("Roll");
    
    addAndMakeVisible(&lblWidth);
    lblWidth.setText("Width");
    
    addAndMakeVisible(&lbW);
    lbW.setText("W");
    
    addAndMakeVisible(&lbX);
    lbX.setText("X");
    
    addAndMakeVisible(&lbY);
    lbY.setText("Y");
    
    addAndMakeVisible(&lbZ);
    lbZ.setText("Z");
    
    startTimer(10);
}


void StereoEncoderAudioProcessorEditor::IEMSphereElementChanged (IEMSphere* sphere, IEMSphereElement* element) {
    
    Vector3D<float> pos = element->getPosition();
    float hypxy = sqrt(pos.x*pos.x+pos.y*pos.y);
    
    float yaw = atan2f(pos.y,pos.x);
    float pitch = atan2f(hypxy,pos.z)-M_PI/2;
    //DBG("yaw: " << yaw/M_PI*180 << " pitch: " << pitch/M_PI*180);
    
    if (element->getID() == "center") {
        valueTreeState.getParameter("yaw")->setValue(valueTreeState.getParameterRange("yaw").convertTo0to1(yaw/M_PI*180.0f));
        valueTreeState.getParameter("pitch")->setValue(valueTreeState.getParameterRange("pitch").convertTo0to1(pitch/M_PI*180.0f));
    }
    else if (element->getID() == "left" || element->getID() == "right")
    {
        Vector3D<float> dPos = pos - processor.posC;
        
        float alpha = 4.0f*asinf(dPos.length()/2.0f);
        
        iem::Quaternion<float> quat;
        float ypr[3];
        ypr[0] = *processor.yaw/180.0*M_PI;
        ypr[1] = *processor.pitch/180.0*M_PI;
        ypr[2] = 0.0f;
        
        quat.fromYPR(ypr);
        quat.conjugate();
        
        float xyz[3];
        xyz[0] = pos.x;
        xyz[1] = pos.y;
        xyz[2] = pos.z;
        
        quat.rotateVector(xyz, xyz);
        
        
        float roll = atan2f(xyz[2], xyz[1]);
        if (element->getID() == "right") roll = atan2f(-xyz[2], -xyz[1]);
        
        valueTreeState.getParameter("width")->setValue(valueTreeState.getParameterRange("width").convertTo0to1(alpha/M_PI*180.0f));
        valueTreeState.getParameter("roll")->setValue(valueTreeState.getParameterRange("roll").convertTo0to1(roll/M_PI*180.0f));
        
    }
}

void StereoEncoderAudioProcessorEditor::IEMSphereMouseWheelMoved(IEMSphere* sphere, const juce::MouseEvent &event, const MouseWheelDetails &wheel)
{
    if (event.mods.isCommandDown() && event.mods.isAltDown())
        rollSlider.mouseWheelMove(event, wheel);
    else if (event.mods.isShiftDown())
        widthSlider.mouseWheelMove(event, wheel);
    else if (event.mods.isAltDown())
        pitchSlider.mouseWheelMove(event, wheel);
    else if (event.mods.isCommandDown())
        yawSlider.mouseWheelMove(event, wheel);
}

StereoEncoderAudioProcessorEditor::~StereoEncoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void StereoEncoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void StereoEncoderAudioProcessorEditor::timerCallback()
{
    // check max possible order and update combobox in title
    if (processor.maxPossibleOrder != maxPossibleOrder)
    {
        maxPossibleOrder = processor.maxPossibleOrder;
        title.getOutputWidgetPtr()->updateOrderCb(maxPossibleOrder);
    }
    
    // update positions, if at least one of them was updated (new data): repaint the sphere
    if (processor.updatedPositionData.get())
    {
        processor.updatedPositionData = false;
        
        bool cChgd = centerElement.setPosition(processor.posC);
        bool lChgd = leftElement.setPosition(processor.posL);
        bool rChgd = rightElement.setPosition(processor.posR);
        
        if (cChgd || lChgd || rChgd)
        {
           sphere.repaint();
        }
    }
}

void StereoEncoderAudioProcessorEditor::resized()
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
    const int sliderHeight = 15;
    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    const int sliderSpacing = 3;
    const int rotSliderWidth = 40;
    const int labelHeight = 15;
    const int labelWidth = 20;

    // -------------- Yaw Pitch Roll ------------------
    Rectangle<int> yprArea (sideBarArea.removeFromTop(25+rotSliderHeight+labelHeight));
    ypGroup.setBounds (yprArea);
    yprArea.removeFromTop(25); //for box headline
    
    sliderRow = (yprArea.removeFromTop(rotSliderHeight));
    yawSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    pitchSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    rollSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    widthSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    
    
    lbYaw.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    yprArea.removeFromLeft(rotSliderSpacing);
    lbPitch.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    yprArea.removeFromLeft(rotSliderSpacing);
    lbRoll.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    yprArea.removeFromLeft(rotSliderSpacing);
    lblWidth.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    
    sideBarArea.removeFromTop(20);
    
    // ------------- Quaternion ------------------------
    Rectangle<int> quatArea (sideBarArea.removeFromTop(165));
    quatGroup.setBounds (quatArea);
    quatArea.removeFromTop(25); //for box headline
    
    sliderRow = quatArea.removeFromTop(sliderHeight);
    qwSlider.setBounds (sliderRow.removeFromRight(185-labelWidth));
    lbW.setBounds(sliderRow);
    quatArea.removeFromTop(sliderSpacing);
    
    sliderRow = quatArea.removeFromTop(sliderHeight);
    qxSlider.setBounds (sliderRow.removeFromRight(185-labelWidth));
    lbX.setBounds(sliderRow);
    quatArea.removeFromTop(sliderSpacing);
    
    sliderRow = quatArea.removeFromTop(sliderHeight);
    qySlider.setBounds (sliderRow.removeFromRight(185-labelWidth));
    lbY.setBounds(sliderRow);
    quatArea.removeFromTop(sliderSpacing);
    
    sliderRow = quatArea.removeFromTop(sliderHeight);
    qzSlider.setBounds (sliderRow.removeFromRight(185-labelWidth));
    lbZ.setBounds(sliderRow);
    quatArea.removeFromTop(sliderSpacing);
    

    // ============== SIDEBAR LEFT ====================

    area.removeFromRight(10); // spacing
    sphere.setBounds(area.getX(), area.getY(),area.getWidth()-20,area.getWidth()-20);
    
    
}

