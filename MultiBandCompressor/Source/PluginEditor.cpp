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
    : AudioProcessorEditor (&p), processor (p), valueTreeState (vts), footer (p.getOSCReceiver()), filterVisualizer(20.0f, 20000.0f, -15.0f, 5.0f, 5.0f), compressor(p.getCompressor(bandToDisplay))
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits (800, 800*0.618, 1300, 1300*0.618); // use this to create a resizable GUI
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
  
    const Colour colours[numFilterBands-1] =
    {
        Colours::cadetblue, // make sure you have enough colours in here
//        Colours::mediumpurple,
//        Colours::cornflowerblue,
        Colours::greenyellow,
//        Colours::yellow,
        Colours::orangered
    };

    for (int i = 0; i < numFilterBands; ++i)
    {
        // TODO: get rid of zombie filter cutoff dot at 0 Hz
        if (i < numFilterBands - 1)
        {
            slFilterFrequencyAttachment[i] = std::make_unique<SliderAttachment>(valueTreeState, "cutoff" + String(i), slFilterFrequency[i]);
            addAndMakeVisible(&slFilterFrequency[i]);

            slFilterFrequency[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
            slFilterFrequency[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
            slFilterFrequency[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
            slFilterFrequency[i].setTooltip("Cutoff " + String(i+1));

            slFilterFrequency[i].addListener(this);
          
            filterVisualizer.addCoefficients (processor.lowPassLRCoeffs[i], colours[i], &slFilterFrequency[i]);
            filterVisualizer.addCoefficients (processor.highPassLRCoeffs[i], colours[i]); // TODO: correct for not bold filter response
        }
      
        addAndMakeVisible(&tbBandSelect[i]);
        tbBandSelect[i].setColour(ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);
        tbBandSelect[i].setTooltip("select Band " + String(i+1));
        if (i == 0)
          tbBandSelect[i].setToggleState(true, dontSendNotification);
        tbBandSelect[i].addListener(this);
      
        tbSoloEnabled[i].setColour(ToggleButton::tickColourId, globalLaF.ClWidgetColours[3]);
        tbSoloEnabled[i].setButtonText("S");
        tbSoloEnabled[i].setTooltip("toggle Solo for Band " + String(i+1));
        tbSoloEnabled[i].setToggleState(false, dontSendNotification);
        soloAttachment[i]  = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "soloEnabled" + String(i), tbSoloEnabled[i]);
        addAndMakeVisible(&tbSoloEnabled[i]);
    }
  
//        TODO: make same colour as filter visualizations
//        tbCompressionEnabled[i].setColour(ToggleButton::tickColourId, globalLaF.ClWidgetColours[3]);
    tbCompressionEnabled.setTooltip("enable compression on band #" + String(bandToDisplay+1));
    tbCompressionEnabled.setToggleState(true, dontSendNotification);
    tbCompressionEnabled.setButtonText("compression on/off");
    compressionEnabledAttachment.insert(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "compressionEnabled" + String(bandToDisplay), tbCompressionEnabled));
//    compressionEnabledAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "compressionEnabled" + String(i), tbCompressionEnabled[i]);
    addAndMakeVisible(&tbCompressionEnabled);
  
    addAndMakeVisible(&slThreshold);
    attachmentMap [&slThreshold] = std::make_unique<SliderAttachment> (valueTreeState, "threshold" + String(bandToDisplay), slThreshold);
    slThreshold.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slThreshold.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slThreshold.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slThreshold.setTextValueSuffix(" dB");
  
    addAndMakeVisible(&slKnee);
    attachmentMap [&slKnee] = std::make_unique<SliderAttachment> (valueTreeState, "knee" + String(bandToDisplay), slKnee);
    slKnee.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slKnee.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slKnee.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slKnee.setTextValueSuffix(" dB");

    addAndMakeVisible(&slRatio);
    attachmentMap [&slRatio] = std::make_unique<SliderAttachment> (valueTreeState, "ratio" + String(bandToDisplay), slRatio);
    slRatio.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slRatio.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slRatio.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);

    addAndMakeVisible(&slAttackTime);
    attachmentMap [&slAttackTime] = std::make_unique<SliderAttachment> (valueTreeState, "attack" + String(bandToDisplay), slAttackTime);
    slAttackTime.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slAttackTime.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slAttackTime.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slRatio.setTextValueSuffix(" ms");

    addAndMakeVisible(&slReleaseTime);
    attachmentMap [&slReleaseTime] = std::make_unique<SliderAttachment> (valueTreeState, "release" + String(bandToDisplay), slReleaseTime);
    slReleaseTime.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slReleaseTime.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slReleaseTime.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slReleaseTime.setTextValueSuffix(" ms");

    addAndMakeVisible(&slMakeUpGain);
    attachmentMap [&slMakeUpGain] = std::make_unique<SliderAttachment> (valueTreeState, "makeUpGain" + String(bandToDisplay), slMakeUpGain);
    slMakeUpGain.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slMakeUpGain.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slMakeUpGain.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slMakeUpGain.setTextValueSuffix(" dB");

    addAndMakeVisible(&GRmeter);
    GRmeter.setMinLevel(-25.0f);
    GRmeter.setColour(Colours::red.withMultipliedAlpha(0.8f));
    GRmeter.setGainReductionMeter(true);

    addAndMakeVisible(&inpMeter);
    inpMeter.setMinLevel(-60.0f);
    inpMeter.setColour(Colours::green.withMultipliedAlpha(0.8f));
    inpMeter.setGainReductionMeter(false);

    // ===== LABELS =====
    addAndMakeVisible(&lbKnee);
    lbKnee.setText("Knee");

    addAndMakeVisible(&lbThreshold);
    lbThreshold.setText("Threshold");

    addAndMakeVisible(&lbMakeUpGain);
    lbMakeUpGain.setText("Makeup");

    addAndMakeVisible(&lbRatio);
    lbRatio.setText("Ratio");

    addAndMakeVisible(&lbAttack);
    lbAttack.setText("Attack");

    addAndMakeVisible(&lbRelease);
    lbRelease.setText("Release");
  
    filterVisualizer.setSampleRate(processor.getCurrentSampleRate());
    filterVisualizer.setOverallGainInDecibels(0.0f);
    addAndMakeVisible (&filterVisualizer);
  
    addAndMakeVisible(&compressor);

    // create the connection between title component's comboBoxes and parameters
    //cbInputChannelsSettingAttachment = new ComboBoxAttachment (valueTreeState, "inputChannelsSetting", *title.getInputWidgetPtr()->getChannelsCbPointer());
    //cbNormalizationSettingAttachment = new ComboBoxAttachment (valueTreeState, "useSN3D", *title.getOutputWidgetPtr()->getNormCbPointer());
    //cbOrderSettingAttachment = new ComboBoxAttachment (valueTreeState, "outputOrderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer());

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
  
    const int filterToCompressorSeparator = 8;
    const int visualizationToParameterSeparator = 4;
    const int totalNumFilterParams = 2*numFilterBands - 1;
    const float filterToCompressorRatio = 0.45;
    const float sliderRowSpacingRatio = 0.02f;
    const float trimFilterAreaRatio = 0.4f;
    const float trimFilterParamHeightRatio = 0.25f;
    const float meterToCompressorWidthRatio = 0.06;
  
    Rectangle<int> compressorArea = area;
    Rectangle<int> filterArea (compressorArea.removeFromLeft( area.getWidth() * filterToCompressorRatio));
    filterArea.removeFromRight(filterToCompressorSeparator);
    compressorArea.removeFromLeft(filterToCompressorSeparator);
  
    Rectangle<int> filterParamArea (filterArea.removeFromBottom (filterArea.getHeight() / 3));
    Rectangle<int> compressorParamArea (compressorArea.removeFromBottom (compressorArea.getHeight() / 3));
  
    filterArea.removeFromBottom(visualizationToParameterSeparator);
    filterParamArea.removeFromTop(visualizationToParameterSeparator);
    compressorArea.removeFromBottom(visualizationToParameterSeparator);
    compressorParamArea.removeFromTop(visualizationToParameterSeparator);

    filterArea.removeFromTop(filterArea.getHeight() * trimFilterAreaRatio);
    filterVisualizer.setBounds(filterArea);

    filterParamArea.removeFromRight (filterParamArea.getWidth() % 7);
  
    const int trimParamAreaAmount = filterParamArea.getHeight() * trimFilterParamHeightRatio;
    filterParamArea.removeFromBottom(trimParamAreaAmount);
    filterParamArea.removeFromTop(trimParamAreaAmount);

    const int paramWidth = (filterParamArea.getWidth() / totalNumFilterParams);
    filterParamArea.removeFromRight((filterParamArea.getWidth() % totalNumFilterParams) / 2);
    filterParamArea.removeFromLeft((filterParamArea.getWidth() % totalNumFilterParams) / 2);
    const int paramHeight = filterParamArea.getHeight() / 2;
  
    for (int i = 0; i < numFilterBands; ++i)
    {
        Rectangle<int> toggleButtonArea = filterParamArea.removeFromLeft(paramWidth);
        tbSoloEnabled[i].setSize(paramWidth / 2, paramHeight);
        tbSoloEnabled[i].setBoundsToFit(toggleButtonArea.removeFromBottom(paramHeight), Justification::centred, true);
//        tbSoloEnabled[i].changeWidthToFitText();
      
        tbBandSelect[i].setSize(paramWidth / 2, paramHeight);
        tbBandSelect[i].setBoundsToFit(toggleButtonArea, Justification::centred, true);
      
        if (i < numFilterBands - 1)
        {
            slFilterFrequency[i].setBounds(filterParamArea.removeFromLeft (paramWidth));
        }
    }

  
    const int meterWidth = compressorArea.getWidth() * meterToCompressorWidthRatio;
    inpMeter.setBounds(compressorArea.removeFromLeft(meterWidth));
    compressorArea.removeFromLeft(meterWidth / 4);
    GRmeter.setBounds(compressorArea.removeFromRight(meterWidth));
    compressorArea.removeFromRight(meterWidth / 4);

    compressorParamArea.removeFromLeft(meterWidth + (meterWidth / 4));
    compressorParamArea.removeFromRight(meterWidth + (meterWidth / 4));
  
    tbCompressionEnabled.setBounds(compressorParamArea.removeFromTop(0.12f * compressorParamArea.getHeight()).removeFromLeft(compressorParamArea.getWidth() * 0.45f));
  
    compressorParamArea.removeFromTop(compressorParamArea.getHeight() * 0.05f);
  
    Rectangle<int> sliderRow1, sliderRow2;
    sliderRow1 = compressorParamArea.removeFromTop(compressorParamArea.getHeight() * 0.5f);
    sliderRow1.removeFromBottom(sliderRow1.getHeight() * sliderRowSpacingRatio);
    sliderRow2 = compressorParamArea.removeFromTop(compressorParamArea.getHeight());
    sliderRow2.removeFromTop(sliderRow2.getHeight() * sliderRowSpacingRatio);

    const int compParamWidth = sliderRow1.getWidth() / 6;
  
    slThreshold.setBounds(sliderRow1.removeFromLeft(compParamWidth));
    lbThreshold.setBounds(sliderRow1.removeFromLeft(compParamWidth));
    slKnee.setBounds(sliderRow1.removeFromLeft(compParamWidth));
    lbKnee.setBounds(sliderRow1.removeFromLeft(compParamWidth));
    slMakeUpGain.setBounds(sliderRow1.removeFromLeft(compParamWidth));
    lbMakeUpGain.setBounds(sliderRow1.removeFromLeft(compParamWidth));
    slRatio.setBounds(sliderRow2.removeFromLeft(compParamWidth));
    lbRatio.setBounds(sliderRow2.removeFromLeft(compParamWidth));
    slAttackTime.setBounds(sliderRow2.removeFromLeft(compParamWidth));
    lbAttack.setBounds(sliderRow2.removeFromLeft(compParamWidth));
    slReleaseTime.setBounds(sliderRow2.removeFromLeft(compParamWidth));
    lbRelease.setBounds(sliderRow2.removeFromLeft(compParamWidth));

    // TODO: compression on/off button
//    area.removeFromBottom(10);
//    tbLookAhead.setBounds(area.removeFromBottom(20).removeFromLeft(130));
//    area.removeFromBottom(10);
//    characteristic.setBounds(area);

    compressor.setBounds(compressorArea);
}

void MultiBandCompressorAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    static double prevMidCutoff;
    static double prevLowCutoff;
    static double prevHighCutoff;
  
    double cutoff = slider->getValue();
  
    for (int f = 0; f < numFilterBands - 1; ++f)
    {
        if (slider == &slFilterFrequency[f])
        {
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
    }
}

void MultiBandCompressorAudioProcessorEditor::buttonClicked(Button *button)
{

  
    for (int i = 0; i < numFilterBands; i++)
    {
        if (button != &tbBandSelect[i])
        {
            tbBandSelect[i].setToggleState(false, dontSendNotification);
        }
        else
        {
            tbBandSelect[i].setToggleState(true, dontSendNotification);
            bandToDisplay = i;
          
            attachmentMap.clear();
            attachmentMap [&slThreshold] = std::make_unique<SliderAttachment> (valueTreeState, "threshold" + String(i), slThreshold);
            attachmentMap [&slKnee] = std::make_unique<SliderAttachment> (valueTreeState, "knee" + String(i), slKnee);
            attachmentMap [&slRatio] = std::make_unique<SliderAttachment> (valueTreeState, "ratio" + String(i), slRatio);
            attachmentMap [&slAttackTime] = std::make_unique<SliderAttachment> (valueTreeState, "attack" + String(i), slAttackTime);
            attachmentMap [&slReleaseTime] = std::make_unique<SliderAttachment> (valueTreeState, "release" + String(i), slReleaseTime);
            attachmentMap [&slMakeUpGain] = std::make_unique<SliderAttachment> (valueTreeState, "makeUpGain" + String(i), slMakeUpGain);
          
            tbCompressionEnabled.setTooltip("enable compression on band #" + String(i+1));
            compressionEnabledAttachment.clear();
            compressionEnabledAttachment.insert(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "compressionEnabled" + String(bandToDisplay), tbCompressionEnabled));
          
            compressor.setCompressorToVisualize(processor.getCompressor(i));
            compressor.updateCharacteristic();
            compressor.setMarkerLevels(*valueTreeState.getRawParameterValue("maxRMS" + String(i)),
                                       *valueTreeState.getRawParameterValue("maxGR" + String(i)));
            compressor.repaint();
        }
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
  
    compressor.setMarkerLevels(*valueTreeState.getRawParameterValue("maxRMS" + String(bandToDisplay)),
                               *valueTreeState.getRawParameterValue("maxGR" + String(bandToDisplay)));
    compressor.updateCharacteristic();
    compressor.repaint();
  
//    DBG(*valueTreeState.getRawParameterValue("ratio" + String(3)));

    inpMeter.setLevel(*valueTreeState.getRawParameterValue("maxRMS" + String(bandToDisplay)));
    GRmeter.setLevel(*valueTreeState.getRawParameterValue("maxGR" + String(bandToDisplay)));
}
