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
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/customComponents/PositionPlane.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/FilterVisualizer.h"
#include "ReflectionsVisualizer.h"

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class RoomEncoderAudioProcessorEditor  : public AudioProcessorEditor, private Timer,
private Slider::Listener, private Button::Listener
{
public:
    RoomEncoderAudioProcessorEditor (RoomEncoderAudioProcessor&, AudioProcessorValueTreeState&);
    ~RoomEncoderAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    //==============================================================================
    void sliderValueChanged(Slider *slider) override;

    //==============================================================================
    void buttonClicked (Button *button) override {};
    void buttonStateChanged (Button *button) override;

private:
    //==============================================================================
    LaF globalLaF;
    TitleBar<DirectivityIOWidget, AmbisonicIOWidget<>> title;
    OSCFooter footer;

    void timerCallback() override;

    RoomEncoderAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    SimpleLabel lbReflCoeff, lbNumReflections;
    TripleLabel lbRoomDim;

    FilterVisualizer<float> fv;
    ReflectionsVisualizer rv;

    ComboBox cbSyncChannel;
    SimpleLabel lbSyncChannel;
    ToggleButton tbSyncRoomSize, tbSyncReflection, tbSyncListener;
    ToggleButton tbDirectPathUnityGain, tbDirectPathZeroDelay, tbRenderDirectPath;
    std::unique_ptr<ComboBoxAttachment> cbSyncChannelAttachment;
    std::unique_ptr<ButtonAttachment> tbSyncRoomSizeAttachment, tbSyncReflectionAttachment, tbSyncListenerAttachment;
    std::unique_ptr<ButtonAttachment> tbDirectPathUnityGainAttachment, tbDirectPathZeroDelayAttachment, tbRenderDirectPathAttachment;

    GroupComponent gcRoomDimensions, gcSourcePosition, gcListenerPosition;
    GroupComponent gcReflectionProperties;
    GroupComponent gcSync;

    SimpleLabel lbRoomX, lbRoomY, lbRoomZ;
    SimpleLabel lbListenerX, lbListenerY, lbListenerZ;
    SimpleLabel lbSourceX, lbSourceY, lbSourceZ;

    SimpleLabel lbLSF, lbLSG, lbHSF, lbHSG;

    SimpleLabel lbWallAttenuation;
    SimpleLabel lbWallAttenuationFront, lbWallAttenuationBack, lbWallAttenuationLeft, lbWallAttenuationRight, lbWallAttenuationCeiling, lbWallAttenuationFloor;


    ReverseSlider slSourceX, slSourceY, slSourceZ;
    ReverseSlider slListenerX, slListenerY, slListenerZ;
    ReverseSlider slRoomX, slRoomY, slRoomZ;

    ReverseSlider slReflCoeff, slLowShelfFreq, slLowShelfGain, slHighShelfFreq, slHighShelfGain;
    ReverseSlider slNumReflections;

    ReverseSlider slWallAttenuationFront, slWallAttenuationBack, slWallAttenuationLeft, slWallAttenuationRight, slWallAttenuationCeiling, slWallAttenuationFloor;


    std::unique_ptr<SliderAttachment> slSourceXAttachment, slSourceYAttachment, slSourceZAttachment;
    std::unique_ptr<SliderAttachment> slListenerXAttachment, slListenerYAttachment, slListenerZAttachment;
    std::unique_ptr<SliderAttachment> slRoomXAttachment, slRoomYAttachment, slRoomZAttachment;

    std::unique_ptr<SliderAttachment> slReflCoeffAttachment, slLowShelfFreqAttachment, slLowShelfGainAttachment, slHighShelfFreqAttachment, slHighShelfGainAttachment;
    std::unique_ptr<SliderAttachment> slNumReflectionsAttachment;

    std::unique_ptr<SliderAttachment> slWallAttenuationFrontAttachment, slWallAttenuationBackAttachment, slWallAttenuationLeftAttachment, slWallAttenuationRightAttachment, slWallAttenuationCeilingAttachment, slWallAttenuationFloorAttachment;


    std::unique_ptr<ComboBoxAttachment> cbNormalizationAttachement;
    std::unique_ptr<ComboBoxAttachment> cbOrderAttachement;
    std::unique_ptr<ComboBoxAttachment> cbDirectivityOrderAttachment;
    std::unique_ptr<ComboBoxAttachment> cbDirectivityNormalizationAttachment;

    PositionPlane xyPlane, zyPlane;
    PositionPlane::ParameterElement sourceElement, listenerElement;

    OpenGLContext mOpenGlContext;

    TooltipWindow toolTipWin;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomEncoderAudioProcessorEditor)
};
