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
#include "../../resources/Quaternion.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/customComponents/IEMSphere.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/MuteSoloButton.h"
#include "../../resources/customComponents/SpherePanner.h"
#include "EncoderList.h"



typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class MultiEncoderAudioProcessorEditor  : public AudioProcessorEditor,
private Timer,
public IEMSphere::IEMSphereListener,
public IEMSphere::IEMSphereElement
{
public:

    MultiEncoderAudioProcessorEditor (MultiEncoderAudioProcessor&, AudioProcessorValueTreeState&);
    ~MultiEncoderAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void IEMSphereElementChanged (IEMSphere* sphere, IEMSphereElement* element) override;

    
private:
    TitleBar<AudioChannelsIOWidget<maxNumberOfInputs>, AmbisonicIOWidget> title;
    Footer footer;
    LaF globalLaF;
    void timerCallback() override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MultiEncoderAudioProcessor& processor;

    GroupComponent quatGroup,ypGroup,settingsGroup;
    ReverseSlider slMasterYaw, slMasterPitch, slMasterRoll;
    
    ToggleButton tbLockedToMaster;
    ComboBox inputChooser;

    //IEMSphere sphere;
    //IEMSphereElement grabElement;
    
    SpherePanner sphere;
    //SpherePanner::Element sphereElements[maxNumberOfInputs];
    SpherePanner::Element masterElement;
    
    AudioProcessorValueTreeState& valueTreeState;
    
    ScopedPointer<SliderAttachment> slMasterYawAttachment;
    ScopedPointer<SliderAttachment> slMasterPitchAttachment;
    ScopedPointer<SliderAttachment> slMasterRollAttachment;
    ScopedPointer<ButtonAttachment> tbLockedToMasterAttachment;
    
    ScopedPointer<ComboBoxAttachment> cbNumInputChannelsAttachment, cbNormalizationAtachment;
    ScopedPointer<ComboBoxAttachment> cbOrderAtachment;

    Viewport viewport;
    EncoderList encoderList;
    
    
    TooltipWindow toolTipWin;

    int maxPossibleOrder = -1;
    int maxNumInputs = -1;
    int lastSetNumChIn = -1;
    
    // labels
    SimpleLabel lbNum, lbYaw, lbPitch, lbGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiEncoderAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
