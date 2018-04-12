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
MultiEncoderAudioProcessorEditor::MultiEncoderAudioProcessorEditor (MultiEncoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), processor (p), valueTreeState(vts), encoderList(p, sphere, &vts),
masterElement(*valueTreeState.getParameter("masterAzimuth"), valueTreeState.getParameterRange("masterAzimuth"), *valueTreeState.getParameter("masterElevation"), valueTreeState.getParameterRange("masterElevation"))
{
    setLookAndFeel (&globalLaF);

    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        valueTreeState.addParameterListener("azimuth"+String(i), this);
        valueTreeState.addParameterListener("elevation"+String(i), this);
    }
    valueTreeState.addParameterListener("masterAzimuth", this);
    valueTreeState.addParameterListener("masterElevation", this);

    // ==== SPHERE AND ELEMENTS ===============
    addAndMakeVisible(&sphere);
    sphere.addListener(this);

    sphere.addElement(&masterElement);
    masterElement.setColour(Colours::black);
    masterElement.setTextColour(Colours::white);
    masterElement.setLabel("M");

    // ======================================

    addAndMakeVisible(&title);
    title.setTitle(String("Multi"),String("Encoder"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);

    addAndMakeVisible(&footer);

    toolTipWin.setMillisecondsBeforeTipAppears(500);

    addAndMakeVisible(&viewport);
    viewport.setViewedComponent(&encoderList);

    cbNumInputChannelsAttachment = new ComboBoxAttachment(valueTreeState,"inputSetting",*title.getInputWidgetPtr()->getChannelsCbPointer());
    cbNormalizationAtachment = new ComboBoxAttachment(valueTreeState,"useSN3D",*title.getOutputWidgetPtr()->getNormCbPointer());
    cbOrderAtachment = new ComboBoxAttachment(valueTreeState,"orderSetting",*title.getOutputWidgetPtr()->getOrderCbPointer());

    // ======================== AZIMUTH ELEVATION ROLL GROUP
    ypGroup.setText("Encoder settings");
    ypGroup.setTextLabelPosition (Justification::centredLeft);
    ypGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    ypGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible(&ypGroup);
    ypGroup.setVisible(true);

    addAndMakeVisible(&slMasterAzimuth);
    slMasterAzimuthAttachment = new SliderAttachment(valueTreeState, "masterAzimuth", slMasterAzimuth);
    slMasterAzimuth.setSliderStyle (Slider::Rotary);
    slMasterAzimuth.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slMasterAzimuth.setReverse(true);
    slMasterAzimuth.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slMasterAzimuth.setRotaryParameters(M_PI, 3*M_PI, false);
    slMasterAzimuth.setTooltip("Master azimuth angle");

    addAndMakeVisible(&slMasterElevation);
    slMasterElevationAttachment = new SliderAttachment(valueTreeState, "masterElevation", slMasterElevation);
    slMasterElevation.setSliderStyle (Slider::Rotary);
    slMasterElevation.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slMasterElevation.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slMasterElevation.setRotaryParameters(0.5*M_PI, 2.5*M_PI, false);
    slMasterElevation.setTooltip("Master elevation angle");

    addAndMakeVisible(&slMasterRoll);
    slMasterRollAttachment = new SliderAttachment(valueTreeState, "masterRoll", slMasterRoll);
    slMasterRoll.setSliderStyle (Slider::Rotary);
    slMasterRoll.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slMasterRoll.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slMasterRoll.setRotaryParameters(M_PI, 3*M_PI, false);
    slMasterRoll.setTooltip("Master roll angle");

    addAndMakeVisible(&tbLockedToMaster);
    tbLockedToMasterAttachment = new ButtonAttachment(valueTreeState, "lockedToMaster", tbLockedToMaster);
    tbLockedToMaster.setName("locking");
    tbLockedToMaster.setButtonText("Lock Directions");

    // ====================== GRAB GROUP
    quatGroup.setText("Master");
    quatGroup.setTextLabelPosition (Justification::centredLeft);
    quatGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    quatGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible(&quatGroup);
    quatGroup.setVisible(true);


    // ================ LABELS ===================
    addAndMakeVisible(&lbNum);
    lbNum.setText("#");

    addAndMakeVisible(&lbAzimuth);
    lbAzimuth.setText("Azimuth");

    addAndMakeVisible(&lbElevation);
    lbElevation.setText("Elevation");

    addAndMakeVisible(&lbGain);
    lbGain.setText("Gain");

    addAndMakeVisible(&lbMasterAzimuth);
    lbMasterAzimuth.setText("Azimuth");

    addAndMakeVisible(&lbMasterElevation);
    lbMasterElevation.setText("Elevation");

    addAndMakeVisible(&lbMasterRoll);
    lbMasterRoll.setText("Roll");

    setResizeLimits(590, 455, 800, 1200);
    startTimer(40);
}


MultiEncoderAudioProcessorEditor::~MultiEncoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        valueTreeState.removeParameterListener("azimuth"+String(i), this);
        valueTreeState.removeParameterListener("elevation"+String(i), this);
    }
    valueTreeState.removeParameterListener("masterAzimuth", this);
    valueTreeState.removeParameterListener("masterElevation", this);
}

//==============================================================================
void MultiEncoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void MultiEncoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    // ==========================================



    const int nChIn = processor.input.getSize();
    if (nChIn != lastSetNumChIn)
    {
        encoderList.setNumberOfChannels(nChIn);
        lastSetNumChIn = nChIn;
        sphere.repaint();
    }

    if (processor.soloMuteChanged)
    {
        if (! processor.soloMask.isZero()) {
            for (int i = 0; i<lastSetNumChIn; ++i)
            {
                encoderList.sphereElementArray[i]->setActive(processor.soloMask[i]);
            }
        }
        else
        {
            for (int i = 0; i<lastSetNumChIn; ++i)
            {
                encoderList.sphereElementArray[i]->setActive(!processor.muteMask[i]);
            }
        }
        processor.soloMuteChanged = false;
        sphere.repaint();
    }

    if (processor.updateColours)
    {
        processor.updateColours = false;
        encoderList.updateColours();
        sphere.repaint();
    }
}

void MultiEncoderAudioProcessorEditor::mouseWheelOnSpherePannerMoved (SpherePanner* sphere, const MouseEvent &event, const MouseWheelDetails &wheel)
{
    if (event.mods.isCommandDown() && event.mods.isAltDown())
        slMasterRoll.mouseWheelMove(event, wheel);
    else if (event.mods.isAltDown())
        slMasterElevation.mouseWheelMove(event, wheel);
    else if (event.mods.isCommandDown())
        slMasterAzimuth.mouseWheelMove(event, wheel);
}
void MultiEncoderAudioProcessorEditor::parameterChanged (const String &parameterID, float newValue)
{
    sphere.repaint();
}


void MultiEncoderAudioProcessorEditor::resized()
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
    area.removeFromBottom(5);

    Rectangle<int> sliderRow;

    // ============== SIDEBAR RIGHT ====================
    // =================================================
    Rectangle<int> sideBarArea (area.removeFromRight(220));
    //const int sliderHeight = 15;
    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    //const int sliderSpacing = 3;
    const int rotSliderWidth = 40;
    //const int labelHeight = 15;
    //const int labelWidth = 20;


    // -------------- Azimuth Elevation Roll Labels ------------------
    Rectangle<int> yprArea (sideBarArea);
    ypGroup.setBounds (yprArea);
    yprArea.removeFromTop(25); //for box headline


    sliderRow = (yprArea.removeFromTop(15));
    lbNum.setBounds(sliderRow.removeFromLeft(22));
    sliderRow.removeFromLeft(5);
    lbAzimuth.setBounds(sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing - 5);
    lbElevation.setBounds(sliderRow.removeFromLeft(rotSliderWidth + 10));
    sliderRow.removeFromLeft(rotSliderSpacing - 5);
    lbGain.setBounds(sliderRow.removeFromLeft(rotSliderWidth));

    viewport.setBounds(yprArea);


    // ============== SIDEBAR LEFT ====================

    const int masterAreaHeight = 90;
    area.removeFromRight(10); // spacing

    Rectangle<int> sphereArea (area);
    sphereArea.removeFromBottom(masterAreaHeight);

    if ((float)sphereArea.getWidth()/sphereArea.getHeight() > 1)
        sphereArea.setWidth(sphereArea.getHeight());
    else
        sphereArea.setHeight(sphereArea.getWidth());
    sphere.setBounds(sphereArea);

    area.removeFromTop(sphereArea.getHeight());

    // ------------- Master Grabber ------------------------
    Rectangle<int> masterArea (area.removeFromTop(masterAreaHeight));
    quatGroup.setBounds (masterArea);
    masterArea.removeFromTop(25); //for box headline


    sliderRow = (masterArea.removeFromBottom(12));
    lbMasterAzimuth.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing - 5);
    lbMasterElevation.setBounds (sliderRow.removeFromLeft(rotSliderWidth + 10));
    sliderRow.removeFromLeft(rotSliderSpacing - 5);
    lbMasterRoll.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);

    sliderRow = masterArea;
    slMasterAzimuth.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    slMasterElevation.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    slMasterRoll.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    tbLockedToMaster.setBounds (sliderRow.removeFromLeft(100));
}
