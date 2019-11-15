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

//Plugin Design Essentials
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"

//Custom Components
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/HammerAitovGrid.h"
#include "VisualizerComponent.h"
#include "VisualizerColormap.h"


typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class EnergyVisualizerAudioProcessorEditor  : public AudioProcessorEditor, private Timer, Slider::Listener
{
public:
    EnergyVisualizerAudioProcessorEditor (EnergyVisualizerAudioProcessor&, AudioProcessorValueTreeState&);
    ~EnergyVisualizerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    LaF globalLaF;

    EnergyVisualizerAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    VisualizerComponent visualizer;
    VisualizerColormap colormap;

    void sliderValueChanged (Slider *slider) override;
    void timerCallback() override;

    TitleBar<AmbisonicIOWidget<>, NoIOWidget> title;
    OSCFooter footer;

    ReverseSlider slPeakLevel;
    ReverseSlider slDynamicRange;
    SimpleLabel lbPeakLevel;
    SimpleLabel lbDynamicRange;
    std::unique_ptr<SliderAttachment> slPeakLevelAttachment, slDynamicRangeAttachment;

    std::unique_ptr<ComboBoxAttachment> cbNormalizationAtachement;
    std::unique_ptr<ComboBoxAttachment> cbOrderAtachement;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnergyVisualizerAudioProcessorEditor)
};
