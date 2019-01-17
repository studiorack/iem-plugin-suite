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
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits (1200, 800*0.618, 1600, 1600*0.618); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (String ("MultiBand"), String ("Compressor"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
  
    cbInputChannelsSettingAttachment = std::make_unique<ComboBoxAttachment>(valueTreeState, "inputChannelsSetting", *title.getInputWidgetPtr()->getChannelsCbPointer());
//    cbOutputOrderSettingAttachment = std::make_unique<ComboBoxAttachment>(valueTreeState, "outputOrderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer());
  
    tooltips.setMillisecondsBeforeTipAppears(500);
    tooltips.setOpaque (false);
  
    const Colour colours[numFilterBands] =
    {
//        Colours::cadetblue, // make sure you have enough colours in here
//        Colours::mediumpurple,
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

//        addAndMakeVisible(&tbBandSelect[i]);
//        tbBandSelect[i].setColour(ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);
//        tbBandSelect[i].setTooltip("select Band " + String(i+1));
//        if (i == 0)
//          tbBandSelect[i].setToggleState(true, dontSendNotification);
//        tbBandSelect[i].addListener(this);
      
        tbSoloEnabled[i].setColour(ToggleButton::tickColourId, Colours::red);
        tbSoloEnabled[i].setButtonText("S");
        tbSoloEnabled[i].setTooltip("toggle Solo for Band " + String(i+1));
        tbSoloEnabled[i].setToggleState(false, dontSendNotification);
        soloAttachment[i]  = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "soloEnabled" + String(i), tbSoloEnabled[i]);
        addAndMakeVisible(&tbSoloEnabled[i]);
      
        //    TODO: make same colour as filter visualizations
        tbCompressionEnabled[i].setColour(ToggleButton::tickColourId, colours[i]);
        tbCompressionEnabled[i].setTooltip("enable compression on band #" + String(i+1));
        tbCompressionEnabled[i].setToggleState(true, dontSendNotification);
        tbCompressionEnabled[i].setButtonText("compression on/off");
        compressionEnabledAttachment[i] = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "compressionEnabled" + String(i), tbCompressionEnabled[i]);
        addAndMakeVisible(&tbCompressionEnabled[i]);
      
        addAndMakeVisible(&slThreshold[i]);
//        attachmentMap [&slThreshold[i]] = std::make_unique<SliderAttachment> (valueTreeState, "threshold" + String(bandToDisplay), slThreshold[i]);
        slThresholdAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "threshold" + String(i), slThreshold[i]);
        slThreshold[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slThreshold[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
        slThreshold[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slThreshold[i].setTextValueSuffix(" dB");
      
        addAndMakeVisible(&slKnee[i]);
//        attachmentMap [&slKnee] = std::make_unique<SliderAttachment> (valueTreeState, "knee" + String(bandToDisplay), slKnee);
        slKneeAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "knee" + String(i), slKnee[i]);
        slKnee[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slKnee[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
        slKnee[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slKnee[i].setTextValueSuffix(" dB");

        addAndMakeVisible(&slRatio[i]);
//        attachmentMap [&slRatio] = std::make_unique<SliderAttachment> (valueTreeState, "ratio" + String(bandToDisplay), slRatio);
        slRatioAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "ratio" + String (i), slRatio[i]);
        slRatio[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slRatio[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
        slRatio[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);

        addAndMakeVisible(&slAttackTime[i]);
//        attachmentMap [&slAttackTime] = std::make_unique<SliderAttachment> (valueTreeState, "attack" + String(bandToDisplay), slAttackTime);
        slAttackTimeAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "attack" + String(i), slAttackTime[i]);
        slAttackTime[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slAttackTime[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
        slAttackTime[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slRatio[i].setTextValueSuffix(" ms");

        addAndMakeVisible(&slReleaseTime[i]);
//        attachmentMap [&slReleaseTime] = std::make_unique<SliderAttachment> (valueTreeState, "release" + String(bandToDisplay), slReleaseTime);
        slReleaseTimeAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "release" + String(i), slReleaseTime[i]);
        slReleaseTime[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slReleaseTime[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
        slReleaseTime[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slReleaseTime[i].setTextValueSuffix(" ms");

        addAndMakeVisible(&slMakeUpGain[i]);
//        attachmentMap [&slMakeUpGain] = std::make_unique<SliderAttachment> (valueTreeState, "makeUpGain" + String(bandToDisplay), slMakeUpGain);
        slMakeUpGainAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "makeUpGain" + String(i), slMakeUpGain[i]);
        slMakeUpGain[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slMakeUpGain[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
        slMakeUpGain[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slMakeUpGain[i].setTextValueSuffix(" dB");
        slMakeUpGain[i].setName(String ("MakeUpGain" + String(i)));
        slMakeUpGain[i].addListener(this);

        addAndMakeVisible(&GRmeter[i]);
        GRmeter[i].setMinLevel(-25.0f);
        GRmeter[i].setColour(Colours::red.withMultipliedAlpha(0.8f));
        GRmeter[i].setGainReductionMeter(true);

        addAndMakeVisible(&inpMeter[i]);
        inpMeter[i].setMinLevel(-60.0f);
        inpMeter[i].setColour(Colours::green.withMultipliedAlpha(0.8f));
        inpMeter[i].setGainReductionMeter(false);
      
      
        // ==== FILTER VISUALIZATION ====
      
        filterVisualizer.setSampleRate(processor.getCurrentSampleRate());
        filterVisualizer.setOverallGainInDecibels(0.0f);
        addAndMakeVisible (&filterVisualizer);
      
        if (i < numFilterBands - 1)
        {
            slFilterFrequencyAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "cutoff" + String(i), slFilterFrequency[i]);
            addAndMakeVisible(&slFilterFrequency[i]);
            slFilterFrequency[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
            slFilterFrequency[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
            slFilterFrequency[i].setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClRotSliderArrow);
            slFilterFrequency[i].setTooltip ("Cutoff " + String(i+1));
            slFilterFrequency[i].setName ("Cutoff" + String(i));
            slFilterFrequency[i].addListener (this);
          
            filterVisualizer.addCoefficients (processor.lowPassLRCoeffs[i], colours[i], &slFilterFrequency[i], &slMakeUpGain[i]);
            filterVisualizer.setFilterToHighlightFilteredBand (2*i, true);
            filterVisualizer.addCoefficients (processor.highPassLRCoeffs[i], colours[i+1], nullptr, &slMakeUpGain[i+1]);
            filterVisualizer.setFilterToHighlightFilteredBand (2*i + 1, true);
            filterVisualizer.setFilterToSyncWith (2*i + 1, 2*i);
        }

        // ===== LABELS =====
        addAndMakeVisible(&lbKnee[i]);
        lbKnee[i].setText("Knee");

        addAndMakeVisible(&lbThreshold[i]);
        lbThreshold[i].setText("Threshold");

        addAndMakeVisible(&lbMakeUpGain[i]);
        lbMakeUpGain[i].setText("Makeup");

        addAndMakeVisible(&lbRatio[i]);
        lbRatio[i].setText("Ratio");

        addAndMakeVisible(&lbAttack[i]);
        lbAttack[i].setText("Attack");

        addAndMakeVisible(&lbRelease[i]);
        lbRelease[i].setText("Release");
    }

    // create the connection between title component's comboBoxes and parameters
    //cbInputChannelsSettingAttachment = new ComboBoxAttachment (valueTreeState, "inputChannelsSetting", *title.getInputWidgetPtr()->getChannelsCbPointer());
    //cbNormalizationSettingAttachment = new ComboBoxAttachment (valueTreeState, "useSN3D", *title.getOutputWidgetPtr()->getNormCbPointer());
    //cbOrderSettingAttachment = new ComboBoxAttachment (valueTreeState, "outputOrderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer());

    // resized () is called here, because otherwise the compressorVisualizers won't be drawn to the GUI until one manually resizes the window.
    // It seems resized() somehow gets called *before* the constructor and therefore OwnedArray<CompressorVisualizers> is still empty on the first resized call...
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
  
//    g.setColour (Colours::red);
//    for (int i = 0; i < numFilterBands; ++i)
//    {
//          g.drawRect (slThreshold[i].getBounds());
//          g.drawRect (slKnee[i].getBounds());
//          g.drawRect (slMakeUpGain[i].getBounds());
//          g.drawRect (slRatio[i].getBounds());
//          g.drawRect (slAttackTime[i].getBounds());
//          g.drawRect (slReleaseTime[i].getBounds());
//    }
  
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
    const int filterToCompressorSeparator = 8;
    const float masterToVisualizationAreaRatio = 0.15f;
    const int masterToVisualizationAreaSeparator = 8;
    const float masterUpperToLowerRatio = 0.5f;
    const int masterUpperToLowerSeparator = 16;

    // split vertically into visualization section and master section
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
    const int cutoffFilterGap = 5;
    filterVisualizer.setBounds (filterArea);
  
    Rectangle<int> filterCutoffArea = upperMasterArea.removeFromLeft(upperMasterArea.proportionOfWidth(0.5));
    const int sliderHeight = (filterCutoffArea.getHeight() - (numFilterBands-2)*cutoffFilterGap) / 3;
    for (int i = 0; i < numFilterBands-1; ++i)
    {
        slFilterFrequency[i].setBounds (filterCutoffArea.removeFromTop(sliderHeight));
        if (i < numFilterBands-2)
            filterCutoffArea.removeFromTop (cutoffFilterGap);
    }
  
    //    const float filterToFilterParamterRatio = 0.25f;
    //    const int totalNumFilterParams = 2*numFilterBands - 1;
    //
    //    Rectangle<int> filterParameterArea = filterArea.removeFromBottom (filterArea.proportionOfHeight (filterToFilterParamterRatio));
  
    //    filterParameterArea.removeFromRight ((filterParameterArea.getWidth() % totalNumFilterParams) / 2);
    //    filterParameterArea.removeFromLeft ((filterParameterArea.getWidth() % totalNumFilterParams) / 2);
    //    const int paramWidth = (filterParameterArea.getWidth() / totalNumFilterParams);
    //    filterParameterArea.removeFromLeft (paramWidth);
    //    for (int i = 0; i < numFilterBands-1; ++i)
    //    {
    //        slFilterFrequency[i].setBounds (filterParameterArea.removeFromLeft (paramWidth));
    //        filterParameterArea.removeFromLeft (paramWidth);
    //    }
  
  
    // ==== COMPRESSOR VISUALIZATION ====
    const float soloAndBypassButtonsToCompressorsRatio = 0.1f;
    const float compressorParamToVisualizationRatio = 0.45f;
    const float metersToCompressorVisualizationRatio = 0.25f;
    const int compressorVisualizationSeparator = 20;
    const int compressorVisualizationToMeterSeparator = 8;
  
    compressorArea.removeFromLeft (((compressorArea.getWidth() - (numFilterBands-1) * compressorVisualizationSeparator) % numFilterBands) / 2);
    compressorArea.removeFromRight (((compressorArea.getWidth() - (numFilterBands-1) * compressorVisualizationSeparator) % numFilterBands) / 2);
    const int widthPerBand = ((compressorArea.getWidth() - (numFilterBands-1) * compressorVisualizationSeparator) / numFilterBands);
  
    for (int i = 0; i < numFilterBands; ++i)
    {
    
        Rectangle<int> currentArea = compressorArea.removeFromLeft (widthPerBand);
    
        // Buttons
        Rectangle<int> buttonArea = currentArea.removeFromTop (currentArea.proportionOfHeight (soloAndBypassButtonsToCompressorsRatio));
        Rectangle<int> soloButtonArea = buttonArea.removeFromLeft (buttonArea.proportionOfWidth (0.5));
        Rectangle<int> bypassButtonArea = buttonArea;
        tbSoloEnabled[i].setBounds (soloButtonArea);
        tbCompressionEnabled[i].setBounds (bypassButtonArea);
      
        // Compressor parameters
        Rectangle<int> compressorParameterArea = currentArea.removeFromBottom (currentArea.proportionOfHeight (compressorParamToVisualizationRatio));
        compressorParameterArea.removeFromRight (compressorParameterArea.getWidth() % 3);
        const int sliderWidth = compressorParameterArea.getWidth() / 3;
      
        Rectangle<int> paramRow1 = compressorParameterArea.removeFromTop (compressorParameterArea.proportionOfHeight (0.5));
        slThreshold[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
        slKnee[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
        slMakeUpGain[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
      
        Rectangle<int> paramRow2 = compressorParameterArea;
        slRatio[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
        slAttackTime[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
        slReleaseTime[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
      
        // Compressor + meter visualization
        Rectangle<int> meterArea = currentArea.removeFromRight (currentArea.proportionOfWidth (metersToCompressorVisualizationRatio));
        currentArea.removeFromRight (compressorVisualizationToMeterSeparator / 2);
        meterArea.removeFromLeft (compressorVisualizationToMeterSeparator / 2);
      
        inpMeter[i].setBounds (meterArea.removeFromLeft (meterArea.proportionOfWidth(0.5)));
        GRmeter[i].setBounds (meterArea);
      
        if (!(compressorVisualizers.isEmpty()))
            compressorVisualizers[i]->setBounds (currentArea);

        if (i < numFilterBands-1)
            compressorArea.removeFromLeft (compressorVisualizationSeparator);
    }
  
  
  
  
  
  
//    Rectangle<int> filterAreavisualizationArea
  
//
//    Rectangle<int> compressorArea = area;
//    Rectangle<int> filterArea (compressorArea.removeFromLeft( area.getWidth() * filterToCompressorRatio));
//    filterArea.removeFromRight(filterToCompressorSeparator);
//    compressorArea.removeFromLeft(filterToCompressorSeparator);
//
//    Rectangle<int> filterParamArea (filterArea.removeFromBottom (filterArea.getHeight() / 3));
//    Rectangle<int> compressorParamArea (compressorArea.removeFromBottom (compressorArea.getHeight() / 3));
//
//    filterArea.removeFromBottom(visualizationToParameterSeparator);
//    filterParamArea.removeFromTop(visualizationToParameterSeparator);
//    compressorArea.removeFromBottom(visualizationToParameterSeparator);
//    compressorParamArea.removeFromTop(visualizationToParameterSeparator);
//
//    filterArea.removeFromTop(filterArea.getHeight() * trimFilterAreaRatio);
//    filterVisualizer.setBounds(filterArea);
//
//    filterParamArea.removeFromRight (filterParamArea.getWidth() % 7);
//
//    const int trimParamAreaAmount = filterParamArea.getHeight() * trimFilterParamHeightRatio;
//    filterParamArea.removeFromBottom(trimParamAreaAmount);
//    filterParamArea.removeFromTop(trimParamAreaAmount);
//
//    const int paramWidth = (filterParamArea.getWidth() / totalNumFilterParams);
//    filterParamArea.removeFromRight((filterParamArea.getWidth() % totalNumFilterParams) / 2);
//    filterParamArea.removeFromLeft((filterParamArea.getWidth() % totalNumFilterParams) / 2);
//    const int paramHeight = filterParamArea.getHeight() / 2;
//
//    for (int i = 0; i < numFilterBands; ++i)
//    {
//        Rectangle<int> toggleButtonArea = filterParamArea.removeFromLeft(paramWidth);
//        tbSoloEnabled[i].setSize(paramWidth / 2, paramHeight);
//        tbSoloEnabled[i].setBoundsToFit(toggleButtonArea.removeFromBottom(paramHeight), Justification::centred, true);
////        tbSoloEnabled[i].changeWidthToFitText();
//
//        tbBandSelect[i].setSize(paramWidth / 2, paramHeight);
//        tbBandSelect[i].setBoundsToFit(toggleButtonArea, Justification::centred, true);
//
//        if (i < numFilterBands - 1)
//        {
//            slFilterFrequency[i].setBounds(filterParamArea.removeFromLeft (paramWidth));
//        }
//    }
//
//
//    const int meterWidth = compressorArea.getWidth() * meterToCompressorWidthRatio;
//    inpMeter.setBounds(compressorArea.removeFromLeft(meterWidth));
//    compressorArea.removeFromLeft(meterWidth / 4);
//    GRmeter.setBounds(compressorArea.removeFromRight(meterWidth));
//    compressorArea.removeFromRight(meterWidth / 4);
//
//    compressorParamArea.removeFromLeft(meterWidth + (meterWidth / 4));
//    compressorParamArea.removeFromRight(meterWidth + (meterWidth / 4));
//
//    tbCompressionEnabled.setBounds(compressorParamArea.removeFromTop(0.12f * compressorParamArea.getHeight()).removeFromLeft(compressorParamArea.getWidth() * 0.45f));
//
//    compressorParamArea.removeFromTop(compressorParamArea.getHeight() * 0.05f);
//
//    Rectangle<int> sliderRow1, sliderRow2;
//    sliderRow1 = compressorParamArea.removeFromTop(compressorParamArea.getHeight() * 0.5f);
//    sliderRow1.removeFromBottom(sliderRow1.getHeight() * sliderRowSpacingRatio);
//    sliderRow2 = compressorParamArea.removeFromTop(compressorParamArea.getHeight());
//    sliderRow2.removeFromTop(sliderRow2.getHeight() * sliderRowSpacingRatio);
//
//    const int compParamWidth = sliderRow1.getWidth() / 6;
//
//    slThreshold.setBounds(sliderRow1.removeFromLeft(compParamWidth));
//    lbThreshold.setBounds(sliderRow1.removeFromLeft(compParamWidth));
//    slKnee.setBounds(sliderRow1.removeFromLeft(compParamWidth));
//    lbKnee.setBounds(sliderRow1.removeFromLeft(compParamWidth));
//    slMakeUpGain.setBounds(sliderRow1.removeFromLeft(compParamWidth));
//    lbMakeUpGain.setBounds(sliderRow1.removeFromLeft(compParamWidth));
//    slRatio.setBounds(sliderRow2.removeFromLeft(compParamWidth));
//    lbRatio.setBounds(sliderRow2.removeFromLeft(compParamWidth));
//    slAttackTime.setBounds(sliderRow2.removeFromLeft(compParamWidth));
//    lbAttack.setBounds(sliderRow2.removeFromLeft(compParamWidth));
//    slReleaseTime.setBounds(sliderRow2.removeFromLeft(compParamWidth));
//    lbRelease.setBounds(sliderRow2.removeFromLeft(compParamWidth));
//
//    // TODO: compression on/off button
////    area.removeFromBottom(10);
////    tbLookAhead.setBounds(area.removeFromBottom(20).removeFromLeft(130));
////    area.removeFromBottom(10);
////    characteristic.setBounds(area);
//
//    compressor.setBounds(compressorArea);
}

void MultiBandCompressorAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    if (slider->getName().startsWith("MakeUpGain"))
    {
        filterVisualizer.repaint();
        return;
    }

    static double prevMidCutoff;
    static double prevLowCutoff;
    static double prevHighCutoff;
  
    // TODO: put in processor::parameterChanged
    if (!(slider->getName().startsWith("Cutoff")))
        return;
  
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
  
//    for (int f = 0; f < numFilterBands - 1; ++f)
//    {
//        if (slider == &slFilterFrequency[f])
//        {
//            switch (f)
//            {
//                case (int)FilterIndex::LowIndex:
//                    if (!(cutoff <= slFilterFrequency[f+1].getValue()))
//                    {
//                        slider->setValue(prevLowCutoff, NotificationType::dontSendNotification);
//                        filterVisualizer.repaint();
//
//                    }
//                    else
//                    {
//                        prevLowCutoff = cutoff;
//                    }
//                    break;
//
//                case (int)FilterIndex::MidIndex:
//                    if (!(cutoff >= slFilterFrequency[f-1].getValue() &&
//                          cutoff <= slFilterFrequency[f+1].getValue()))
//                    {
//                        slider->setValue(prevMidCutoff, NotificationType::dontSendNotification);
//                        filterVisualizer.repaint();
//
//                    }
//                    else
//                    {
//                        prevMidCutoff = cutoff;
//                    }
//                    break;
//
//                case (int)FilterIndex::HighIndex:
//                    if (!(cutoff >= slFilterFrequency[f-1].getValue()))
//                    {
//                        slider->setValue(prevHighCutoff, NotificationType::dontSendNotification);
//                        filterVisualizer.repaint();
//                    }
//                    else
//                    {
//                        prevHighCutoff = cutoff;
//                    }
//                    break;
//            }
//        }
//    }
}

//void MultiBandCompressorAudioProcessorEditor::buttonClicked(Button *button)
//{
//
//
//    for (int i = 0; i < numFilterBands; i++)
//    {
//        if (button != &tbBandSelect[i])
//        {
//            tbBandSelect[i].setToggleState(false, dontSendNotification);
//        }
//        else
//        {
//            tbBandSelect[i].setToggleState(true, dontSendNotification);
//            bandToDisplay = i;
//
//            attachmentMap.clear();
//            attachmentMap [&slThreshold] = std::make_unique<SliderAttachment> (valueTreeState, "threshold" + String(i), slThreshold);
//            attachmentMap [&slKnee] = std::make_unique<SliderAttachment> (valueTreeState, "knee" + String(i), slKnee);
//            attachmentMap [&slRatio] = std::make_unique<SliderAttachment> (valueTreeState, "ratio" + String(i), slRatio);
//            attachmentMap [&slAttackTime] = std::make_unique<SliderAttachment> (valueTreeState, "attack" + String(i), slAttackTime);
//            attachmentMap [&slReleaseTime] = std::make_unique<SliderAttachment> (valueTreeState, "release" + String(i), slReleaseTime);
//            attachmentMap [&slMakeUpGain] = std::make_unique<SliderAttachment> (valueTreeState, "makeUpGain" + String(i), slMakeUpGain);
//
//            tbCompressionEnabled.setTooltip("enable compression on band #" + String(i+1));
//            compressionEnabledAttachment.clear();
//            compressionEnabledAttachment.insert(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "compressionEnabled" + String(bandToDisplay), tbCompressionEnabled));
//
//            compressor.setCompressorToVisualize(processor.getCompressor(i));
//            compressor.updateCharacteristic();
//            compressor.setMarkerLevels(*valueTreeState.getRawParameterValue("maxRMS" + String(i)),
//                                       *valueTreeState.getRawParameterValue("maxGR" + String(i)));
//            compressor.repaint();
//        }
//    }
//}

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
  
    for (int i = 0; i < numFilterBands; ++i)
    {
        compressorVisualizers[i]->setMarkerLevels(*valueTreeState.getRawParameterValue("maxRMS" + String(i)), *valueTreeState.getRawParameterValue("maxGR" + String(i)));
        compressorVisualizers[i]->updateCharacteristic();
        compressorVisualizers[i]->repaint();
      
    //    DBG(*valueTreeState.getRawParameterValue("ratio" + String(3)));

        inpMeter[i].setLevel(*valueTreeState.getRawParameterValue("maxRMS" + String(i)));
        GRmeter[i].setLevel(*valueTreeState.getRawParameterValue("maxGR" + String(i)));
    }

}
