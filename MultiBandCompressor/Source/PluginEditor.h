/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Markus Huber
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
#include "FilterBankVisualizer.h"
#include "MasterControl.h"
#include "../../resources/customComponents/RoundButton.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/CompressorVisualizer.h"
#include "../../resources/customComponents/FilterVisualizer.h"
#include "../../resources/customComponents/LevelMeter.h"



using SliderAttachment = ReverseSlider::SliderAttachment ; // all ReverseSliders will make use of the parameters' valueToText() function
using ComboBoxAttachment = AudioProcessorValueTreeState::ComboBoxAttachment;
using ButtonAttachment = AudioProcessorValueTreeState::ButtonAttachment;

//==============================================================================
/**
*/
class MultiBandCompressorAudioProcessorEditor  : public AudioProcessorEditor, private Timer, public Slider::Listener, public Button::Listener
{
public:
    MultiBandCompressorAudioProcessorEditor (MultiBandCompressorAudioProcessor&, AudioProcessorValueTreeState&);
    ~MultiBandCompressorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void sliderValueChanged (Slider *slider) override;
    void buttonClicked (Button* bypassButton) override;

    void timerCallback() override;

private:
    // ====================== begin essentials ==================
    // lookAndFeel class with the IEM plug-in suite design
    LaF globalLaF;

    // stored references to the AudioProcessor and ValueTreeState holding all the parameters
    MultiBandCompressorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    /* title and footer component
     title component can hold different widgets for in- and output:
        - NoIOWidget (if there's no need for an input or output widget)
        - AudioChannelsIOWidget<maxNumberOfChannels, isChoosable>
        - AmbisonicIOWidget<maxOrder>
        - DirectivitiyIOWidget
     */
    TitleBar<AmbisonicIOWidget<>, NoIOWidget> title;
    OSCFooter footer;
    // =============== end essentials ============

    static constexpr const int& numFreqBands = MultiBandCompressorAudioProcessor::numFreqBands;

    std::unique_ptr<ComboBoxAttachment> cbNormalizationAtachement;
    std::unique_ptr<ComboBoxAttachment> cbOrderAtachement;

    FilterBankVisualizer<double> filterBankVisualizer;
    TooltipWindow tooltips;

    // Filter Crossovers
    ReverseSlider slCrossover[numFreqBands-1];
    std::unique_ptr<SliderAttachment> slCrossoverAttachment[numFreqBands-1];

    // Solo and Bypass Buttons
    RoundButton tbSolo[numFreqBands];
    RoundButton tbBypass[numFreqBands];
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> soloAttachment[numFreqBands], bypassAttachment[numFreqBands];

    // Compressor Parameters
    ReverseSlider slKnee[numFreqBands], slThreshold[numFreqBands], slRatio[numFreqBands], slAttackTime[numFreqBands], slReleaseTime[numFreqBands], slMakeUpGain[numFreqBands];
    std::unique_ptr<SliderAttachment> slKneeAttachment[numFreqBands], slThresholdAttachment[numFreqBands], slRatioAttachment[numFreqBands], slAttackTimeAttachment[numFreqBands], slReleaseTimeAttachment[numFreqBands], slMakeUpGainAttachment[numFreqBands];

    // Master parameters
    GroupComponent gcMasterControls;
    MasterControl slMasterThreshold, slMasterMakeUpGain, slMasterKnee, slMasterRatio, slMasterAttackTime, slMasterReleaseTime;

    // Compressor Visualization
    OwnedArray<CompressorVisualizer> compressorVisualizers;

    // Meters
    LevelMeter GRmeter[numFreqBands], omniInputMeter, omniOutputMeter;

    // Toggle Buttons
    ToggleButton tbOverallMagnitude;
    bool displayOverallMagnitude {false};

    // Labels
    SimpleLabel lbKnee[numFreqBands+1], lbThreshold[numFreqBands+1], lbMakeUpGain[numFreqBands+1], lbRatio[numFreqBands+1], lbAttack[numFreqBands+1], lbRelease[numFreqBands+1], lbInput, lbOutput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiBandCompressorAudioProcessorEditor)
};
