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

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

class ColourChangeButton  : public TextButton,
public ChangeListener
{
public:
    ColourChangeButton(MultiEncoderAudioProcessor& p, SpherePanner::Element* elem, int i)
    : TextButton (String(i)), processor(p), id(i), element(elem)
    {
        setSize (10, 24);
        changeWidthToFitText();
    }
    void clicked() override
    {
        ColourSelector* colourSelector = new ColourSelector();
        colourSelector->setName ("background");
        colourSelector->setCurrentColour (findColour (TextButton::buttonColourId));
        colourSelector->addChangeListener (this);
        colourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
        colourSelector->setSize (300, 400);
        
        CallOutBox::launchAsynchronously (colourSelector, getScreenBounds(), nullptr);
    }
    
    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (ColourSelector* cs = dynamic_cast<ColourSelector*> (source))
        {
            processor.elementColours[id-1] = cs->getCurrentColour();
            setColour (TextButton::buttonColourId, cs->getCurrentColour());
            element->setColour (cs->getCurrentColour());
            setColour(TextButton::textColourOffId, Colours::white.overlaidWith (cs->getCurrentColour()).contrasting());
            element->setTextColour(Colours::white.overlaidWith (cs->getCurrentColour()).contrasting());
        }
    }
private:
    MultiEncoderAudioProcessor& processor;
    int id;
    SpherePanner::Element* element;
};

class  EncoderList :  public Component
{
public:
    EncoderList(MultiEncoderAudioProcessor& p, SpherePanner& sphere, AudioProcessorValueTreeState* vts) : Component(), processor(p), spherePanner(sphere),  pVts(vts), nChannels(2) {
        setNumberOfChannels(nChannels);
    };
    ~EncoderList() {};
    
    void setNumberOfChannels (int nCh) {
        const int nElements = sphereElementArray.size();
        const int nExcessive = nElements - nCh;
        
        if (nExcessive > 0) // we have to delete some
        {
            for (int i = nCh; i < nElements; ++i)
            {
                spherePanner.removeElement(sphereElementArray[i]);
            }
            slYawAttachmentArray.removeLast(nExcessive);
            slPitchAttachmentArray.removeLast(nExcessive);
            slGainAttachmentArray.removeLast(nExcessive);
            muteButtonAttachmentArray.removeLast(nExcessive);
            soloButtonAttachmentArray.removeLast(nExcessive);
            sphereElementArray.removeLast(nExcessive);
            colourChooserArray.removeLast(nExcessive);
            slYawArray.removeLast(nExcessive);
            slPitchArray.removeLast(nExcessive);
            slGainArray.removeLast(nExcessive);
            muteButtonArray.removeLast(nExcessive);
            soloButtonArray.removeLast(nExcessive);
        }
        else // we have to add some
        {
            for (int i = nElements; i < nCh; ++i)
            {
                sphereElementArray.add(new SpherePanner::Element());
                colourChooserArray.add(new ColourChangeButton(processor, sphereElementArray.getLast(), i+1));
                slYawArray.add(new ReverseSlider());
                slPitchArray.add(new ReverseSlider());
                slGainArray.add(new ReverseSlider());
                muteButtonArray.add(new MuteSoloButton());
                soloButtonArray.add(new MuteSoloButton());
                
                SpherePanner::Element* sphereElement = sphereElementArray.getLast();
                ColourChangeButton* colourChooser = colourChooserArray.getLast();
                ReverseSlider* yawSlider = slYawArray.getLast();
                ReverseSlider* pitchSlider = slPitchArray.getLast();
                ReverseSlider* gainSlider = slGainArray.getLast();
                MuteSoloButton* muteButton = muteButtonArray.getLast();
                MuteSoloButton* soloButton = soloButtonArray.getLast();
                
                sphereElement->setLabel(String(i+1));
                sphereElement->setID(String(i));
                sphereElement->setColour(processor.elementColours[i]);
                sphereElement->setTextColour(Colours::white.overlaidWith(processor.elementColours[i]).contrasting());
                sphereElement->setSliders(yawSlider, pitchSlider);
                
                spherePanner.addElement(sphereElement);
                
                addAndMakeVisible(colourChooser);
                colourChooser->setColour(TextButton::buttonColourId, processor.elementColours[i]);
                colourChooser->setColour(TextButton::textColourOffId, Colours::white.overlaidWith (processor.elementColours[i]).contrasting());
                
                
                addAndMakeVisible(yawSlider);
                slYawAttachmentArray.add(new SliderAttachment(*pVts,"yaw"+String(i), *yawSlider));
                yawSlider->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
                yawSlider->setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
                yawSlider->setReverse(true);
                yawSlider->setColour (Slider::rotarySliderOutlineColourId, Colour(0xFF00CAFF));
                yawSlider->setRotaryParameters(M_PI, 3*M_PI, false);
                yawSlider->setTooltip("Yaw angle");
                yawSlider->setTextValueSuffix(CharPointer_UTF8 ("\xc2\xb0"));
                
                addAndMakeVisible(pitchSlider);
                slPitchAttachmentArray.add(new SliderAttachment(*pVts,"pitch" + String(i), *pitchSlider));
                pitchSlider->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
                pitchSlider->setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
                pitchSlider->setColour (Slider::rotarySliderOutlineColourId, Colour(0xFF4FFF00));
                pitchSlider->setReverse(true);
                pitchSlider->setRotaryParameters(0.5*M_PI, 2.5*M_PI, false);
                pitchSlider->setTooltip("Pitch angle");
                pitchSlider->setTextValueSuffix(CharPointer_UTF8 ("\xc2\xb0"));
                
                addAndMakeVisible(gainSlider);
                slGainAttachmentArray.add(new SliderAttachment(*pVts,"gain" + String(i), *gainSlider));
                gainSlider->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
                gainSlider->setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
                gainSlider->setColour (Slider::rotarySliderOutlineColourId, Colour(0xFFD0011B));
                gainSlider->setReverse(false);
                gainSlider->setTooltip("Gain");
                gainSlider->setTextValueSuffix("dB");
                
                addAndMakeVisible(muteButton);
                muteButtonAttachmentArray.add(new ButtonAttachment(*pVts,"mute" + String(i), *muteButton));
                muteButton->setType(MuteSoloButton::Type::mute);
                
                addAndMakeVisible(soloButton);
                soloButtonAttachmentArray.add(new ButtonAttachment(*pVts,"solo" + String(i), *soloButton));
                soloButton->setType(MuteSoloButton::Type::solo);
            }
        }
        
        nChannels = nCh;
        
        setBounds(0, 0, 200, nChannels*(63));  // const int rotSliderHeight = 55; const int labelHeight = 15;
        
    }
    
    void updateColours() {
        for (int i = 0; i < nChannels; ++i)
        {
            colourChooserArray[i]->setColour(TextButton::buttonColourId, processor.elementColours[i]);
            colourChooserArray[i]->setColour(TextButton::textColourOffId, Colours::white.overlaidWith (processor.elementColours[i]).contrasting());
            sphereElementArray[i]->setColour(processor.elementColours[i]);
            sphereElementArray[i]->setTextColour(Colours::white.overlaidWith (processor.elementColours[i]).contrasting());
        }
        repaint();
    }
    
    void paint (Graphics& g) override {
    };
    
    void resized() override {
        
        Rectangle<int> bounds = getBounds();
        Rectangle<int> sliderRow;
        
        const int rotSliderSpacing = 10;
        const int rotSliderHeight = 55;
        const int rotSliderWidth = 40;
        
        for (int i = 0; i < nChannels; ++i)
        {
            sliderRow = bounds.removeFromTop(rotSliderHeight);
            colourChooserArray[i]->setBounds(sliderRow.removeFromLeft(22).reduced(0,18));
            sliderRow.removeFromLeft(5);
            slYawArray[i]->setBounds (sliderRow.removeFromLeft(rotSliderWidth));
            sliderRow.removeFromLeft(rotSliderSpacing);
            slPitchArray[i]->setBounds (sliderRow.removeFromLeft(rotSliderWidth));
            sliderRow.removeFromLeft(rotSliderSpacing);
            slGainArray[i]->setBounds (sliderRow.removeFromLeft(rotSliderWidth));
            sliderRow.removeFromLeft(rotSliderSpacing);
            
            sliderRow.reduce(0, 6);
            sliderRow.setWidth(18);
            soloButtonArray[i]->setBounds(sliderRow.removeFromTop(18));
            sliderRow.removeFromTop(5);
            muteButtonArray[i]->setBounds(sliderRow.removeFromTop(18));
            bounds.removeFromTop(8); //spacing
        }
        
        repaint();
    }
    OwnedArray<SpherePanner::Element> sphereElementArray;
    
private:
    MultiEncoderAudioProcessor& processor;
    SpherePanner& spherePanner;
    AudioProcessorValueTreeState* pVts;
    int nChannels;
    
    
    
    OwnedArray<ReverseSlider> slYawArray, slPitchArray, slGainArray;
    OwnedArray<MuteSoloButton> muteButtonArray, soloButtonArray;
    OwnedArray<ColourChangeButton> colourChooserArray;
    OwnedArray<SliderAttachment> slYawAttachmentArray;
    OwnedArray<SliderAttachment> slPitchAttachmentArray;
    OwnedArray<SliderAttachment> slGainAttachmentArray;
    OwnedArray<ButtonAttachment> muteButtonAttachmentArray;
    OwnedArray<ButtonAttachment> soloButtonAttachmentArray;
};
