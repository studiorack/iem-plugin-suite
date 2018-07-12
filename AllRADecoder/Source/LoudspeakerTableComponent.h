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


//==============================================================================
/*
*/
class LoudspeakerTableComponent : public Component, public TableListBoxModel
{

public:
    LoudspeakerTableComponent(ValueTree& loudspeakers, LoudspeakerVisualizer& visualizer, EnergyDistributionVisualizer& energyVis, UndoManager& undoM, AllRADecoderAudioProcessor& audioProcessor) : data(loudspeakers), undoManager(undoM), processor(audioProcessor), lspVisualizer(visualizer), engVisualizer(energyVis)
    {
        typeFace = getLookAndFeel().getTypefaceForFont(12);

        addAndMakeVisible (table);
        table.setModel (this);
        table.setColour (ListBox::outlineColourId, Colours::grey);
        table.setOutlineThickness (1);

        table.getHeader().addColumn(getAttributeNameForColumnId(1), 1, 23, 20, 25, TableHeaderComponent::notSortable);
        table.getHeader().addColumn(getAttributeNameForColumnId(2), 2, 55);
        table.getHeader().addColumn(getAttributeNameForColumnId(3), 3, 55);
        table.getHeader().addColumn(getAttributeNameForColumnId(4), 4, 45);
        table.getHeader().addColumn(getAttributeNameForColumnId(5), 5, 50);
        table.getHeader().addColumn(getAttributeNameForColumnId(6), 6, 50);
        table.getHeader().addColumn(getAttributeNameForColumnId(7), 7, 33);
        table.getHeader().addColumn(getAttributeNameForColumnId(9), 9, 40, 40, 40, TableHeaderComponent::notSortable);
        table.getHeader().addColumn(getAttributeNameForColumnId(8), 8, 60, 60, 60, TableHeaderComponent::notSortable);

        table.setHeaderHeight(23);
        table.setMultipleSelectionEnabled (false);
        table.setColour (ListBox::outlineColourId, Colours::steelblue);
        table.setOutlineThickness (0);
    }

    ~LoudspeakerTableComponent()
    {
    }

    void playNoise (const int row)
    {
        const auto& modifiers = ModifierKeys::getCurrentModifiers();
        if (modifiers.isAltDown())
        {
            const float azimuth = (float) data.getChild (row).getProperty ("Azimuth");
            const float elevation = (float) data.getChild (row).getProperty ("Elevation");
            processor.playAmbisonicNoiseBurst (azimuth, elevation);
        }
        else
        {
            if (! data.getChild(row).getProperty("Imaginary"))
            {
                const int ch = (int) data.getChild(row).getProperty("Channel");
                processor.playNoiseBurst (ch);
            }
        }
    }

    void selectedRowsChanged (int lastRowSelected) override
    {
        lspVisualizer.setActiveSpeakerIndex(lastRowSelected);
        engVisualizer.setActiveSpeakerIndex(lastRowSelected);
    }

    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        const Colour alternateColour (getLookAndFeel().findColour (ListBox::backgroundColourId)
                                      .interpolatedWith (getLookAndFeel().findColour (ListBox::textColourId), 0.03f));
        if (rowIsSelected)
            g.fillAll (Colours::limegreen.withMultipliedAlpha(0.3f));
        else if (rowNumber % 2)
            g.fillAll (alternateColour);
    }

    int getNumRows() override
    {
        return data.getNumChildren();
    }

    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour (getLookAndFeel().findColour (ListBox::textColourId));
        g.setFont (typeFace);

        if (columnId == 1)
            g.drawText (String(rowNumber + 1), 2, 0, width - 4, height, Justification::centred, true);
        else
            g.drawText (getText(columnId, rowNumber), 2, 0, width - 4, height, Justification::centred, true);

    }

    // This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
    // to change the sort order.
    void updateContent()
    {
        table.updateContent();
    }

    void sortOrderChanged (int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            data.sort(sorter, nullptr, true);

            table.updateContent();
        }
    }

    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                        Component* existingComponentToUpdate) override
    {
        if (columnId == 1) // The ID and Length columns do not have a custom component
        {
            jassert (existingComponentToUpdate == nullptr);
            return nullptr;
        }

//        if (columnId == 5) // For the ratings column, we return the custom combobox component
//        {
//            RatingColumnCustomComponent* ratingsBox = static_cast<RatingColumnCustomComponent*> (existingComponentToUpdate);
//
//            // If an existing component is being passed-in for updating, we'll re-use it, but
//            // if not, we'll have to create one.
//            if (ratingsBox == nullptr)
//                ratingsBox = new RatingColumnCustomComponent (*this);
//
//            ratingsBox->setRowAndColumn (rowNumber, columnId);
//            return ratingsBox;
//        }

        else if (columnId == 6) // Imaginary
        {
            ImaginaryButton* imaginaryButton = static_cast<ImaginaryButton*> (existingComponentToUpdate);
            if (imaginaryButton == nullptr)
                imaginaryButton = new ImaginaryButton (*this);

            imaginaryButton->setRowAndColumn (rowNumber, columnId);
            return imaginaryButton;
        }

        else if (columnId == 8) // Remove
        {
            RemoveButton* removeButton = static_cast<RemoveButton*> (existingComponentToUpdate);
            if (removeButton == nullptr)
                removeButton = new RemoveButton (*this);

            removeButton->setRowAndColumn (rowNumber, columnId);
            return removeButton;
        }
        else if (columnId == 9) // Noise
        {
            NoiseButton* noiseButton = static_cast<NoiseButton*> (existingComponentToUpdate);
            if (noiseButton == nullptr)
                noiseButton = new NoiseButton (*this);

            noiseButton->setRowAndColumn (rowNumber, columnId);
            noiseButton->setTooltip ("Sends a short noise burst to that loudspeaker. \n Alt+click: Encodes a noise burst to the loudspeaker's position and decodes it with the current decoder.");
            noiseButton->setEnabled(! data.getChild(rowNumber).getProperty("Imaginary"));
            return noiseButton;
        }


        // The other columns are editable text columns, for which we use the custom Label component
        EditableTextCustomComponent* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        // same as above...
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent (*this);

        textLabel->setRowAndColumn (rowNumber, columnId);
        return textLabel;
    }

    String getAttributeNameForColumnId (const int columnId) const
    {
        switch (columnId) {
            case 1: return "ID"; break;
            case 2: return "Azimuth"; break;
            case 3: return "Elevation"; break;
            case 4: return "Radius"; break;
            case 5: return "Channel"; break;
            case 6: return "Imaginary";  break;
            case 7: return "Gain"; break;
            case 8: return "Remove"; break;
            case 9: return "Noise"; break;
            default: return ""; break;
        }
    }

    int getColumnAutoSizeWidth (int columnId) override
    {
        int widest = 32;

        for (int i = getNumRows(); --i >= 0;)
        {
            //TODO
        }

        return widest + 8;
    }

    void resized() override
    {
        table.setBounds(getLocalBounds());
    }

    String getText (const int columnId, const int rowNumber) const
    {
        if (columnId == 5 && data.getChild(rowNumber).getProperty(getAttributeNameForColumnId(columnId)).isInt())
        {
            const int value = data.getChild(rowNumber).getProperty(getAttributeNameForColumnId(columnId));
            return String(value);
        }
        else if (data.getChild(rowNumber).getProperty(getAttributeNameForColumnId(columnId)).isDouble())
        {
            const float value = data.getChild(rowNumber).getProperty(getAttributeNameForColumnId(columnId));
            String ret = String(value, 0);
            if (columnId == 2 || columnId == 3)
                ret = ret + String(CharPointer_UTF8 (R"(°)"));
            return ret;
        }
        else return("NaN");
    }

    void setFloat (const int columnId, const int rowNumber, const float newValue)
    {
        undoManager.beginNewTransaction();
        data.getChild(rowNumber).setProperty(getAttributeNameForColumnId(columnId), newValue, &undoManager);
    }

    void setInt (const int columnId, const int rowNumber, const int newValue)
    {
        undoManager.beginNewTransaction();
        data.getChild(rowNumber).setProperty(getAttributeNameForColumnId(columnId), newValue, &undoManager);
    }

    void setBool (const int columnId, const int rowNumber, const bool newValue)
    {
        undoManager.beginNewTransaction();
        data.getChild(rowNumber).setProperty(getAttributeNameForColumnId(columnId), newValue, &undoManager);
    }

    bool getBool (const int columnId, const int rowNumber)
    {
        return data.getChild(rowNumber).getProperty(getAttributeNameForColumnId(columnId));
    }

private:
    TableListBox table;     // the table component itself
    Typeface::Ptr typeFace;
    ValueTree& data;
    UndoManager& undoManager;

    AllRADecoderAudioProcessor& processor;
    LoudspeakerVisualizer& lspVisualizer;
    EnergyDistributionVisualizer& engVisualizer;

    class EditableTextCustomComponent  : public Label
    {
    public:
        EditableTextCustomComponent (LoudspeakerTableComponent& td)  : owner (td)
        {
            setEditable (false, true, false);
            setJustificationType(Justification::centred);
        }

        void mouseDown (const MouseEvent& event) override
        {
            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);
            Label::mouseDown (event);
        }

        void textWasEdited() override
        {
            if (columnId == 5)
                owner.setInt (columnId, row, getText().getIntValue());
            else
                owner.setFloat (columnId, row, getText().getFloatValue());
        }

        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText(columnId, row), dontSendNotification);
        }

        void paint (Graphics& g) override
        {
            if (! isBeingEdited())
            {
                const float alpha = isEnabled() ? 1.0f : 0.5f;

                if ((columnId == 4 || columnId == 7) && ! owner.data.getChild(row).getProperty("Imaginary"))
                    g.setColour (Colours::white.withMultipliedAlpha(0.4f));
                else
                {
                    if (columnId == 5 && owner.data.getChild(row).getProperty("Imaginary"))
                        g.setColour (Colours::white.withMultipliedAlpha(0.4f));
                    else
                        g.setColour (Colours::white);
                }


                g.setFont (getLookAndFeel().getTypefaceForFont(Font(12.0f)));
                g.setFont (13.f);

                Rectangle<int> textArea (getBorderSize().subtractedFrom (getLocalBounds()));

                g.drawFittedText (getText(), textArea, getJustificationType(),
                                  jmax (1, (int) (textArea.getHeight() / 12.0f)),
                                  getMinimumHorizontalScale());

                g.setColour (findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
            }
        }

    private:
        LoudspeakerTableComponent& owner;
        int row, columnId;
        Colour textColour;
    };

    class RemoveButton  : public TextButton
    {
    public:
        RemoveButton (LoudspeakerTableComponent& td)  : owner (td)
        {
            setButtonText("Remove");
            setColour(TextButton::buttonColourId, Colours::orangered);
            onClick = [this](){ owner.undoManager.beginNewTransaction(); owner.data.removeChild(owner.data.getChild(row), &owner.undoManager);};
        }

        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
        }

        void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
        {
            LookAndFeel& lf = getLookAndFeel();

            Rectangle<float> buttonArea(0.0f, 0.0f, getWidth(), getHeight());
            buttonArea.reduce(2.0f, 2.0f);

            g.setColour(findColour(TextButton::buttonColourId).withMultipliedAlpha(isButtonDown ? 1.0f : isMouseOverButton ? 0.7f : 0.5f));
            if (isButtonDown)
                buttonArea.reduce(0.8f, 0.8f);
            else if (isMouseOverButton)
                buttonArea.reduce(0.4f, 0.4f);

            g.drawRoundedRectangle(buttonArea, 2.0f, 1.0f);

            buttonArea.reduce(1.5f, 1.5f);
            g.setColour(findColour(TextButton::buttonColourId).withMultipliedAlpha(isButtonDown ? 1.0f : isMouseOverButton ? 0.5f : 0.2f));

            g.fillRoundedRectangle(buttonArea, 2.0f);

            lf.drawButtonText (g, *this, isMouseOverButton, isButtonDown);
        }

    private:
        LoudspeakerTableComponent& owner;
        int row, columnId;
    };

    class NoiseButton  : public TextButton
    {
    public:
        NoiseButton (LoudspeakerTableComponent& td)  : owner (td)
        {
            setButtonText("Noise");
            setColour(TextButton::buttonColourId, Colours::green);
            onClick = [this](){ owner.playNoise(row); };
        }

        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            column = newColumn;
        }

        void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
        {
            LookAndFeel& lf = getLookAndFeel();

            Rectangle<float> buttonArea(0.0f, 0.0f, getWidth(), getHeight());
            buttonArea.reduce(2.0f, 2.0f);

            g.setColour(findColour(TextButton::buttonColourId).withMultipliedAlpha(isButtonDown ? 1.0f : isMouseOverButton ? 0.7f : 0.5f));
            if (isButtonDown)
                buttonArea.reduce(0.8f, 0.8f);
            else if (isMouseOverButton)
                buttonArea.reduce(0.4f, 0.4f);

            g.drawRoundedRectangle(buttonArea, 2.0f, 1.0f);

            buttonArea.reduce(1.5f, 1.5f);
            g.setColour(findColour(TextButton::buttonColourId).withMultipliedAlpha(isButtonDown ? 1.0f : isMouseOverButton ? 0.5f : 0.2f));

            g.fillRoundedRectangle(buttonArea, 2.0f);

            lf.drawButtonText (g, *this, isMouseOverButton, isButtonDown);
        }

    private:
        LoudspeakerTableComponent& owner;
        int row, column;
    };


    class ImaginaryButton  : public Component
    {
    public:
        ImaginaryButton (LoudspeakerTableComponent& td)  : owner (td)
        {
            addAndMakeVisible(button);
            button.setButtonText("");
            button.setColour(ToggleButton::tickColourId, Colours::orange);
            button.onClick = [this](){
                owner.setBool(columnId, row, button.getToggleState());
                owner.repaint();
            };
        }

        void mouseDown (const MouseEvent& event) override
        {
            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);
        }

        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            button.setToggleState(owner.getBool(columnId, row), dontSendNotification);
        }

        void resized() override
        {
            Rectangle<int> bounds = getLocalBounds();
            const int height = bounds.getHeight();
            button.setBounds(bounds.reduced((bounds.getWidth() - height)/2, 0));
        }

    private:
        LoudspeakerTableComponent& owner;
        int row, columnId;
        ToggleButton button;
    };

    class DataSorter
    {
    public:
        DataSorter (const String& attributeToSortBy, bool forwards)
        : attributeToSort (attributeToSortBy),
        direction (forwards ? 1 : -1)
        {
        }

        int compareElements (const ValueTree& first, const ValueTree& second) const
        {
            int result = first.getProperty(attributeToSort).toString()
            .compareNatural (second.getProperty(attributeToSort).toString());
            return direction * result;
        }

    private:
        String attributeToSort;
        int direction;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoudspeakerTableComponent)
};
