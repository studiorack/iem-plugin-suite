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
StereoEncoderAudioProcessorEditor::StereoEncoderAudioProcessorEditor (StereoEncoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), footer (p.getOSCParameterInterface()), processor (p), valueTreeState(vts),
    centerElement(*valueTreeState.getParameter("azimuth"), valueTreeState.getParameterRange("azimuth"), *valueTreeState.getParameter("elevation"), valueTreeState.getParameterRange("elevation")),
    leftElement(centerElement, *valueTreeState.getParameter("roll"), valueTreeState.getParameterRange("roll"), *valueTreeState.getParameter("width"), valueTreeState.getParameterRange("width")),
    rightElement(centerElement, *valueTreeState.getParameter("roll"), valueTreeState.getParameterRange("roll"), *valueTreeState.getParameter("width"), valueTreeState.getParameterRange("width"))
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 325);
    setLookAndFeel (&globalLaF);

    // ==== SPHERE AND ELEMENTS ===============
    addAndMakeVisible(&sphere);
    sphere.addListener(this);

    leftElement.setColour(Colours::aqua);
    sphere.addElement(&leftElement);
    leftElement.setLabel("L");

    rightElement.setColour(Colours::red);
    rightElement.setMirrored(true);
    sphere.addElement(&rightElement);
    rightElement.setLabel("R");

    centerElement.setColour(Colours::white);
    sphere.addElement(&centerElement);
    centerElement.setGrabPriority(1);
    // ======================================

    addAndMakeVisible(&title);
    title.setTitle(String("Stereo"),String("Encoder"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);

    addAndMakeVisible(&footer);

    toolTipWin.setLookAndFeel (&globalLaF);
    toolTipWin.setMillisecondsBeforeTipAppears (500);
    toolTipWin.setOpaque (false);


    cbNormalizationAtachement.reset (new ComboBoxAttachment (valueTreeState,"useSN3D", *title.getOutputWidgetPtr()->getNormCbPointer()));
    cbOrderAtachement.reset (new ComboBoxAttachment (valueTreeState,"orderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer()));


    // ======================== AZIMUTH ELEVATION ROLL WIDTH GROUP
    ypGroup.setText("Azimuth, Elevation, Roll, Width");
    ypGroup.setTextLabelPosition (Justification::centredLeft);
    ypGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    ypGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible(&ypGroup);
    ypGroup.setVisible(true);

    addAndMakeVisible(&azimuthSlider);
    azimuthAttachment.reset (new SliderAttachment (valueTreeState,"azimuth", azimuthSlider));
    azimuthSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    azimuthSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    azimuthSlider.setReverse(true);
    azimuthSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    azimuthSlider.setRotaryParameters(MathConstants<float>::pi, 3*MathConstants<float>::pi, false);
    azimuthSlider.setTooltip("Azimuth angle");
    azimuthSlider.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible(&elevationSlider);
    elevationAttachment.reset (new SliderAttachment (valueTreeState,"elevation", elevationSlider));
    elevationSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    elevationSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    elevationSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    elevationSlider.setRotaryParameters(0.5 * MathConstants<float>::pi, 2.5 * MathConstants<float>::pi, false);
    elevationSlider.setTooltip("Elevation angle");
    elevationSlider.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible(&rollSlider);
    rollAttachment.reset (new SliderAttachment (valueTreeState,"roll", rollSlider));
    rollSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    rollSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    rollSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    rollSlider.setReverse(false);
    rollSlider.setRotaryParameters(MathConstants<float>::pi, 3*MathConstants<float>::pi, false);
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
    qwAttachment.reset (new SliderAttachment (valueTreeState,"qw", qwSlider));
    qwSlider.setSliderStyle (Slider::LinearHorizontal);
    qwSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    qwSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible(&qxSlider);
    qxAttachment.reset (new SliderAttachment (valueTreeState,"qx", qxSlider));
    qxSlider.setSliderStyle (Slider::LinearHorizontal);
    qxSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    qxSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible(&qySlider);
    qyAttachment.reset (new SliderAttachment (valueTreeState,"qy", qySlider));
    qySlider.setSliderStyle (Slider::LinearHorizontal);
    qySlider.setTextBoxStyle (Slider::TextBoxLeft, false, 50, 15);
    qySlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible(&qzSlider);
    qzAttachment.reset (new SliderAttachment (valueTreeState,"qz", qzSlider));
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
    widthAttachment.reset (new SliderAttachment (valueTreeState,"width", widthSlider));
    widthSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    widthSlider.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    widthSlider.setReverse(false);
    widthSlider.setRotaryParameters(MathConstants<float>::pi, 3*MathConstants<float>::pi, false);
    widthSlider.setTooltip("Stereo Width");
    //widthSlider.setEnabled(*processor.inputMode >= 0.5f);


    // ================ LABELS ===================
    addAndMakeVisible(&lbAzimuth);
    lbAzimuth.setText("Azimuth");

    addAndMakeVisible(&lbElevation);
    lbElevation.setText("Elevation");

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


    // KeyListener
    addKeyListener (this);

    startTimer(20);
}


void StereoEncoderAudioProcessorEditor::mouseWheelOnSpherePannerMoved (SpherePanner* sphere, const MouseEvent &event, const MouseWheelDetails &wheel)
{
    if (event.mods.isCommandDown() && event.mods.isAltDown())
        rollSlider.mouseWheelMove(event, wheel);
    else if (event.mods.isShiftDown())
        widthSlider.mouseWheelMove(event, wheel);
    else if (event.mods.isAltDown())
        elevationSlider.mouseWheelMove(event, wheel);
    else if (event.mods.isCommandDown())
        azimuthSlider.mouseWheelMove(event, wheel);
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
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    if (processor.updatedPositionData.get())
    {
        processor.updatedPositionData = false;
        sphere.repaint();
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

    // -------------- Azimuth Elevation Roll Width ------------------
    Rectangle<int> yprArea (sideBarArea.removeFromTop(25+rotSliderHeight+labelHeight));
    ypGroup.setBounds (yprArea);
    yprArea.removeFromTop(25); //for box headline

    sliderRow = (yprArea.removeFromTop(rotSliderHeight));
    azimuthSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    elevationSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    rollSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    widthSlider.setBounds (sliderRow.removeFromLeft(rotSliderWidth));


    lbAzimuth.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    yprArea.removeFromLeft(rotSliderSpacing - 5);
    lbElevation.setBounds(yprArea.removeFromLeft(rotSliderWidth + 10));
    yprArea.removeFromLeft(rotSliderSpacing - 5);
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

bool StereoEncoderAudioProcessorEditor::keyPressed (const KeyPress &key, Component *originatingComponent)
{
    DBG("Key pressed: " << key.getKeyCode());

    if (key.getModifiers().isShiftDown())
    {
        switch (key.getKeyCode())
        {
            case 90: // zenith
            case 84: // top
            case 85: // up
                azimuthSlider.setValue (0.0);
                elevationSlider.setValue (90.0);
                break;

            case 68: // down
            case 78: // nadir
                azimuthSlider.setValue (0.0);
                elevationSlider.setValue (-90.0);
                break;

            case 70: // front
                azimuthSlider.setValue (0.0);
                elevationSlider.setValue (0.0);
                break;

            case 66: // back
                azimuthSlider.setValue (-180.0);
                elevationSlider.setValue (0.0);
                break;

            case 76: // left
                azimuthSlider.setValue (90.0);
                elevationSlider.setValue (0.0);
                break;

            case 82: // right
                azimuthSlider.setValue (-90.0);
                elevationSlider.setValue (0.0);
                break;

            default:
                return false;
        }
        return true;
    }

    return false;
}
