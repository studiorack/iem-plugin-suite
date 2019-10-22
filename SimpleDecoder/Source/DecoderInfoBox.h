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
#include "ReferenceCountedDecoder.h"
#include "ambisonicTools.h"

//==============================================================================
/*
 */
class DecoderInfoBox : public Component
{
    static constexpr int attributeHeight = 12;
    static constexpr int valueHeight = 17;
    static constexpr int spacing = 5;

public:
    DecoderInfoBox (AudioProcessorValueTreeState& parameters)
    {
        addAndMakeVisible (cbWeights);
        cbWeights.setJustificationType (Justification::centred);
        cbWeights.addItemList ({"basic", "maxrE", "inPhase"}, 1);
        cbWeightsAttachment.reset (new AudioProcessorValueTreeState::ComboBoxAttachment (parameters, "weights", cbWeights));
    }

    ~DecoderInfoBox()
    {
    }

    void setDecoderConfig (ReferenceCountedDecoder::Ptr newDecoderConfig)
    {
        decoder = newDecoderConfig;
        if (decoder == nullptr)
            cbWeights.setVisible (false);
        else
            cbWeights.setVisible (true);

        resized();
        repaint();
    }

    const int getMaxAttributeWidth()
    {
        auto font = Font (getLookAndFeel().getTypefaceForFont (Font (12.0f, 2)));
        font.setHeight (attributeHeight);
        return font.getStringWidth ("LOUDSPEAKERS:");
    }

    void resized() override
    {
        auto retainedDecoder = decoder;
        Rectangle<int> bounds = getLocalBounds();
        arr.clear();

        if (retainedDecoder != nullptr)
        {
            const auto maxAttWidth = getMaxAttributeWidth();
            const int valueStart = maxAttWidth + spacing;
            const int valueWidth = jmax (bounds.getWidth() - valueStart, 0);

            auto font = Font (getLookAndFeel().getTypefaceForFont (Font (12.0f, 2)));
            font.setHeight (valueHeight);

            arr.addFittedText (font, retainedDecoder->getDescription(), valueStart, valueHeight + 1, valueWidth, 3 * valueHeight, Justification::topLeft, 4, 0.8f);

            const int descriptionEnd = arr.getBoundingBox (jmax (0, arr.getNumGlyphs() - 1), 1, true).getBottom();

            cbWeights.setBounds (valueStart, descriptionEnd + 2 * valueHeight + 2, 80, valueHeight);
        }
    }

    void paint (Graphics& g) override
    {
        auto retainedDecoder = decoder;
        Rectangle<int> bounds = getLocalBounds();
        const int width = bounds.getWidth();

        g.setColour (Colours::white);
        g.setFont (getLookAndFeel().getTypefaceForFont (Font (12.0f, 2))); // regular font

        if (retainedDecoder == nullptr)
        {
            g.setFont (valueHeight);
            g.drawText ("No configuration loaded.", 20, 1, width, valueHeight, Justification::bottomLeft);
            g.drawMultiLineText (errorText, 20, 30, width - 20);
        }
        else
        {
            g.setFont (attributeHeight);

            const int maxAttributeWidth = getMaxAttributeWidth();
            const int resStart = maxAttributeWidth + spacing;
            const int resWidth = jmax(width - resStart, 0);

            g.drawText ("NAME:", 0, 0, maxAttributeWidth, valueHeight, Justification::bottomRight);
            g.drawText ("DESCRIPTION:", 0, valueHeight, maxAttributeWidth, valueHeight, Justification::bottomRight);

            g.setFont (getLookAndFeel().getTypefaceForFont (Font (12.0f, 1))); // bold font
            g.setFont (valueHeight);
            g.drawText (retainedDecoder->getName(), resStart, 1, resWidth, valueHeight, Justification::bottomLeft);

            String descriptionText = retainedDecoder->getDescription();

            arr.draw (g);
            const int descEnd = arr.getBoundingBox (jmax (0, arr.getNumGlyphs() - 1), 1, true).getBottom();

            g.setFont (getLookAndFeel().getTypefaceForFont (Font (12.0f, 2))); // regular font
            g.setFont (attributeHeight);

            g.drawText ("ORDER:", 0, descEnd, maxAttributeWidth, valueHeight, Justification::bottomRight);
            g.drawText ("LOUDSPEAKERS:", 0, descEnd + valueHeight, maxAttributeWidth, valueHeight, Justification::bottomRight);
            g.drawText ("WEIGHTS:", 0, descEnd + 2 * valueHeight, maxAttributeWidth, valueHeight, Justification::bottomRight);

            g.setFont (valueHeight);
            g.drawText (getOrderString (retainedDecoder->getOrder()), resStart, descEnd + 1, resWidth, valueHeight, Justification::bottomLeft);
            g.drawText (String (retainedDecoder->getMatrix().getNumRows()), resStart, descEnd + valueHeight + 1, resWidth, valueHeight, Justification::bottomLeft);
        }

    }

    void setErrorMessage (String errorMessage)
    {
        errorText = errorMessage;
        repaint();
    }

private:
    ComboBox cbWeights;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> cbWeightsAttachment;

    String errorText {""};
    ReferenceCountedDecoder::Ptr decoder {nullptr};

    GlyphArrangement arr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecoderInfoBox)
};
