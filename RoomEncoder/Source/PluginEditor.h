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
#include "../../resources/Quaternion.h"
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
class RoomEncoderAudioProcessorEditor  : public AudioProcessorEditor, private Timer, public PositionPlane::PositionPlaneListener,
                                        private Slider::Listener
{
public:
    RoomEncoderAudioProcessorEditor (RoomEncoderAudioProcessor&, AudioProcessorValueTreeState&);
    ~RoomEncoderAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void PositionPlaneElementChanged (PositionPlane* plane, PositionPlane::PositionPlaneElement* element) override;
    void sliderValueChanged(Slider *slider) override;
private:
    LaF globalLaF;
    TitleBar<DirectivityIOWidget, AmbisonicIOWidget<>> title;
    Footer footer;
    
    void timerCallback() override;
    
    
    AudioProcessorValueTreeState& valueTreeState;
    
    ReverseSlider slSourceX, slSourceY, slSourceZ;
    ReverseSlider slListenerX, slListenerY, slListenerZ;
    ReverseSlider slRoomX, slRoomY, slRoomZ;
    
    ReverseSlider slReflCoeff;
    
    ReverseSlider slLowShelfFreq, slLowShelfGain, slHighShelfFreq, slHighShelfGain;
    
    ReverseSlider slNumReflections;
    
    SimpleLabel lbReflCoeff, lbNumReflections;
    TripleLabel lbRoomDim;
    
    FilterVisualizer<float> fv;
    ReflectionsVisualizer rv;
    
    ComboBox cbSyncChannel;
    SimpleLabel lbSyncChannel;
    ToggleButton tbSyncRoomSize, tbSyncReflection, tbSyncListener;
    ScopedPointer<ComboBoxAttachment> cbSyncChannelAttachment;
    ScopedPointer<ButtonAttachment> tbSyncRoomSizeAttachment, tbSyncReflectionAttachment, tbSyncListenerAttachment;

    GroupComponent gcRoomDimensions, gcSourcePosition, gcListenerPosition;
    GroupComponent gcReflectionProperties;
    GroupComponent gcSync;
    
    SimpleLabel lbRoomX, lbRoomY, lbRoomZ;
    SimpleLabel lbListenerX, lbListenerY, lbListenerZ;
    SimpleLabel lbSourceX, lbSourceY, lbSourceZ;
    
    SimpleLabel lbLSF, lbLSG, lbHSF, lbHSG;

    
    ScopedPointer<SliderAttachment> slSourceXAttachment, slSourceYAttachment, slSourceZAttachment;
    ScopedPointer<SliderAttachment> slListenerXAttachment, slListenerYAttachment, slListenerZAttachment;
    ScopedPointer<SliderAttachment> slRoomXAttachment, slRoomYAttachment, slRoomZAttachment;
    
    ScopedPointer<SliderAttachment> slReflCoeffAttachment;
    ScopedPointer<SliderAttachment> slLowShelfFreqAttachment, slLowShelfGainAttachment, slHighShelfFreqAttachment, slHighShelfGainAttachment;
    ScopedPointer<SliderAttachment> slNumReflectionsAttachment;
    
    ScopedPointer<ComboBoxAttachment> cbNormalizationAtachement;
    ScopedPointer<ComboBoxAttachment> cbOrderAtachement;
    ScopedPointer<ComboBoxAttachment> cbDirectivityOrderSetting;
    
    PositionPlane xyPlane, zyPlane;
    PositionPlane::PositionPlaneElement sourceElement, listenerElement;

    OpenGLContext mOpenGlContext;
    
    TooltipWindow toolTipWin;

    RoomEncoderAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomEncoderAudioProcessorEditor)
};
