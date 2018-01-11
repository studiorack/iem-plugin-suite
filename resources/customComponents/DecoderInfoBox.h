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
#include "../ReferenceCountedDecoder.h"
#include "../ambisonicTools.h"

//==============================================================================
/*
 */
class DecoderInfoBox    : public Component
{
public:
    DecoderInfoBox()
    {
    }
    
    ~DecoderInfoBox()
    {
    }
    
    void setDecoderConfig (ReferenceCountedDecoder::Ptr newDecoderConfig)
    {
        decoder = newDecoderConfig;
        repaint();
    }
    
    
    void paint (Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        const int width = bounds.getWidth();
        const int fontHeight = 17;
        const int baselineOffset = 15;
        const int attHeight = 12;
        
        g.setColour(Colours::white);
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 2)));
        g.setFont(attHeight);
        
        if (decoder != nullptr)
        {
            Font font = g.getCurrentFont();
            font.setHeight(attHeight);
            g.setFont(font);
            const int maxAttributeWidth = font.getStringWidth("LOUDSPEAKERS:");
            const int resStart = maxAttributeWidth + 5;
            const int resWidth = jmax(width - resStart, 0);
            

            
            g.drawText("NAME:", 0, 0, maxAttributeWidth, fontHeight, Justification::bottomRight);
            g.drawText("DESCRIPTION:", 0, fontHeight, maxAttributeWidth, fontHeight, Justification::bottomRight);
            
            
            g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 1)));
            g.setFont(fontHeight);
            g.drawText(decoder->getName(), resStart, 1, resWidth, fontHeight, Justification::bottomLeft);
            
            String descriptionText = decoder->getDescription();
            font.setHeight(fontHeight);
            GlyphArrangement arr;
            arr.addJustifiedText (font, descriptionText, resStart, fontHeight + baselineOffset, resWidth, Justification::left);
            arr.draw (g);
            const int descEnd = arr.getBoundingBox(arr.getNumGlyphs()-1, 1, true).getBottom();
            
            font.setHeight(attHeight);
            g.setFont(font);
            g.drawText("ORDER:", 0, descEnd, maxAttributeWidth, fontHeight, Justification::bottomRight);
            g.drawText("LOUDSPEAKERS:", 0, descEnd + fontHeight, maxAttributeWidth, fontHeight, Justification::bottomRight);
            g.drawText("WEIGHTS:", 0, descEnd + 2 * fontHeight, maxAttributeWidth, fontHeight, Justification::bottomRight);
            
            font.setHeight(fontHeight);
            g.setFont(font);
            g.drawText(getOrderString(decoder->getOrder()), resStart, descEnd + 1, resWidth, fontHeight, Justification::bottomLeft);
            g.drawText(String(decoder->getMatrix()->rows()), resStart, descEnd + fontHeight + 1, resWidth, fontHeight, Justification::bottomLeft);
            g.drawText(decoder->getWeightsString(), resStart, descEnd + 2 * fontHeight + 1, resWidth, fontHeight, Justification::bottomLeft);
        }
        else
        {
            g.setFont(fontHeight);
            g.drawText("No configuration loaded.", 20, 1, width, fontHeight, Justification::bottomLeft);
            g.drawMultiLineText(errorText, 20, 30, width - 20);
        }
    }
    
    void setErrorMessage(String errorMessage)
    {
        errorText = errorMessage;
        repaint();
    }
    
    void resized() override
    {
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecoderInfoBox)
    String errorText {""};
    ReferenceCountedDecoder::Ptr decoder {nullptr};
};




