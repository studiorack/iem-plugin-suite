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

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
MultiBandCompressorAudioProcessorEditor::MultiBandCompressorAudioProcessorEditor (MultiBandCompressorAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState (vts), footer (p.getOSCReceiver()), filterVisualizer(20.0f, 20000.0f, -15.0f, 20.0f, 5.0f)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    setResizeLimits (1000, 1000*0.56, 1600, 1600*0.56); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (String ("MultiBand"), String ("Compressor"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
  
    cbInputChannelsSettingAttachment = std::make_unique<ComboBoxAttachment>(valueTreeState, "inputChannelsSetting", *title.getInputWidgetPtr()->getChannelsCbPointer());
  
    tooltips.setMillisecondsBeforeTipAppears(500);
    tooltips.setOpaque (false);
  
    const Colour colours[numFilterBands] =
    {
        Colours::cornflowerblue,
        Colours::greenyellow,
        Colours::yellow,
        Colours::orangered
    };
  
    for (int i = 0; i < numFilterBands; ++i)
    {
    
        // ==== COMPRESSOR VISUALIZATION ====
        compressorVisualizers.add (new CompressorVisualizer (p.getCompressor (i)));
        addAndMakeVisible(compressorVisualizers[i]);
      
        // Solo + Bypass buttons
        tbSoloEnabled[i].setColour (TextButton::buttonColourId, Colours::darkgrey);
        tbSoloEnabled[i].setColour (TextButton::buttonOnColourId, Colours::red);
        tbSoloEnabled[i].setButtonText ("Solo");
        tbSoloEnabled[i].setToggleState (false, dontSendNotification);
        soloAttachment[i]  = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "soloEnabled" + String(i), tbSoloEnabled[i]);
        tbSoloEnabled[i].setClickingTogglesState (true);
        addAndMakeVisible (&tbSoloEnabled[i]);
      
        tbBypass[i].setColour (TextButton::buttonColourId, Colours::darkgrey);
        tbBypass[i].setColour (TextButton::buttonOnColourId, colours[i]);
        tbBypass[i].setToggleState (false, dontSendNotification);
        tbBypass[i].setButtonText ("Bypass");
        bypassAttachment[i] = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "bypass" + String(i), tbBypass[i]);
        tbBypass[i].setClickingTogglesState (true);
        addAndMakeVisible (&tbBypass[i]);
      
        // Sliders
        addAndMakeVisible(&slThreshold[i]);
        slThresholdAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "threshold" + String(i), slThreshold[i]);
        slThreshold[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slThreshold[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
        slThreshold[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slThreshold[i].setTextValueSuffix(" dB");
      
        addAndMakeVisible(&slKnee[i]);
        slKneeAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "knee" + String(i), slKnee[i]);
        slKnee[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slKnee[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
        slKnee[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slKnee[i].setTextValueSuffix(" dB");

        addAndMakeVisible(&slRatio[i]);
        slRatioAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "ratio" + String (i), slRatio[i]);
        slRatio[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slRatio[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
        slRatio[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);

        addAndMakeVisible(&slAttackTime[i]);
        slAttackTimeAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "attack" + String(i), slAttackTime[i]);
        slAttackTime[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slAttackTime[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
        slAttackTime[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slRatio[i].setTextValueSuffix(" ms");

        addAndMakeVisible(&slReleaseTime[i]);
        slReleaseTimeAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "release" + String(i), slReleaseTime[i]);
        slReleaseTime[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slReleaseTime[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
        slReleaseTime[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slReleaseTime[i].setTextValueSuffix(" ms");

        addAndMakeVisible(&slMakeUpGain[i]);
        slMakeUpGainAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "makeUpGain" + String(i), slMakeUpGain[i]);
        slMakeUpGain[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slMakeUpGain[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
        slMakeUpGain[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slMakeUpGain[i].setTextValueSuffix(" dB");
        slMakeUpGain[i].setName(String ("MakeUpGain" + String(i)));
        slMakeUpGain[i].addListener(this);

        addAndMakeVisible(&GRmeter[i]);
        GRmeter[i].setMinLevel(-25.0f);
        GRmeter[i].setColour(Colours::red.withMultipliedAlpha(0.8f));
        GRmeter[i].setGainReductionMeter(true);

        addAndMakeVisible(&omniFilteredMeter[i]);
        omniFilteredMeter[i].setMinLevel(-60.0f);
        omniFilteredMeter[i].setColour(Colours::green.withMultipliedAlpha(0.8f));
        omniFilteredMeter[i].setGainReductionMeter(false);
      
      
        // ==== FILTER VISUALIZATION ====
      
        filterVisualizer.setSampleRate(processor.getCurrentSampleRate());
        filterVisualizer.setOverallGainInDecibels(0.0f);
        addAndMakeVisible (&filterVisualizer);
      
        if (i < numFilterBands - 1)
        {
            // cutoffs
            slFilterFrequencyAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "cutoff" + String(i), slFilterFrequency[i]);
            addAndMakeVisible(&slFilterFrequency[i]);
            slFilterFrequency[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
            slFilterFrequency[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
            slFilterFrequency[i].setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClRotSliderArrow);
            slFilterFrequency[i].setTooltip ("Cutoff " + String(i+1));
            slFilterFrequency[i].setName ("Cutoff" + String(i));
            slFilterFrequency[i].addListener (this);
          
            // filtervisualization / coeffs
            filterVisualizer.addCoefficients (processor.lowPassLRCoeffs[i], colours[i], &slFilterFrequency[i], &slMakeUpGain[i]);
            filterVisualizer.setFilterToHighlightFilteredBand (2*i, true);
            filterVisualizer.addCoefficients (processor.highPassLRCoeffs[i], colours[i+1], nullptr, &slMakeUpGain[i+1]);
            filterVisualizer.setFilterToHighlightFilteredBand (2*i + 1, true);
            filterVisualizer.setFilterToSyncWith (2*i + 1, 2*i);
        }


        // ===== LABELS =====
        addAndMakeVisible(&lbKnee[i]);
        lbKnee[i].setText("Knee");
        lbKnee[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible(&lbThreshold[i]);
        lbThreshold[i].setText("Threshold");
        lbThreshold[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible(&lbMakeUpGain[i]);
        lbMakeUpGain[i].setText("Makeup");
        lbMakeUpGain[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible(&lbRatio[i]);
        lbRatio[i].setText("Ratio");
        lbRatio[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible(&lbAttack[i]);
        lbAttack[i].setText("Attack");
        lbAttack[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible(&lbRelease[i]);
        lbRelease[i].setText("Release");
        lbRelease[i].setTextColour (globalLaF.ClFace);
    }
  
    // ==== METERS - INPUT/OUTPUT ====
    addAndMakeVisible(&omniInputMeter);
    omniInputMeter.setMinLevel(-60.0f);
    omniInputMeter.setColour(Colours::green.withMultipliedAlpha(0.8f));
    omniInputMeter.setGainReductionMeter(false);
    addAndMakeVisible(&lbInput);
    lbInput.setText ("Input");
    lbInput.setTextColour (globalLaF.ClFace);
  
    addAndMakeVisible(&omniOutputMeter);
    omniOutputMeter.setMinLevel(-60.0f);
    omniOutputMeter.setColour(Colours::green.withMultipliedAlpha(0.8f));
    omniOutputMeter.setGainReductionMeter(false);
    addAndMakeVisible(&lbOutput);
    lbOutput.setText ("Output");
    lbOutput.setTextColour (globalLaF.ClFace);
  
  
    // ==== MASTER CONTROLS ====
    addAndMakeVisible (&slMasterThreshold);
    slMasterThreshold.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slMasterThreshold.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
    slMasterThreshold.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClSliderFace);
    slMasterThreshold.setNormalisableRange (NormalisableRange<double> (-50.0f, 50.0f, 0.1f));
    slMasterThreshold.setValue (0.0f);
    slMasterThreshold.setTextValueSuffix (" dB");
    slMasterThreshold.setName ("MasterThreshold");
    slMasterThreshold.addListener (this);
    masterSliderPrevValueMap["Threshold"] = slMasterThreshold.getValue();
    addAndMakeVisible(&lbThreshold[numFilterBands]);
    lbThreshold[numFilterBands].setText("Threshold");
    lbThreshold[numFilterBands].setTextColour (globalLaF.ClFace);
  
    addAndMakeVisible (&slMasterKnee);
    slMasterKnee.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slMasterKnee.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
    slMasterKnee.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClSliderFace);
    slMasterKnee.setNormalisableRange (NormalisableRange<double> (-30.0f, 30.0f, 0.1f));
    slMasterKnee.setValue (0.0f);
    slMasterKnee.setTextValueSuffix (" dB");
    slMasterKnee.setName ("MasterKnee");
    slMasterKnee.addListener (this);
    masterSliderPrevValueMap["Knee"] = slMasterKnee.getValue();
    addAndMakeVisible(&lbKnee[numFilterBands]);
    lbKnee[numFilterBands].setText("Knee");
    lbKnee[numFilterBands].setTextColour (globalLaF.ClFace);
  
    addAndMakeVisible (&slMasterMakeUpGain);
    slMasterMakeUpGain.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slMasterMakeUpGain.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
    slMasterMakeUpGain.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClSliderFace);
    slMasterMakeUpGain.setNormalisableRange (NormalisableRange<double> (-50.0f, 50.0f, 0.1f));
    slMasterMakeUpGain.setValue (0.0f);
    slMasterMakeUpGain.setTextValueSuffix (" dB");
    slMasterMakeUpGain.setName ("MasterMakeUpGain");
    slMasterMakeUpGain.addListener (this);
    masterSliderPrevValueMap["MakeUpGain"] = slMasterMakeUpGain.getValue();
    addAndMakeVisible(&lbMakeUpGain[numFilterBands]);
    lbMakeUpGain[numFilterBands].setText("Makeup");
    lbMakeUpGain[numFilterBands].setTextColour (globalLaF.ClFace);
  
    addAndMakeVisible (&slMasterRatio);
    slMasterRatio.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slMasterRatio.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
    slMasterRatio.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClSliderFace);
    slMasterRatio.setNormalisableRange (NormalisableRange<double> (-15.0f, 15.0f, 0.1f));
    slMasterRatio.setValue (0.0f);
    slMasterRatio.setTextValueSuffix ("");
    slMasterRatio.setName ("MasterRatio");
    slMasterRatio.addListener (this);
    masterSliderPrevValueMap["Ratio"] = slMasterRatio.getValue();
    addAndMakeVisible(&lbRatio[numFilterBands]);
    lbRatio[numFilterBands].setText("Ratio");
    lbRatio[numFilterBands].setTextColour (globalLaF.ClFace);
  
    addAndMakeVisible (&slMasterAttackTime);
    slMasterAttackTime.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slMasterAttackTime.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
    slMasterAttackTime.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClSliderFace);
    slMasterAttackTime.setNormalisableRange (NormalisableRange<double> (-100.0f, 100.0f, 0.1f));
    slMasterAttackTime.setValue (0.0f);
    slMasterAttackTime.setTextValueSuffix (" ms");
    slMasterAttackTime.setName ("MasterAttackTime");
    slMasterAttackTime.addListener (this);
    masterSliderPrevValueMap["AttackTime"] = slMasterAttackTime.getValue();
    addAndMakeVisible(&lbAttack[numFilterBands]);
    lbAttack[numFilterBands].setText("Attack");
    lbAttack[numFilterBands].setTextColour (globalLaF.ClFace);
  
    addAndMakeVisible (&slMasterReleaseTime);
    slMasterReleaseTime.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slMasterReleaseTime.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
    slMasterReleaseTime.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClSliderFace);
    slMasterReleaseTime.setNormalisableRange (NormalisableRange<double> (-500.0f, 500.0f, 0.1f));
    slMasterReleaseTime.setValue (0.0f);
    slMasterReleaseTime.setTextValueSuffix (" ms");
    slMasterReleaseTime.setName ("MasterReleaseTime");
    slMasterReleaseTime.addListener (this);
    masterSliderPrevValueMap["ReleaseTime"] = slMasterReleaseTime.getValue();
    addAndMakeVisible(&lbRelease[numFilterBands]);
    lbRelease[numFilterBands].setText("Release");
    lbRelease[numFilterBands].setTextColour (globalLaF.ClFace);
  
    gcMasterControls.setText ("Master controls");
    addAndMakeVisible (&gcMasterControls);
  

    /* resized () is called here, because otherwise the compressorVisualizers won't be drawn to the GUI until one manually resizes the window.
    It seems resized() somehow gets called *before* the constructor and therefore OwnedArray<CompressorVisualizers> is still empty on the first resized call... */
    resized ();
  
    // start timer after everything is set up properly
    startTimer (20);
}

MultiBandCompressorAudioProcessorEditor::~MultiBandCompressorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void MultiBandCompressorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
  
}

void MultiBandCompressorAudioProcessorEditor::resized()
{
    // ============ BEGIN: header and footer ============
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());

    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);
    area.removeFromBottom (5);
    // =========== END: header and footer =================


    // ==== SPLIT INTO 4 BASIC SECTIONS ====
    const float filterToCompressorSectionRatio = 0.4f;
    const int filterToCompressorSeparator = 4;
    const float masterToVisualizationAreaRatio = 0.15f;
    const int masterToVisualizationAreaSeparator = 0;
    const float masterUpperToLowerRatio = 0.41f;
    const int masterUpperToLowerSeparator = 16;

    // split vertically into visualization section and 'master' (cutoffs, summed signal and master sliders) section
    Rectangle<int> compressorArea = area;
    Rectangle<int> masterArea = compressorArea.removeFromRight (compressorArea.proportionOfWidth (masterToVisualizationAreaRatio));
  
    // split horizontally into filter section and compressor section
    Rectangle<int> filterArea = compressorArea.removeFromTop (compressorArea.proportionOfHeight (filterToCompressorSectionRatio));
    filterArea.removeFromBottom (filterToCompressorSeparator / 2);
    compressorArea.removeFromTop (filterToCompressorSeparator / 2);
  
    Rectangle<int> upperMasterArea = masterArea.removeFromTop (masterArea.proportionOfHeight (masterUpperToLowerRatio));
    Rectangle<int> lowerMasterArea = masterArea;
    upperMasterArea.removeFromBottom (filterToCompressorSeparator / 2);
    lowerMasterArea.removeFromTop (filterToCompressorSeparator / 2);
    filterArea.removeFromRight (masterToVisualizationAreaSeparator / 2);
    compressorArea.removeFromRight (masterToVisualizationAreaSeparator / 2);
    upperMasterArea.removeFromLeft (masterUpperToLowerSeparator / 2);
    lowerMasterArea.removeFromLeft (masterUpperToLowerSeparator / 2);
  
  
    // ==== FILTER VISUALIZATION ====
    const int cutoffFilterSeparator = 5;
    filterVisualizer.setBounds (filterArea);
  
    Rectangle<int> filterCutoffArea = upperMasterArea.removeFromLeft(upperMasterArea.proportionOfWidth(0.5));
    const int sliderHeight = (filterCutoffArea.getHeight() - (numFilterBands-2)*cutoffFilterSeparator) / 3;
    for (int i = 0; i < numFilterBands-1; ++i)
    {
        slFilterFrequency[i].setBounds (filterCutoffArea.removeFromTop(sliderHeight));
        if (i < numFilterBands-2)
            filterCutoffArea.removeFromTop (cutoffFilterSeparator);
    }

  
    // ==== INPUT & OUTPUT METER
    const float labelToMeterRatio = 0.1f;
    Rectangle<int> summedSignalMeterArea = upperMasterArea;
    Rectangle<int> labelMeterArea = summedSignalMeterArea.removeFromBottom (summedSignalMeterArea.proportionOfHeight (labelToMeterRatio));
    omniInputMeter.setBounds (summedSignalMeterArea.removeFromLeft (summedSignalMeterArea.proportionOfWidth(0.5f)));
    omniOutputMeter.setBounds (summedSignalMeterArea);
    lbInput.setBounds (labelMeterArea.removeFromLeft (labelMeterArea.proportionOfWidth (0.5f)));
    lbOutput.setBounds (labelMeterArea);
  
  
  
  
    // ==== COMPRESSOR VISUALIZATION ====
    const float buttonsToCompressorsRatio = 0.08f;
    const int buttonsToCompressorVisualizationSeparator = 4;
    const float compressorParamToVisualizationRatio = 0.48f;
    const float metersToCompressorVisualizationRatio = 0.25f;
    const int paramRowSeparator = 4;
    const int paramToCompressorVisualizationSeparator = 4;
    const int compressorVisualizationSeparator = 20;
    const int compressorVisualizationToMeterSeparator = 8;
    const float labelToParamRatio = 0.15f;
  
    compressorArea.removeFromLeft (((compressorArea.getWidth() - (numFilterBands-1) * compressorVisualizationSeparator) % numFilterBands) / 2);
    compressorArea.removeFromRight (((compressorArea.getWidth() - (numFilterBands-1) * compressorVisualizationSeparator) % numFilterBands) / 2);
    const int widthPerBand = ((compressorArea.getWidth() - (numFilterBands-1) * compressorVisualizationSeparator) / numFilterBands);
  
    for (int i = 0; i < numFilterBands; ++i)
    {
    
        Rectangle<int> currentArea = compressorArea.removeFromLeft (widthPerBand);
    
        // Buttons
        Rectangle<int> buttonArea = currentArea.removeFromTop (currentArea.proportionOfHeight (buttonsToCompressorsRatio));
        buttonArea.removeFromBottom (buttonsToCompressorVisualizationSeparator / 2);
        currentArea.removeFromTop (buttonsToCompressorVisualizationSeparator / 2);
        Rectangle<int> soloButtonArea = buttonArea.removeFromLeft (buttonArea.proportionOfWidth (0.5));
        Rectangle<int> bypassButtonArea = buttonArea;
        const int bestWidth = tbBypass[i].getBestWidthForHeight (bypassButtonArea.getHeight());

        int centreX = soloButtonArea.getCentreX();
        soloButtonArea.setWidth(bestWidth);
        soloButtonArea.setCentre(centreX, soloButtonArea.getCentreY());
        tbSoloEnabled[i].setBounds (soloButtonArea);

        centreX = bypassButtonArea.getCentreX();
        bypassButtonArea.setWidth(bestWidth);
        bypassButtonArea.setCentre(centreX, bypassButtonArea.getCentreY());
        tbBypass[i].setBounds (bypassButtonArea);


        // Compressor parameters
        Rectangle<int> compressorParameterArea = currentArea.removeFromBottom (currentArea.proportionOfHeight (compressorParamToVisualizationRatio));
        compressorParameterArea.removeFromTop (paramToCompressorVisualizationSeparator / 2);
        compressorParameterArea.removeFromRight (compressorParameterArea.getWidth() % 3);
        const int sliderWidth = compressorParameterArea.getWidth() / 3;
      
        Rectangle<int> paramRow1 = compressorParameterArea.removeFromTop (compressorParameterArea.proportionOfHeight (0.5));
        Rectangle<int> paramRow2 = compressorParameterArea;
        paramRow1.removeFromBottom (paramRowSeparator / 2);
        paramRow2.removeFromTop (paramRowSeparator / 2);
        Rectangle<int> labelRow1 = paramRow1.removeFromBottom(paramRow1.proportionOfHeight (labelToParamRatio));
      
        lbThreshold[i].setBounds (labelRow1.removeFromLeft (sliderWidth));
        lbKnee[i].setBounds (labelRow1.removeFromLeft (sliderWidth));
        lbMakeUpGain[i].setBounds (labelRow1.removeFromLeft (sliderWidth));
        slThreshold[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
        slKnee[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
        slMakeUpGain[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
      
        Rectangle<int> labelRow2 = paramRow2.removeFromBottom(paramRow2.proportionOfHeight (labelToParamRatio));
        lbRatio[i].setBounds (labelRow2.removeFromLeft (sliderWidth));
        lbAttack[i].setBounds (labelRow2.removeFromLeft (sliderWidth));
        lbRelease[i].setBounds (labelRow2.removeFromLeft (sliderWidth));
        slRatio[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
        slAttackTime[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
        slReleaseTime[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
      
      
        // Compressor + meter visualization
        currentArea.removeFromBottom (paramToCompressorVisualizationSeparator / 2);
        Rectangle<int> meterArea = currentArea.removeFromRight (currentArea.proportionOfWidth (metersToCompressorVisualizationRatio));
        currentArea.removeFromRight (compressorVisualizationToMeterSeparator / 2);
        meterArea.removeFromLeft (compressorVisualizationToMeterSeparator / 2);
      
        omniFilteredMeter[i].setBounds (meterArea.removeFromLeft (meterArea.proportionOfWidth(0.5)));
        GRmeter[i].setBounds (meterArea);
      
        if (!(compressorVisualizers.isEmpty()))
            compressorVisualizers[i]->setBounds (currentArea);

        if (i < numFilterBands-1)
            compressorArea.removeFromLeft (compressorVisualizationSeparator);
    }
  
  
    // ==== MASTER SLIDERS + LABELS ====
    const float labelToSliderRatio = 0.2f;
  
    lowerMasterArea.removeFromTop (30);
    gcMasterControls.setBounds (lowerMasterArea);
    lowerMasterArea.removeFromTop (25);
  
    Rectangle<int> row1 = lowerMasterArea.removeFromTop (lowerMasterArea.proportionOfHeight (0.5f));
    row1.reduce(0, row1.proportionOfHeight (0.1));
    Rectangle<int> labelRow = row1.removeFromBottom (row1.proportionOfHeight (labelToSliderRatio));
    const int width = row1.proportionOfWidth (0.33f);
  
    slMasterThreshold.setBounds (row1.removeFromLeft (width));
    slMasterKnee.setBounds (row1.removeFromLeft (width));
    slMasterMakeUpGain.setBounds (row1.removeFromLeft (width));
    lbThreshold[numFilterBands].setBounds (labelRow.removeFromLeft (width));
    lbKnee[numFilterBands].setBounds (labelRow.removeFromLeft (width));
    lbMakeUpGain[numFilterBands].setBounds (labelRow.removeFromLeft (width));

    lowerMasterArea.reduce(0, lowerMasterArea.proportionOfHeight (0.1));
    labelRow = lowerMasterArea.removeFromBottom (lowerMasterArea.proportionOfHeight (labelToSliderRatio));

    slMasterRatio.setBounds (lowerMasterArea.removeFromLeft (width));
    slMasterAttackTime.setBounds (lowerMasterArea.removeFromLeft (width));
    slMasterReleaseTime.setBounds (lowerMasterArea.removeFromLeft (width));
  
    lbRatio[numFilterBands].setBounds (labelRow.removeFromLeft (width));
    lbAttack[numFilterBands].setBounds (labelRow.removeFromLeft (width));
    lbRelease[numFilterBands].setBounds (labelRow.removeFromLeft (width));
  
}


void MultiBandCompressorAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    // makeup gain affects filter magnitude
    if (slider->getName().startsWith("MakeUpGain"))
    {
        filterVisualizer.repaint();
        return;
    }

    // Cutoffs - prevent overlaps
    static double prevMidCutoff;
    static double prevLowCutoff;
    static double prevHighCutoff;
  
    // TODO: put in processor::parameterChanged
    if (slider->getName().startsWith("Cutoff"))
    {
        int f = slider->getName().getLastCharacters(1).getIntValue();
        double cutoff = slider->getValue();
        switch (f)
        {
            case (int)FilterIndex::LowIndex:
                if (!(cutoff <= slFilterFrequency[f+1].getValue()))
                {
                    slider->setValue(prevLowCutoff, NotificationType::dontSendNotification);
                    filterVisualizer.repaint();

                }
                else
                {
                    prevLowCutoff = cutoff;
                }
                break;
            
            case (int)FilterIndex::MidIndex:
                if (!(cutoff >= slFilterFrequency[f-1].getValue() &&
                      cutoff <= slFilterFrequency[f+1].getValue()))
                {
                    slider->setValue(prevMidCutoff, NotificationType::dontSendNotification);
                    filterVisualizer.repaint();

                }
                else
                {
                    prevMidCutoff = cutoff;
                }
                break;

            case (int)FilterIndex::HighIndex:
                if (!(cutoff >= slFilterFrequency[f-1].getValue()))
                {
                    slider->setValue(prevHighCutoff, NotificationType::dontSendNotification);
                    filterVisualizer.repaint();
                }
                else
                {
                    prevHighCutoff = cutoff;
                }
                break;
        }
    }
  
    // Master sliders control
    if (slider->getName().startsWith("Master"))
    {
        ReverseSlider* parameterSlidersControlledByMaster;
        const String id = slider->getName().fromLastOccurrenceOf("Master", false, true);
        if (id.contains ("Threshold"))       parameterSlidersControlledByMaster = slThreshold;
        else if (id.contains ("Knee"))       parameterSlidersControlledByMaster = slKnee;
        else if (id.contains ("MakeUpGain")) parameterSlidersControlledByMaster = slMakeUpGain;
        else if (id.contains ("Ratio"))      parameterSlidersControlledByMaster = slRatio;
        else if (id.contains ("AttackTime"))     parameterSlidersControlledByMaster = slAttackTime;
        else if (id.contains ("ReleaseTime"))    parameterSlidersControlledByMaster = slReleaseTime;
      
        double currentValue = slider->getValue();
        double offset = currentValue - masterSliderPrevValueMap[id];
      
        // first do a check if we can update without breaking ratios between individual parameters
        // FIXME: not working smoothly
        for (int i = 0; i < numFilterBands; ++i)
        {
            if (!(parameterSlidersControlledByMaster[i].getRange().contains (parameterSlidersControlledByMaster[i].getValue() + offset)))
            {
                return;
            }
        }
      
        for (int i = 0; i < numFilterBands; ++i)
        {
            parameterSlidersControlledByMaster[i].setValue (offset + parameterSlidersControlledByMaster[i].getValue());
        }
        masterSliderPrevValueMap[id] = currentValue;
    }
}


void MultiBandCompressorAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    //int maxInSize, maxOutSize;
    // processor.getMaxSize (maxInSize, maxOutSize);
    //title.setMaxSize (maxInSize, maxOutSize);
    // ==========================================

    if (processor.repaintFilterVisualization.get())
    {
        processor.repaintFilterVisualization = false;
        filterVisualizer.setSampleRate(processor.getCurrentSampleRate());
        filterVisualizer.repaint();
    }
  
    omniInputMeter.setLevel (processor.inputRMS.get());
    omniOutputMeter.setLevel (processor.outputRMS.get());
  
    for (int i = 0; i < numFilterBands; ++i)
    {
        compressorVisualizers[i]->setMarkerLevels(*valueTreeState.getRawParameterValue("maxRMS" + String(i)), *valueTreeState.getRawParameterValue("maxGR" + String(i)));
        compressorVisualizers[i]->updateCharacteristic();
        compressorVisualizers[i]->repaint();
      
        omniFilteredMeter[i].setLevel(*valueTreeState.getRawParameterValue("maxRMS" + String(i)));
        GRmeter[i].setLevel(*valueTreeState.getRawParameterValue("maxGR" + String(i)));
    }

}
