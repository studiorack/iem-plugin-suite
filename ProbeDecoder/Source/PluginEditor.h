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

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/customComponents/SpherePanner.h"
#include "../../resources/customComponents/SimpleLabel.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class ProbeDecoderAudioProcessorEditor  : public AudioProcessorEditor,
private Timer
{
public:

    ProbeDecoderAudioProcessorEditor (ProbeDecoderAudioProcessor&, AudioProcessorValueTreeState&);
    ~ProbeDecoderAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    TitleBar<AmbisonicIOWidget, AudioChannelsIOWidget<1,false>> title;
    Footer footer;
    LaF globalLaF;
    void timerCallback() override;
    

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ProbeDecoderAudioProcessor& processor;

    GroupComponent ypGroup, settingsGroup;
    ReverseSlider slYaw, slPitch;

    SpherePanner sphere;
    SpherePanner::Element probe;
    
    AudioProcessorValueTreeState& valueTreeState;


    ScopedPointer<SliderAttachment> slYawAttachment;
    ScopedPointer<SliderAttachment> slPitchAttachment;

    ScopedPointer<ComboBoxAttachment> cbNormalizationAtachement;
    ScopedPointer<ComboBoxAttachment> cbOrderAtachement;

    TooltipWindow toolTipWin;
    
    // labels
    SimpleLabel lbYaw, lbPitch, lbRoll, lblWidth, lbW, lbX, lbY, lbZ;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProbeDecoderAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
