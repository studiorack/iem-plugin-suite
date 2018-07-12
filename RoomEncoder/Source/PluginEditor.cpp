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
RoomEncoderAudioProcessorEditor::RoomEncoderAudioProcessorEditor (RoomEncoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p),
processor (p), valueTreeState(vts), footer (p.getOSCReceiver()),
sourceElement(*valueTreeState.getParameter("sourceX"), valueTreeState.getParameterRange("sourceX"),
              *valueTreeState.getParameter("sourceY"), valueTreeState.getParameterRange("sourceY"),
              *valueTreeState.getParameter("sourceZ"), valueTreeState.getParameterRange("sourceZ")),
listenerElement(*valueTreeState.getParameter("listenerX"), valueTreeState.getParameterRange("listenerX"),
              *valueTreeState.getParameter("listenerY"), valueTreeState.getParameterRange("listenerY"),
              *valueTreeState.getParameter("listenerZ"), valueTreeState.getParameterRange("listenerZ"))

{

    setSize (800, 600);
    setLookAndFeel (&globalLaF);
    toolTipWin.setMillisecondsBeforeTipAppears(500);
    toolTipWin.setOpaque (false);

    addAndMakeVisible(&title);
    title.setTitle(String("Room"),String("Encoder"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);
    cbNormalizationAttachement = new ComboBoxAttachment(valueTreeState,"useSN3D", *title.getOutputWidgetPtr()->getNormCbPointer());
    cbOrderAttachement = new ComboBoxAttachment(valueTreeState,"orderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer());
    cbDirectivityOrderAttachment = new ComboBoxAttachment(valueTreeState,"directivityOrderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());
    cbDirectivityNormalizationAttachment = new ComboBoxAttachment(valueTreeState, "inputIsSN3D", *title.getInputWidgetPtr()->getNormCbPointer());
    addAndMakeVisible (&footer);


    addAndMakeVisible(&gcRoomDimensions);
    gcRoomDimensions.setText("Room Dimensions");
    gcRoomDimensions.setTextLabelPosition (Justification::left);
    gcRoomDimensions.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcRoomDimensions.setColour (GroupComponent::textColourId, Colours::white);

    addAndMakeVisible(&gcListenerPosition);
    gcListenerPosition.setText("Listener Position");
    gcListenerPosition.setTextLabelPosition (Justification::left);
    gcListenerPosition.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcListenerPosition.setColour (GroupComponent::textColourId, Colours::white);

    addAndMakeVisible(&gcSourcePosition);
    gcSourcePosition.setText("Source Position");
    gcSourcePosition.setTextLabelPosition (Justification::left);
    gcSourcePosition.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcSourcePosition.setColour (GroupComponent::textColourId, Colours::white);

    addAndMakeVisible(&gcReflectionProperties);
    gcReflectionProperties.setText("Reflection Properties");
    gcReflectionProperties.setTextLabelPosition (Justification::centredLeft);
    gcReflectionProperties.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcReflectionProperties.setColour (GroupComponent::textColourId, Colours::white);

    addAndMakeVisible(&gcSync);
    gcSync.setText("Synchronize Room Settings");
    gcSync.setTextLabelPosition (Justification::left);
    gcSync.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcSync.setColour (GroupComponent::textColourId, Colours::white);


    addAndMakeVisible(&lbRoomX);
    lbRoomX.setText("Depth");
    addAndMakeVisible(&lbRoomY);
    lbRoomY.setText("Width");
    addAndMakeVisible(&lbRoomZ);
    lbRoomZ.setText("Height");

    addAndMakeVisible(&lbListenerX);
    lbListenerX.setText("x");
    addAndMakeVisible(&lbListenerY);
    lbListenerY.setText("y");
    addAndMakeVisible(&lbListenerZ);
    lbListenerZ.setText("z");

    addAndMakeVisible(&lbSourceX);
    lbSourceX.setText("x");
    addAndMakeVisible(&lbSourceY);
    lbSourceY.setText("y");
    addAndMakeVisible(&lbSourceZ);
    lbSourceZ.setText("z");

    addAndMakeVisible(&slRoomX);
    slRoomXAttachment = new SliderAttachment(valueTreeState,"roomX", slRoomX);
    slRoomX.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slRoomX.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slRoomX.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slRoomX.setTextValueSuffix(" m");
    slRoomX.setTooltip("room size x");
    slRoomX.addListener(this);

    addAndMakeVisible(&slRoomY);
    slRoomYAttachment = new SliderAttachment(valueTreeState, "roomY", slRoomY);
    slRoomY.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slRoomY.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slRoomY.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slRoomY.setTextValueSuffix(" m");
    slRoomY.setTooltip("room size y");
    slRoomY.addListener(this);

    addAndMakeVisible(&slRoomZ);
    slRoomZAttachment = new SliderAttachment(valueTreeState, "roomZ", slRoomZ);
    slRoomZ.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slRoomZ.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slRoomZ.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slRoomZ.setTextValueSuffix(" m");
    slRoomZ.setTooltip("room size z");
    slRoomZ.addListener(this);

    addAndMakeVisible(&xyPlane);
    xyPlane.addElement(&sourceElement);
    xyPlane.addElement(&listenerElement);
    xyPlane.useAutoScale(false);

    addAndMakeVisible(&zyPlane);
    zyPlane.setPlane(PositionPlane::Planes::zy);
    zyPlane.addElement(&sourceElement);
    zyPlane.addElement(&listenerElement);
    zyPlane.useAutoScale(false);

    addAndMakeVisible(&slSourceX);
    slSourceXAttachment = new SliderAttachment(valueTreeState, "sourceX", slSourceX);
    slSourceX.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slSourceX.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slSourceX.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slSourceX.setTextValueSuffix(" m");
    slSourceX.setTooltip("source position x");
    slSourceX.addListener(this);

    addAndMakeVisible(&slSourceY);
    slSourceYAttachment = new SliderAttachment(valueTreeState, "sourceY", slSourceY);
    slSourceY.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slSourceY.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slSourceY.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slSourceY.setTextValueSuffix(" m");
    slSourceY.setTooltip("source position y");
    slSourceY.addListener(this);

    addAndMakeVisible(&slSourceZ);
    slSourceZAttachment = new SliderAttachment(valueTreeState, "sourceZ", slSourceZ);
    slSourceZ.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slSourceZ.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slSourceZ.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slSourceZ.setTextValueSuffix(" m");
    slSourceZ.setTooltip("source position z");
    slSourceZ.addListener(this);

    addAndMakeVisible(&slListenerX);
    slListenerXAttachment = new SliderAttachment(valueTreeState, "listenerX", slListenerX);
    slListenerX.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slListenerX.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slListenerX.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slListenerX.setRotaryParameters(M_PI, 3*M_PI, false);
    slListenerX.setTextValueSuffix(" m");
    slListenerX.setTooltip("listener position x");
    slListenerX.addListener(this);

    addAndMakeVisible(&slListenerY);
    slListenerYAttachment = new SliderAttachment(valueTreeState,"listenerY", slListenerY);
    slListenerY.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slListenerY.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slListenerY.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slListenerY.setTextValueSuffix(" m");
    slListenerY.setTooltip("listener position y");
    slListenerY.addListener(this);

    addAndMakeVisible(&slListenerZ);
    slListenerZAttachment = new SliderAttachment(valueTreeState,"listenerZ", slListenerZ);
    slListenerZ.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slListenerZ.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slListenerZ.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slListenerZ.setTextValueSuffix(" m");
    slListenerZ.setTooltip("listener position z");
    slListenerZ.addListener(this);

    sourceElement.setColour(globalLaF.ClWidgetColours[2]);
    listenerElement.setColour(globalLaF.ClWidgetColours[1]);

    addAndMakeVisible(&lbNumReflections);
    lbNumReflections.setText("Number of Reflections");
    addAndMakeVisible(&slNumReflections);
    slNumReflectionsAttachment = new SliderAttachment(valueTreeState,"numRefl", slNumReflections);
    slNumReflections.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slNumReflections.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slNumReflections.setColour (Slider::rotarySliderOutlineColourId, Colours::lightgrey);


    addAndMakeVisible(&lbReflCoeff);
    lbReflCoeff.setText("Reflection Attenuation");

    addAndMakeVisible(&slReflCoeff);
    slReflCoeffAttachment = new SliderAttachment(valueTreeState,"reflCoeff", slReflCoeff);
    slReflCoeff.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slReflCoeff.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slReflCoeff.setColour (Slider::rotarySliderOutlineColourId, Colours::lightgrey);
    slReflCoeff.setTextValueSuffix(" dB");

    addAndMakeVisible(&lbLSF);
    lbLSF.setText("Freq.");
    addAndMakeVisible(&lbLSG);
    lbLSG.setText("Gain");
    addAndMakeVisible(&lbHSF);
    lbHSF.setText("Freq.");
    addAndMakeVisible(&lbHSG);
    lbHSG.setText("Gain");

    addAndMakeVisible(&slLowShelfFreq);
    slLowShelfFreqAttachment = new SliderAttachment(valueTreeState,"lowShelfFreq", slLowShelfFreq);
    slLowShelfFreq.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slLowShelfFreq.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slLowShelfFreq.setColour (Slider::rotarySliderOutlineColourId, Colours::cyan);
    slLowShelfFreq.setTooltip("low shelf freq");


    addAndMakeVisible(&slLowShelfGain);
    slLowShelfGainAttachment = new SliderAttachment(valueTreeState,"lowShelfGain", slLowShelfGain);
    slLowShelfGain.setTextValueSuffix (" dB");
    slLowShelfGain.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slLowShelfGain.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slLowShelfGain.setColour (Slider::rotarySliderOutlineColourId, Colours::cyan);


    addAndMakeVisible(&slHighShelfFreq);
    slHighShelfFreqAttachment = new SliderAttachment(valueTreeState,"highShelfFreq", slHighShelfFreq);
    slHighShelfFreq.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slHighShelfFreq.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slHighShelfFreq.setColour (Slider::rotarySliderOutlineColourId, Colours::orangered);
    slHighShelfFreq.setTooltip("high shelf freq");

    addAndMakeVisible(&slHighShelfGain);
    slHighShelfGainAttachment = new SliderAttachment(valueTreeState,"highShelfGain", slHighShelfGain);
    slHighShelfGain.setTextValueSuffix (" dB");
    slHighShelfGain.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slHighShelfGain.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slHighShelfGain.setColour (Slider::rotarySliderOutlineColourId, Colours::orangered);

    addAndMakeVisible(&fv);
    fv.addCoefficients(&processor.lowShelfCoefficients, Colours::cyan, &slLowShelfFreq, &slLowShelfGain);
    fv.addCoefficients(&processor.highShelfCoefficients, Colours::orangered, &slHighShelfFreq, &slHighShelfGain);

    addAndMakeVisible(&rv);
    rv.setDataPointers(p.allGains, p.mRadius, p.numRefl);

    Vector3D<float> dims(slRoomX.getValue(), slRoomY.getValue(), slRoomZ.getValue());
    float scale = jmin(xyPlane.setDimensions(dims), zyPlane.setDimensions(dims));
    xyPlane.setScale(scale);
    zyPlane.setScale(scale);

    addAndMakeVisible(&cbSyncChannel);
    cbSyncChannel.setJustificationType(Justification::centred);
    cbSyncChannel.addItem("none", 1);
    cbSyncChannel.addItem("1", 2);
    cbSyncChannel.addItem("2", 3);
    cbSyncChannel.addItem("3", 4);
    cbSyncChannel.addItem("4", 5);
    cbSyncChannelAttachment = new ComboBoxAttachment(valueTreeState, "syncChannel", cbSyncChannel);

    addAndMakeVisible(&lbSyncChannel);
    lbSyncChannel.setText("Synchronize to Channel");

    addAndMakeVisible(&tbSyncRoomSize);
    tbSyncRoomSizeAttachment = new ButtonAttachment(valueTreeState, "syncRoomSize", tbSyncRoomSize);
    tbSyncRoomSize.setButtonText("Room Dimensions");
    tbSyncRoomSize.setColour (ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible(&tbSyncListener);
    tbSyncListenerAttachment = new ButtonAttachment(valueTreeState, "syncListener", tbSyncListener);
    tbSyncListener.setButtonText("Listener Position");
    tbSyncListener.setColour (ToggleButton::tickColourId, globalLaF.ClWidgetColours[1]);

    addAndMakeVisible(&tbSyncReflection);
    tbSyncReflectionAttachment = new ButtonAttachment(valueTreeState, "syncReflection", tbSyncReflection);
    tbSyncReflection.setButtonText("Reflection Properties");
    tbSyncReflection.setColour (ToggleButton::tickColourId, Colours::orangered);


    startTimer(20);
}

RoomEncoderAudioProcessorEditor::~RoomEncoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void RoomEncoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void RoomEncoderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;

    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    const int sliderSpacing = 3;
    const int rotSliderWidth = 40;
    const int labelHeight = 15;



    Rectangle<int> area (getLocalBounds());

    {
        Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
        footer.setBounds(footerArea);
    }

    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop    (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);

    {
        Rectangle<int> bottomStrip(area.removeFromBottom(120));
        rv.setBounds(bottomStrip.removeFromRight(540));
        bottomStrip.removeFromRight(20);
        gcSync.setBounds(bottomStrip);
        bottomStrip.removeFromTop(25);
        Rectangle<int> channelRow(bottomStrip.removeFromTop(20));
        lbSyncChannel.setBounds(channelRow.removeFromLeft(130));
        cbSyncChannel.setBounds(channelRow.removeFromLeft(50));
        bottomStrip.removeFromLeft(5);
        tbSyncRoomSize.setBounds(bottomStrip.removeFromTop(20));
        tbSyncListener.setBounds(bottomStrip.removeFromTop(20));
        tbSyncReflection.setBounds(bottomStrip.removeFromTop(20));
    }

    area.removeFromBottom(10);



    Rectangle<int> propArea (area.removeFromRight(9*rotSliderWidth+8*rotSliderSpacing));
    {
        Rectangle<int> coordinateArea (propArea.removeFromTop(100));

        Rectangle<int> roomArea (coordinateArea.removeFromLeft(3*rotSliderWidth+2*rotSliderSpacing));
        gcRoomDimensions.setBounds (roomArea);
        roomArea.removeFromTop(25);

        Rectangle<int> sliderRow = (roomArea.removeFromTop(rotSliderHeight));
        slRoomX.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        slRoomY.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        slRoomZ.setBounds (sliderRow.removeFromLeft(rotSliderWidth));

        lbRoomX.setBounds (roomArea.removeFromLeft(rotSliderWidth));
        roomArea.removeFromLeft(rotSliderSpacing);
        lbRoomY.setBounds (roomArea.removeFromLeft(rotSliderWidth));
        roomArea.removeFromLeft(rotSliderSpacing);
        lbRoomZ.setBounds (roomArea.removeFromLeft(rotSliderWidth));

        coordinateArea.removeFromLeft(rotSliderSpacing);
        Rectangle<int> sourceArea (coordinateArea.removeFromLeft(3*rotSliderWidth+2*rotSliderSpacing));
        gcSourcePosition.setBounds (sourceArea);
        sourceArea.removeFromTop(25);

        sliderRow = (sourceArea.removeFromTop(rotSliderHeight));
        slSourceX.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        slSourceY.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        slSourceZ.setBounds (sliderRow.removeFromLeft(rotSliderWidth));

        lbSourceX.setBounds (sourceArea.removeFromLeft(rotSliderWidth));
        sourceArea.removeFromLeft(rotSliderSpacing);
        lbSourceY.setBounds (sourceArea.removeFromLeft(rotSliderWidth));
        sourceArea.removeFromLeft(rotSliderSpacing);
        lbSourceZ.setBounds (sourceArea.removeFromLeft(rotSliderWidth));

        coordinateArea.removeFromLeft(rotSliderSpacing);
        Rectangle<int> listenerArea (coordinateArea.removeFromLeft(3*rotSliderWidth+2*rotSliderSpacing));
        gcListenerPosition.setBounds (listenerArea);
        listenerArea.removeFromTop(25);

        sliderRow = (listenerArea.removeFromTop(rotSliderHeight));
        slListenerX.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        slListenerY.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        slListenerZ.setBounds (sliderRow.removeFromLeft(rotSliderWidth));

        lbListenerX.setBounds (listenerArea.removeFromLeft(rotSliderWidth));
        listenerArea.removeFromLeft(rotSliderSpacing);
        lbListenerY.setBounds (listenerArea.removeFromLeft(rotSliderWidth));
        listenerArea.removeFromLeft(rotSliderSpacing);
        lbListenerZ.setBounds (listenerArea.removeFromLeft(rotSliderWidth));
    }

    propArea.removeFromTop(20);
    gcReflectionProperties.setBounds(propArea);
    propArea.removeFromTop(20);
    Rectangle<int> fvCol (propArea.removeFromLeft(330));

    { // 120
        Rectangle<int> fvRow (fvCol.removeFromTop(120));
        fv.setBounds(fvRow);

        fvCol.removeFromTop(10);

        Rectangle<int> sliderRow (fvCol.removeFromTop(rotSliderHeight));
        sliderRow.removeFromLeft(20);
        slLowShelfFreq.setBounds(sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(sliderSpacing);
        slLowShelfGain.setBounds(sliderRow.removeFromLeft(rotSliderWidth));

        sliderRow.removeFromRight(10);
        slHighShelfGain.setBounds(sliderRow.removeFromRight(rotSliderWidth));
        sliderRow.removeFromRight(sliderSpacing);
        slHighShelfFreq.setBounds(sliderRow.removeFromRight(rotSliderWidth));

        sliderRow = fvCol.removeFromTop(labelHeight);
        sliderRow.removeFromLeft(20);
        lbLSF.setBounds(sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(sliderSpacing);
        lbLSG.setBounds(sliderRow.removeFromLeft(rotSliderWidth));

        sliderRow.removeFromRight(10);
        lbHSG.setBounds(sliderRow.removeFromRight(rotSliderWidth));
        sliderRow.removeFromRight(sliderSpacing);
        lbHSF.setBounds(sliderRow.removeFromRight(rotSliderWidth));
    }

    propArea.removeFromTop(20);
    slReflCoeff.setBounds(propArea.removeFromTop(rotSliderHeight+10));
    lbReflCoeff.setBounds(propArea.removeFromTop(labelHeight));
    propArea.removeFromTop(20);
    slNumReflections.setBounds(propArea.removeFromTop(rotSliderHeight+10));
    lbNumReflections.setBounds(propArea.removeFromTop(labelHeight));


    area.removeFromRight(10);

    {
        Rectangle<int> planeArea (area.removeFromLeft(300));
        int height = planeArea.getHeight()/2;

        xyPlane.setBounds(planeArea.removeFromTop(height));
        zyPlane.setBounds(planeArea.removeFromTop(height));
    }
}

void RoomEncoderAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    if (slider == &slRoomX || slider == &slRoomY || slider == &slRoomZ)
    {
        Vector3D<float> dims(slRoomX.getValue(), slRoomY.getValue(), slRoomZ.getValue());
        float scale = jmin(xyPlane.setDimensions(dims), zyPlane.setDimensions(dims));
        xyPlane.setScale(scale);
        zyPlane.setScale(scale);
    }
}

void RoomEncoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    // ==========================================

    if (processor.updateFv)
    {
        fv.setOverallGainInDecibels(*valueTreeState.getRawParameterValue("reflCoeff"));
        processor.updateFv = false;
        fv.repaint();
    }

    if (processor.repaintPositionPlanes.get())
    {
        processor.repaintPositionPlanes = false;
        xyPlane.repaint();
        zyPlane.repaint();
    }
}
