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

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#define OutputAmbi

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/customComponents/IEMSphere.h"
#include "../../resources/customComponents/SimpleLabel.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class StereoEncoderAudioProcessorEditor  : public AudioProcessorEditor,
private Timer,
public IEMSphere::IEMSphereListener,
public IEMSphere::IEMSphereElement
{
public:

    StereoEncoderAudioProcessorEditor (StereoEncoderAudioProcessor&, AudioProcessorValueTreeState&);
    ~StereoEncoderAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void IEMSphereElementChanged (IEMSphere* sphere, IEMSphereElement* element) override;
    void IEMSphereMouseWheelMoved(IEMSphere* sphere, const juce::MouseEvent &event, const MouseWheelDetails &wheel) override;
private:
    TitleBar<AudioChannelsIOWidget<2,false>, AmbisonicIOWidget> title;
    Footer footer;
    LaF globalLaF;
    void timerCallback() override;
    

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StereoEncoderAudioProcessor& processor;

    GroupComponent quatGroup,ypGroup,settingsGroup;
    ReverseSlider yawSlider, pitchSlider, rollSlider, widthSlider, qwSlider, qxSlider, qySlider, qzSlider;
    ComboBox inputChooser;

    IEMSphere sphere;
    IEMSphereElement leftElement = IEMSphereElement("left");
    IEMSphereElement rightElement = IEMSphereElement("right");
    IEMSphereElement centerElement = IEMSphereElement("center");
    
    AudioProcessorValueTreeState& valueTreeState;

    ScopedPointer<SliderAttachment> qwAttachment;
    ScopedPointer<SliderAttachment> qxAttachment;
    ScopedPointer<SliderAttachment> qyAttachment;
    ScopedPointer<SliderAttachment> qzAttachment;
    ScopedPointer<SliderAttachment> yawAttachment;
    ScopedPointer<SliderAttachment> pitchAttachment;
    ScopedPointer<SliderAttachment> rollAttachment;
    ScopedPointer<SliderAttachment> widthAttachment;
    ScopedPointer<ComboBoxAttachment> cbNormalizationAtachement;
    ScopedPointer<ComboBoxAttachment> cbOrderAtachement;

    TooltipWindow toolTipWin;

    int maxPossibleOrder = -1;
    
    // labels
    SimpleLabel lbYaw, lbPitch, lbRoll, lblWidth, lbW, lbX, lbY, lbZ;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoEncoderAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
