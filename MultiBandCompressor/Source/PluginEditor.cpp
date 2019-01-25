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
    : AudioProcessorEditor (&p), processor (p), valueTreeState (vts), footer (p.getOSCReceiver()), filterBankVisualizer(20.0f, 20000.0f, -15.0f, 20.0f, 5.0f)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    setResizeLimits (1000, 1000*0.54, 1600, 1600*0.54); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (String ("MultiBand"), String ("Compressor"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
  
    cbNormalizationAtachement = std::make_unique<ComboBoxAttachment> (valueTreeState, "useSN3D", *title.getInputWidgetPtr()->getNormCbPointer());
    cbOrderAtachement = std::make_unique<ComboBoxAttachment> (valueTreeState,"orderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());
  
    tooltips.setMillisecondsBeforeTipAppears(800);
    tooltips.setOpaque (false);
  
    const Colour colours[numFilterBands] =
    {
        Colours::cornflowerblue,
        Colours::greenyellow,
        Colours::yellow,
        Colours::orangered
    };
  
  
    // ==== FILTERS ====
    filterBankVisualizer.setSampleRate(processor.getCurrentSampleRate());
    filterBankVisualizer.setOverallGain(0.0f);
    filterBankVisualizer.setNumFreqBands (numFilterBands);
    addAndMakeVisible (&filterBankVisualizer);
  
    for (int i = 0; i < numFilterBands -1; ++i)
    {
        // ==== CROSSOVER SLIDERS ====
        slCrossoverAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState, "crossover" + String(i), slCrossover[i]);
        addAndMakeVisible(&slCrossover[i]);
        slCrossover[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slCrossover[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
        slCrossover[i].setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClRotSliderArrow);
        slCrossover[i].setTooltip ("Crossover Frequency " + String(i+1));
        slCrossover[i].setName ("Crossover" + String(i));
        slCrossover[i].addListener (this);
      
        // add coefficients to visualizer
        filterBankVisualizer.addCoefficients (processor.lowPassLRCoeffs[i], colours[i], &slCrossover[i], &slMakeUpGain[i]);
        filterBankVisualizer.addCoefficients (processor.highPassLRCoeffs[i], colours[i+1], &slCrossover[i], &slMakeUpGain[i+1]);
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
    addAndMakeVisible(&lbThreshold[numFilterBands]);
    lbThreshold[numFilterBands].setText("Threshold");
    lbThreshold[numFilterBands].setTextColour (globalLaF.ClFace);

    addAndMakeVisible (&slMasterMakeUpGain);
    slMasterMakeUpGain.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slMasterMakeUpGain.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
    slMasterMakeUpGain.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClSliderFace);
    slMasterMakeUpGain.setNormalisableRange (NormalisableRange<double> (-50.0f, 50.0f, 0.1f));
    slMasterMakeUpGain.setValue (0.0f);
    slMasterMakeUpGain.setTextValueSuffix (" dB");
    slMasterMakeUpGain.setName ("MasterMakeUpGain");
    addAndMakeVisible(&lbMakeUpGain[numFilterBands]);
    lbMakeUpGain[numFilterBands].setText("Makeup");
    lbMakeUpGain[numFilterBands].setTextColour (globalLaF.ClFace);

    addAndMakeVisible (&slMasterAttackTime);
    slMasterAttackTime.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slMasterAttackTime.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 12);
    slMasterAttackTime.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClSliderFace);
    slMasterAttackTime.setNormalisableRange (NormalisableRange<double> (-100.0f, 100.0f, 0.1f));
    slMasterAttackTime.setValue (0.0f);
    slMasterAttackTime.setTextValueSuffix (" ms");
    slMasterAttackTime.setName ("MasterAttackTime");
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
    addAndMakeVisible(&lbRelease[numFilterBands]);
    lbRelease[numFilterBands].setText("Release");
    lbRelease[numFilterBands].setTextColour (globalLaF.ClFace);
  
    gcMasterControls.setText ("Master controls");
    addAndMakeVisible (&gcMasterControls);
  
  
    for (int i = 0; i < numFilterBands; ++i)
    {
    
        // ==== COMPRESSOR VISUALIZATION ====
        compressorVisualizers.add (new CompressorVisualizer (p.getCompressor (i)));
        addAndMakeVisible(compressorVisualizers[i]);
      
      
        // ==== BUTTONS ====
        tbSolo[i].setColour (ToggleButton::tickColourId, Colour (0xFFFFFF66).withMultipliedAlpha (0.85f));
        tbSolo[i].setScaleFontSize (0.75f);
        tbSolo[i].setButtonText ("S");
        tbSolo[i].setTooltip ("Solo band #" + String(i));
        tbSolo[i].setToggleState (false, dontSendNotification);
        soloAttachment[i]  = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "soloEnabled" + String(i), tbSolo[i]);
        tbBypass[i].setClickingTogglesState (true);
        addAndMakeVisible (&tbSolo[i]);
      
        tbBypass[i].setColour (ToggleButton::tickColourId, Colour (0xFF74809D).withMultipliedAlpha (0.85f));
        tbBypass[i].setScaleFontSize (0.75f);
        tbBypass[i].setToggleState (false, dontSendNotification);
        tbBypass[i].setButtonText ("B");
        tbBypass[i].setTooltip ("Bypass band #" + String(i));
        bypassAttachment[i] = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (valueTreeState, "bypass" + String(i), tbBypass[i]);
        tbBypass[i].setClickingTogglesState (true);
        addAndMakeVisible (&tbBypass[i]);
      
      
        // ==== SLIDERS ====
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
      
        // add sliders to master controls
        slMasterThreshold.addSlave (slThreshold[i]);
        slMasterMakeUpGain.addSlave (slMakeUpGain[ i]);
        slMasterAttackTime.addSlave (slAttackTime[i]);
        slMasterReleaseTime.addSlave (slReleaseTime[i]);


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


    // ==== SPLIT INTO 5 BASIC SECTIONS ====
    const float leftToRightRatio = 0.85;
    const int leftToRightGap = 6;
    const float filterBankToLowerRatio = 0.35f;
    const float meterToMasterRatio = 0.42f;
    const float crossoverAndButtonToCompressorsRatio = 0.16f;
    const int filterToCrossoverAndButtonGap = 4;
    const int compressorToCrossoverAndButtonGap = 4;
    const int meterToMasterGap = 16;
  
    // split
    Rectangle<int> leftArea = area.removeFromLeft (area.proportionOfWidth (leftToRightRatio));
    Rectangle<int> rightArea (area);
    leftArea.removeFromRight (leftToRightGap / 2);
    rightArea.removeFromLeft (leftToRightGap / 2);
    Rectangle<int> filterBankArea = leftArea.removeFromTop (leftArea.proportionOfHeight (filterBankToLowerRatio));
    Rectangle<int> compressorArea = leftArea;
    Rectangle<int> crossoverAndButtonArea = compressorArea.removeFromTop (compressorArea.proportionOfHeight (crossoverAndButtonToCompressorsRatio));
    Rectangle<int> meterArea = rightArea.removeFromTop (rightArea.proportionOfHeight (meterToMasterRatio));
    Rectangle<int> masterArea (rightArea);
  
    // safeguard against haphephobia
    filterBankArea.removeFromBottom (filterToCrossoverAndButtonGap / 2);
    crossoverAndButtonArea.removeFromTop (filterToCrossoverAndButtonGap / 2);
    crossoverAndButtonArea.removeFromBottom (compressorToCrossoverAndButtonGap / 2);
    compressorArea.removeFromTop (compressorToCrossoverAndButtonGap / 2);
    meterArea.removeFromBottom (meterToMasterGap / 2);
    masterArea.removeFromTop (meterToMasterGap / 2);
  
  
    // ==== FILTER VISUALIZATION ====
    filterBankVisualizer.setBounds (filterBankArea);


    // ==== BUTTONS AND CROSSOVER SLIDERS ====
    const int crossoverToButtonGap = 4;
    const int buttonToButtonGap = 10;
    const float crossoverToButtonsRatio = 0.65f;
    const float trimButtonsHeight = 0.125f;
    const float trimButtonsWidth = 0.125f;
    Rectangle<int> soloButtonArea;
    Rectangle<int> bypassButtonArea;
    Rectangle<int> crossoverArea;
  
    const int buttonsWidth = crossoverAndButtonArea.getWidth () / (numFilterBands + (numFilterBands-1)*crossoverToButtonsRatio);
    const int crossoverSliderWidth = buttonsWidth * crossoverToButtonsRatio;


    for (int i = 0; i < numFilterBands; ++i)
    {
        // Buttons
        bypassButtonArea = crossoverAndButtonArea.removeFromLeft (buttonsWidth);
        bypassButtonArea.reduce (crossoverToButtonGap / 2, 0);
        soloButtonArea = bypassButtonArea.removeFromLeft (bypassButtonArea.proportionOfWidth (0.5));
        soloButtonArea.removeFromRight (buttonToButtonGap / 2);
        bypassButtonArea.removeFromLeft (buttonToButtonGap / 2);
        tbSolo[i].setBounds (soloButtonArea.reduced (soloButtonArea.proportionOfWidth (trimButtonsWidth), soloButtonArea.proportionOfHeight (trimButtonsHeight)));
        tbBypass[i].setBounds  (bypassButtonArea.reduced (bypassButtonArea.proportionOfWidth (trimButtonsWidth), bypassButtonArea.proportionOfHeight (trimButtonsHeight)));
      
        // Sliders
        if (i < numFilterBands - 1)
        {
            crossoverArea = crossoverAndButtonArea.removeFromLeft (crossoverSliderWidth);
            slCrossover[i].setBounds (crossoverArea.reduced (crossoverToButtonGap / 2, 0));
        }
    }

  
    // ==== INPUT & OUTPUT METER
    const float labelToMeterRatio = 0.1f;
    const int meterToMeterGap = 8;
  
    meterArea.reduce (meterArea.proportionOfWidth (0.2f), 0);
    Rectangle<int> inputMeterArea = meterArea.removeFromLeft (meterArea.proportionOfWidth (0.5f));
    Rectangle<int> outputMeterArea = meterArea;
    inputMeterArea.removeFromRight (meterToMeterGap / 2);
    outputMeterArea.removeFromLeft (meterToMeterGap / 2);
    Rectangle<int> inputMeterLabelArea = inputMeterArea.removeFromBottom (inputMeterArea.proportionOfHeight (labelToMeterRatio));
    Rectangle<int> outputMeterLabelArea = outputMeterArea.removeFromBottom (outputMeterArea.proportionOfHeight (labelToMeterRatio));
  
    omniInputMeter.setBounds (inputMeterArea);
    omniOutputMeter.setBounds (outputMeterArea);
    lbInput.setBounds (inputMeterLabelArea);
    lbOutput.setBounds (outputMeterLabelArea);
  
  
    // ==== COMPRESSOR VISUALIZATION ====
    const float paramToCharacteristiscRatio = 0.48f;
    const float meterToCharacteristicRatio = 0.175f;
    const float labelToParamRatio = 0.15f;
    const int paramRowToRowGap = 4;
    const int paramToCharacteristicGap = 4;
    const int bandToBandGap = 6;
    const int meterToCharacteristicGap = 6;
    const float trimMeterHeightRatio = 0.02f;

    compressorArea.reduce (((compressorArea.getWidth() - (numFilterBands-1) * bandToBandGap) % numFilterBands) / 2, 0);
    const int widthPerBand = ((compressorArea.getWidth() - (numFilterBands-1) * bandToBandGap) / numFilterBands);
    Rectangle<int> characteristicArea, paramArea, paramRow1, paramRow2, labelRow1, labelRow2, grMeterArea;
  
    for (int i = 0; i < numFilterBands; ++i)
    {
        characteristicArea = compressorArea.removeFromLeft (widthPerBand);

        // Compressor parameters
        paramArea = characteristicArea.removeFromBottom (characteristicArea.proportionOfHeight (paramToCharacteristiscRatio));
        paramArea.removeFromTop (paramToCharacteristicGap / 2);
        characteristicArea.removeFromBottom (paramToCharacteristicGap / 2);
      
        paramArea.reduce ((paramArea.getWidth() % 3) / 2, 0);
        const int sliderWidth = paramArea.getWidth() / 3;
      
        paramRow1 = paramArea.removeFromTop (paramArea.proportionOfHeight (0.5f));
        paramRow2 = paramArea;
        paramRow1.removeFromBottom (paramRowToRowGap / 2);
        paramRow2.removeFromTop (paramRowToRowGap / 2);
        labelRow1 = paramRow1.removeFromBottom (paramRow1.proportionOfHeight (labelToParamRatio));
        labelRow2 = paramRow2.removeFromBottom (paramRow2.proportionOfHeight (labelToParamRatio));
      
        lbThreshold[i].setBounds (labelRow1.removeFromLeft (sliderWidth));
        lbKnee[i].setBounds (labelRow1.removeFromLeft (sliderWidth));
        lbMakeUpGain[i].setBounds (labelRow1.removeFromLeft (sliderWidth));
        slThreshold[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
        slKnee[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
        slMakeUpGain[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
      
        lbRatio[i].setBounds (labelRow2.removeFromLeft (sliderWidth));
        lbAttack[i].setBounds (labelRow2.removeFromLeft (sliderWidth));
        lbRelease[i].setBounds (labelRow2.removeFromLeft (sliderWidth));
        slRatio[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
        slAttackTime[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
        slReleaseTime[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
      
        // Gain-Reduction meter
        grMeterArea = characteristicArea.removeFromRight (characteristicArea.proportionOfWidth (meterToCharacteristicRatio));
        grMeterArea.removeFromLeft (meterToCharacteristicGap / 2);
        characteristicArea.removeFromRight (meterToCharacteristicGap / 2);
        GRmeter[i].setBounds (grMeterArea.reduced (0, grMeterArea.proportionOfHeight (trimMeterHeightRatio)));
      
        // Compressor characteristic
        if (!(compressorVisualizers.isEmpty()))
            compressorVisualizers[i]->setBounds (characteristicArea);

        if (i < numFilterBands-1)
            compressorArea.removeFromLeft (bandToBandGap);
    }
  
  
    // ==== MASTER SLIDERS + LABELS ====
    const float labelToSliderRatio = 0.2f;
    const int trimFromTop = 30;
    const int trimFromGroupComponentHeader = 25;
    const float trimSliderHeight = 0.125f;
    const float trimSliderWidth = 0.05f;
  
    masterArea.removeFromTop (trimFromTop);
    gcMasterControls.setBounds (masterArea);
    masterArea.removeFromTop (trimFromGroupComponentHeader);
  
    Rectangle<int> sliderRow = masterArea.removeFromTop (masterArea.proportionOfHeight (0.5f));
    sliderRow.reduce (sliderRow.proportionOfWidth (trimSliderWidth), sliderRow.proportionOfHeight (trimSliderHeight));
    Rectangle<int> labelRow = sliderRow.removeFromBottom (sliderRow.proportionOfHeight (labelToSliderRatio));
    const int sliderWidth = sliderRow.proportionOfWidth (0.5f);
  
    slMasterThreshold.setBounds (sliderRow.removeFromLeft (sliderWidth));
    slMasterMakeUpGain.setBounds (sliderRow.removeFromLeft (sliderWidth));
    lbThreshold[numFilterBands].setBounds (labelRow.removeFromLeft (sliderWidth));
    lbMakeUpGain[numFilterBands].setBounds (labelRow.removeFromLeft (sliderWidth));

    sliderRow = masterArea;
    sliderRow.reduce (sliderRow.proportionOfWidth (trimSliderWidth), sliderRow.proportionOfHeight (trimSliderHeight));
    labelRow = sliderRow.removeFromBottom (sliderRow.proportionOfHeight (labelToSliderRatio));

    slMasterAttackTime.setBounds (sliderRow.removeFromLeft (sliderWidth));
    slMasterReleaseTime.setBounds (sliderRow.removeFromLeft (sliderWidth));
    lbAttack[numFilterBands].setBounds (labelRow.removeFromLeft (sliderWidth));
    lbRelease[numFilterBands].setBounds (labelRow.removeFromLeft (sliderWidth));
  
}


void MultiBandCompressorAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    // makeup gain affects filter magnitude
    if (slider->getName().startsWith("MakeUpGain"))
    {
        filterBankVisualizer.repaint();
        return;
    }

    // Crossover - prevent overlaps
    static double prevMidCrossover;
    static double prevLowCrossover;
    static double prevHighCrossover;
  
    // TODO: put in processor::parameterChanged
    if (slider->getName().startsWith("Crossover"))
    {
        int f = slider->getName().getLastCharacters(1).getIntValue();
        double crossover = slider->getValue();
        switch (f)
        {
            case (int)FilterIndex::LowIndex:
                if (!(crossover <= slCrossover[f+1].getValue()))
                {
                    slider->setValue(prevLowCrossover, NotificationType::dontSendNotification);
                    filterBankVisualizer.repaint();

                }
                else
                {
                    prevLowCrossover = crossover;
                }
                break;
            
            case (int)FilterIndex::MidIndex:
                if (!(crossover >= slCrossover[f-1].getValue() &&
                      crossover <= slCrossover[f+1].getValue()))
                {
                    slider->setValue(prevMidCrossover, NotificationType::dontSendNotification);
                    filterBankVisualizer.repaint();

                }
                else
                {
                    prevMidCrossover = crossover;
                }
                break;

            case (int)FilterIndex::HighIndex:
                if (!(crossover >= slCrossover[f-1].getValue()))
                {
                    slider->setValue(prevHighCrossover, NotificationType::dontSendNotification);
                    filterBankVisualizer.repaint();
                }
                else
                {
                    prevHighCrossover = crossover;
                }
                break;
        }
    }
}


void MultiBandCompressorAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize (maxInSize, maxOutSize);
    title.setMaxSize (maxInSize, maxOutSize);
    // ==========================================

    if (processor.repaintFilterVisualization.get())
    {
        processor.repaintFilterVisualization = false;
        filterBankVisualizer.setSampleRate(processor.getCurrentSampleRate());
        filterBankVisualizer.repaint();
    }
  
    omniInputMeter.setLevel (processor.inputPeak.get());
    omniOutputMeter.setLevel (processor.outputPeak.get());
  
    float gainReduction;
    for (int i = 0; i < numFilterBands; ++i)
    {
        gainReduction = processor.maxGR[i].get();
        compressorVisualizers[i]->setMarkerLevels(processor.maxPeak[i].get(), gainReduction);
        compressorVisualizers[i]->updateCharacteristic();
        compressorVisualizers[i]->repaint();
      
        GRmeter[i].setLevel(gainReduction);
    }

}
