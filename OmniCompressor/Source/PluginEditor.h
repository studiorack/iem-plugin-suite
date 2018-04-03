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
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/LevelMeter.h"

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class OmniCompressorAudioProcessorEditor  : public AudioProcessorEditor,
private Timer
{
public:
    OmniCompressorAudioProcessorEditor (OmniCompressorAudioProcessor&, AudioProcessorValueTreeState&);
    ~OmniCompressorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    LaF globalLaF;

    OmniCompressorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    TitleBar<AmbisonicIOWidget<>, NoIOWidget> title;
    Footer footer;
    
    
    void timerCallback() override;
    
    ReverseSlider sliderKnee, sliderThreshold, sliderRatio, sliderAttackTime, sliderReleaseTime, sliderMakeupGain;
    
    ScopedPointer<ComboBoxAttachment> cbNormalizationAtachement;
    ScopedPointer<ComboBoxAttachment> cbOrderAtachement;
    
    ScopedPointer<SliderAttachment> KnAttachment;
    ScopedPointer<SliderAttachment> ThAttachment;
    ScopedPointer<SliderAttachment> RaAttachment;
    ScopedPointer<SliderAttachment> ATAttachment;
    ScopedPointer<SliderAttachment> RTAttachment;
    ScopedPointer<SliderAttachment> MGAttachment;
    

    LevelMeter inpMeter, dbGRmeter;
    
    SimpleLabel lbKnee, lbThreshold, lbOutGain, lbRatio, lbAttack, lbRelease;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmniCompressorAudioProcessorEditor)
};
