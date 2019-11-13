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

#include "TitleBarPaths.h"
#include "../ambisonicTools.h"

#ifdef JUCE_OSC_H_INCLUDED
#include "../OSC/OSCStatus.h"
#endif

class AlertSymbol : public Component, public TooltipClient
{
public:
    AlertSymbol() : Component()
    {
        warningSign.loadPathFromData (WarningSignData, sizeof (WarningSignData));
        setBufferedToImage (true);
    }

    String getTooltip() override
    {
        return "Not enough channels available \n for your current setting.";
    }

    void paint (Graphics& g) override
    {
        warningSign.applyTransform (warningSign.getTransformToScaleToFit (getLocalBounds().toFloat(), true, Justification::centred));
        g.setColour (Colours::yellow);
        g.fillPath (warningSign);
    }

private:
    Path warningSign;
};

class IOWidget : public Component
{
public:
    IOWidget() : Component()
    {
        addChildComponent (alert);
        alert.setBounds (15, 15, 15, 15);
    }

    virtual const int getComponentSize() = 0;
    virtual void setMaxSize (int maxSize) = 0;

    void setBusTooSmall (bool isBusTooSmall)
    {
        busTooSmall = isBusTooSmall;
        alert.setVisible (isBusTooSmall);
    }

    bool isBusTooSmall ()
    {
        return busTooSmall;
    }

private:
    AlertSymbol alert;
    bool busTooSmall = false;
};

class  NoIOWidget :  public IOWidget
{
public:
    NoIOWidget() {}
    void setMaxSize (int maxSize) override { ignoreUnused (maxSize); }
    const int getComponentSize() override { return 0; }
};

class  BinauralIOWidget : public IOWidget
{
public:
    BinauralIOWidget() : IOWidget()
    {
        BinauralPath.loadPathFromData (BinauralPathData, sizeof (BinauralPathData));
        setBufferedToImage (true);
    }

    const int getComponentSize() override { return 30; }
    void setMaxSize (int maxSize) override { ignoreUnused (maxSize); }
    void paint (Graphics& g) override
    {
        BinauralPath.applyTransform (BinauralPath.getTransformToScaleToFit (0, 0, 30, 30, true,Justification::centred));
        g.setColour ((Colours::white).withMultipliedAlpha (0.5));
        g.fillPath (BinauralPath);

    }

private:
    Path BinauralPath;
};


template <int maxChannels, bool selectable = true>
class AudioChannelsIOWidget : public IOWidget, private ComboBox::Listener
{
public:
    AudioChannelsIOWidget() : IOWidget()
    {
        WaveformPath.loadPathFromData (WaveformPathData, sizeof (WaveformPathData));
        setBufferedToImage(true);

        if (selectable)
        {
            cbChannels.reset (new ComboBox());
            addAndMakeVisible (cbChannels.get());
            cbChannels->setJustificationType (Justification::centred);
            cbChannels->addSectionHeading ("Number of channels");
            cbChannels->addItem ("Auto", 1);
            for (int i = 1; i <= maxChannels; ++i)
                cbChannels->addItem (String (i), i + 1);
            cbChannels->setBounds (35, 8, 70, 15);
            cbChannels->addListener (this);
        }
    }

    const int getComponentSize() override { return selectable ? 110 : 75; }

    void updateDisplayTextIfNotSelectable()
    {
        if (availableChannels < channelSizeIfNotSelectable)
        {
            displayTextIfNotSelectable = String (channelSizeIfNotSelectable) + " (bus too small)";
            setBusTooSmall (true);
        }
        else
        {
            displayTextIfNotSelectable = String (channelSizeIfNotSelectable);
            setBusTooSmall (false);
        }
        repaint();
    }

    void checkIfBusIsTooSmall()
    {
        if (availableChannels < cbChannels->getSelectedId() - 1)
            setBusTooSmall (true);
        else
            setBusTooSmall (false);
    }

    void comboBoxChanged (ComboBox *comboBoxThatHasChanged) override
    {
        ignoreUnused (comboBoxThatHasChanged);
        checkIfBusIsTooSmall();
    }

    void setMaxSize (int maxPossibleNumberOfChannels) override
    {
        if (availableChannels != maxPossibleNumberOfChannels)
        {
            availableChannels = maxPossibleNumberOfChannels;
            if (selectable)
            {
                if (maxPossibleNumberOfChannels > 0) cbChannels->changeItemText (1, "Auto (" + String (maxPossibleNumberOfChannels) + ")");
                else cbChannels->changeItemText (1, "(Auto)");
                int currId = cbChannels->getSelectedId();
                if (currId == 0) currId = 1; //bad work around
                int i;
                for (i = 1; i <= maxPossibleNumberOfChannels; ++i)
                {
                    cbChannels->changeItemText (i + 1, String(i));
                }
                for (i = maxPossibleNumberOfChannels+1; i<=maxChannels; ++i)
                {
                    cbChannels->changeItemText (i + 1, String(i) + " (bus too small)");
                }
                checkIfBusIsTooSmall();

                cbChannels->setText (cbChannels->getItemText (cbChannels->indexOfItemId (currId)));
            }
            else
            {
                updateDisplayTextIfNotSelectable();
            }
        }
    }

    void setSizeIfUnselectable (int newSize)
    {
        if (! selectable && channelSizeIfNotSelectable != newSize)
        {
            channelSizeIfNotSelectable = newSize;
            updateDisplayTextIfNotSelectable();
        }
    }

    ComboBox* getChannelsCbPointer()
    {
        if (selectable) return cbChannels.get();
        return nullptr;
    }

    void paint (Graphics& g) override
    {
        WaveformPath.applyTransform(WaveformPath.getTransformToScaleToFit (0, 0, 30, 30, true, Justification::centred));
        g.setColour ((Colours::white).withMultipliedAlpha (0.5));
        g.fillPath (WaveformPath);

        if (!selectable)
        {
            g.setColour ((Colours::white).withMultipliedAlpha (0.5));
            g.setFont (getLookAndFeel().getTypefaceForFont (Font (12.0f, 1)));
            g.setFont (15.0f);
            g.drawFittedText (displayTextIfNotSelectable, 35, 0, 40, 30, Justification::centredLeft, 2);
        }
    }

private:
    std::unique_ptr<ComboBox> cbChannels;
    Path WaveformPath;
    int availableChannels {64};
    int channelSizeIfNotSelectable = maxChannels;
    String displayTextIfNotSelectable = String(maxChannels);
};

template <int order = 7, bool selectable = true>
class  AmbisonicIOWidget :  public IOWidget
{
public:
    AmbisonicIOWidget() : IOWidget()
    {
        AmbiLogoPath.loadPathFromData (AmbiLogoPathData, sizeof (AmbiLogoPathData));
        setBufferedToImage (true);

        if (selectable)
        {
            addAndMakeVisible (&cbOrder);
            cbOrder.setJustificationType (Justification::centred);
            cbOrder.setBounds (35, 15, 70, 15);
            updateMaxOrder();
        }
        else
        {
            displayTextIfNotSelectable = getOrderString (order) + " order";
        }

        addAndMakeVisible (&cbNormalization);
        cbNormalization.setJustificationType (Justification::centred);
        cbNormalization.addSectionHeading ("Normalization");
        cbNormalization.addItem ("N3D", 1);
        cbNormalization.addItem ("SN3D", 2);
        cbNormalization.setBounds (35, 0, 70, 15);
    };

    ~AmbisonicIOWidget() {};

    void setOrderIfUnselectable (int newOrder)
    {
        if (! selectable && orderIfNotSelectable != newOrder)
        {
            orderIfNotSelectable = newOrder;
            updateDisplayTextIfNotSelectable();
        }
    }

    void updateDisplayTextIfNotSelectable()
    {
        if (maxPossibleOrder < orderIfNotSelectable)
        {
            displayTextIfNotSelectable = getOrderString (orderIfNotSelectable) + " (bus too small)";
            setBusTooSmall (true);
        }
        else
        {
            displayTextIfNotSelectable = getOrderString (orderIfNotSelectable) + " order";
            setBusTooSmall (false);
        }
        repaint();
    }

    void updateMaxOrder()
    {
        const int previousIndex = cbOrder.getSelectedItemIndex();
        cbOrder.clear();
        cbOrder.addSectionHeading ("Ambisonic Order");
        cbOrder.addItem ("Auto", 1);
        for (int o = 0; o <= maxOrder; ++o)
            cbOrder.addItem (getOrderString(o), o + 2);

        cbOrder.setSelectedItemIndex (previousIndex);
    }

    void setMaxOrder (int newMaxOrder)
    {
        maxOrder = newMaxOrder;
        updateMaxOrder();
        const int savedMaxPossibleOrder = maxPossibleOrder;
        maxPossibleOrder = -1;
        setMaxSize (savedMaxPossibleOrder);
    }

    const int getComponentSize() override { return 110; }

    /** Sets the maximally available size of the processor for this Widget.
     */
    void setMaxSize (int newMaxPossibleOrder) override
    {
        if (maxPossibleOrder != jmin (newMaxPossibleOrder, maxOrder))
        {
            maxPossibleOrder = jmin (newMaxPossibleOrder, maxOrder);

            if (selectable)
            {
                if (maxPossibleOrder > -1)
                    cbOrder.changeItemText (1, "Auto (" + getOrderString (maxPossibleOrder) + ")");
                else
                    cbOrder.changeItemText (1, "(Auto)");

                int currId = cbOrder.getSelectedId();
                if (currId == 0) currId = 1; //bad work around

                for (int i = 1; i <= maxPossibleOrder; ++i)
                    cbOrder.changeItemText (i + 2, getOrderString (i));

                for (int i = maxPossibleOrder + 1; i<=maxOrder; ++i)
                    cbOrder.changeItemText (i + 2, getOrderString (i) + " (bus too small)");

                DBG (cbOrder.getItemText (cbOrder.indexOfItemId ((currId))));
                cbOrder.setText (cbOrder.getItemText (cbOrder.indexOfItemId ((currId))));
                if (currId - 2 > maxPossibleOrder)
                    setBusTooSmall (true);
                else
                    setBusTooSmall (false);
            }
            else
            {
                updateDisplayTextIfNotSelectable();
            }
        }
    }

    ComboBox* getNormCbPointer() { return &cbNormalization; }
    ComboBox* getOrderCbPointer()
    {
        if (! selectable)
            // There's no Ambisonic Order ComboBox, when order is not selectable!
            jassertfalse;

        return &cbOrder;
    }

    void paint (Graphics& g) override
    {
        AmbiLogoPath.applyTransform (AmbiLogoPath.getTransformToScaleToFit (0, 0, 30, 30, true, Justification::centred));
        g.setColour ((Colours::white).withMultipliedAlpha (0.5));
        g.fillPath (AmbiLogoPath);

        if (!selectable)
        {
            g.setColour ((Colours::white).withMultipliedAlpha (0.5));
            g.setFont (getLookAndFeel().getTypefaceForFont (Font (12.0f, 1)));
            g.setFont (15.0f);
            g.drawFittedText (displayTextIfNotSelectable, 35, 15, 55, 15, Justification::centred, 1);
        }
    };

private:
    ComboBox cbNormalization, cbOrder;
    Path AmbiLogoPath;
    int maxOrder = order;
    int orderIfNotSelectable = order;
    int maxPossibleOrder = -1;
    String displayTextIfNotSelectable;
};

class  DirectivityIOWidget :  public IOWidget
{
public:
    DirectivityIOWidget() : IOWidget()
    {
        DirectivityPath.loadPathFromData (DirectivityPathData, sizeof (DirectivityPathData));
        setBufferedToImage (true);
        orderStrings[0] = String ("0th");
        orderStrings[1] = String ("1st");
        orderStrings[2] = String ("2nd");
        orderStrings[3] = String ("3rd");
        orderStrings[4] = String ("4th");
        orderStrings[5] = String ("5th");
        orderStrings[6] = String ("6th");
        orderStrings[7] = String ("7th");

        addAndMakeVisible (&cbOrder);
        cbOrder.setJustificationType (Justification::centred);
        cbOrder.addSectionHeading ("Directivity Order");
        cbOrder.addItem ("Auto", 1);
        cbOrder.addItem ("0th", 2);
        cbOrder.addItem ("1st", 3);
        cbOrder.addItem ("2nd", 4);
        cbOrder.addItem ("3rd", 5);
        cbOrder.addItem ("4th", 6);
        cbOrder.addItem ("5th", 7);
        cbOrder.addItem ("6th", 8);
        cbOrder.addItem ("7th", 9);
        cbOrder.setBounds (35, 15, 70, 15);

        addAndMakeVisible (&cbNormalization);
        cbNormalization.setJustificationType (Justification::centred);
        cbNormalization.addSectionHeading ("Normalization");
        cbNormalization.addItem ("N3D", 1);
        cbNormalization.addItem ("SN3D", 2);
        cbNormalization.setBounds (35, 0, 70, 15);
    }

    const int getComponentSize() override { return 110; }

    void setMaxSize (int maxPossibleOrder) override
    {
        if (maxPossibleOrder > -1) cbOrder.changeItemText( 1, "Auto (" + orderStrings[maxPossibleOrder] + ")");
        else cbOrder.changeItemText (1, "(Auto)");
        int currId = cbOrder.getSelectedId();
        if (currId == 0) currId = 1; //bad work around
        int i;
        for (i = 1; i <= maxPossibleOrder; ++i)
        {
            cbOrder.changeItemText (i + 2, orderStrings[i]);
        }
        for (i = maxPossibleOrder + 1; i <= 7; ++i)
        {
            cbOrder.changeItemText (i + 2, orderStrings[i] + " (bus too small)");
        }
        cbOrder.setText (cbOrder.getItemText (cbOrder.indexOfItemId (currId)));
        if (currId - 2> maxPossibleOrder)
            setBusTooSmall (true);
        else
            setBusTooSmall (false);

    }

    ComboBox* getNormCbPointer() { return &cbNormalization; }
    ComboBox* getOrderCbPointer() { return &cbOrder; }

    void paint (Graphics& g) override
    {
        DirectivityPath.applyTransform (DirectivityPath.getTransformToScaleToFit (0, 0, 30, 30, true, Justification::centred));
        g.setColour ((Colours::white).withMultipliedAlpha (0.5));
        g.fillPath (DirectivityPath);
    }

private:
    String orderStrings[8];
    ComboBox cbNormalization, cbOrder;
    Path DirectivityPath;
};

// ======================================================== TITLEBAR =========================
template <class Tin, class Tout>
class TitleBar :  public Component
{
public:
    TitleBar() : Component()
    {
        addAndMakeVisible(&inputWidget);
        addAndMakeVisible(&outputWidget);
    }

    Tin* getInputWidgetPtr() { return &inputWidget; }
    Tout* getOutputWidgetPtr() { return &outputWidget; }


    void setTitle (String newBoldText, String newRegularText)
    {
        boldText = newBoldText;
        regularText = newRegularText;
    }

    void setFont (Typeface::Ptr newBoldFont, Typeface::Ptr newRegularFont)
    {
        boldFont = newBoldFont;
        regularFont = newRegularFont;
    }

    void resized () override
    {
        const int leftWidth = inputWidget.getComponentSize();
        const int rightWidth = outputWidget.getComponentSize();

        inputWidget.setBounds (getLocalBounds().removeFromLeft (leftWidth).reduced (0, 15));
        outputWidget.setBounds (getLocalBounds().removeFromRight (rightWidth).reduced (0, 15));
    }

    void setMaxSize (std::pair<int, int> inOutSizes)
    {
        inputWidget.setMaxSize (inOutSizes.first);
        outputWidget.setMaxSize (inOutSizes.second);
    }

    void paint (Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        const float centreX = bounds.getX() + bounds.getWidth() * 0.5f;
        const float centreY = bounds.getY() + bounds.getHeight() * 0.5f;
        const float boldHeight = 25.f;
        const float regularHeight = 25.f;
        const int leftWidth = inputWidget.getComponentSize();
        const int rightWidth = outputWidget.getComponentSize();

        boldFont.setHeight (boldHeight);
        regularFont.setHeight (regularHeight);

        const float boldWidth = boldFont.getStringWidth (boldText);
        const float regularWidth = regularFont.getStringWidth (regularText);

        Rectangle<float> textArea (0, 0, boldWidth + regularWidth, jmax (boldHeight, regularHeight));
        textArea.setCentre (centreX,centreY);

        if (textArea.getX() < leftWidth) textArea.setX(leftWidth);
        if (textArea.getRight() > bounds.getRight() - rightWidth) textArea.setRight (bounds.getRight() - rightWidth);


        g.setColour (Colours::white);
        g.setFont (boldFont);
        g.drawFittedText (boldText, textArea.removeFromLeft (boldWidth).toNearestInt(), Justification::bottom, 1);
        g.setFont (regularFont);
        g.drawFittedText (regularText, textArea.toNearestInt(), Justification::bottom, 1);

        g.setColour ((Colours::white).withMultipliedAlpha (0.5));
        g.drawLine (bounds.getX(),bounds.getY() + bounds.getHeight() - 4, bounds.getX() + bounds.getWidth(), bounds.getY()+bounds.getHeight() - 4);
    }

private:
    Tin inputWidget;
    Tout outputWidget;
    Font boldFont = Font (25.f);
    Font regularFont = Font (25.f);
    juce::String boldText = "Bold";
    juce::String regularText = "Regular";
};


class IEMLogo : public Component
{
public:
    IEMLogo() : Component()
    {
        IEMPath.loadPathFromData (IEMpathData, sizeof (IEMpathData));
        url = URL("https://plugins.iem.at/");
    }

    void paint (Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        bounds.removeFromBottom (3);
        bounds.removeFromLeft (1);
        IEMPath.applyTransform(IEMPath.getTransformToScaleToFit (bounds.reduced (2, 2).toFloat(), true, Justification::bottomLeft));

        if (isMouseOver())
        {
            g.setColour (Colour::fromRGB(52, 88, 165));
            g.fillAll();
        }

        g.setColour (isMouseOver() ? Colour::fromRGB (249, 226, 45) : Colours::white.withMultipliedAlpha (0.5));
        g.fillPath (IEMPath);
    }

    void mouseEnter (const MouseEvent &event) override
    {
        ignoreUnused (event);
        setMouseCursor (MouseCursor (MouseCursor::PointingHandCursor));
        repaint();
    }

    void mouseExit (const MouseEvent &event) override
    {
        ignoreUnused (event);
        setMouseCursor (MouseCursor (MouseCursor::NormalCursor));
        repaint();
    }

    void mouseUp (const MouseEvent &event) override
    {
        ignoreUnused (event);
        if (url.isWellFormed())
            url.launchInDefaultBrowser();
    }

private:
    Path IEMPath;
    URL url;
};

class  Footer :  public Component
{
public:
    Footer() : Component()
    {
        addAndMakeVisible(&iemLogo);
    }

    void paint (Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        g.setColour (Colours::white.withAlpha (0.5f));
        g.setFont (getLookAndFeel().getTypefaceForFont (Font (12.0f, 0)));
        g.setFont (14.0f);
        String versionString = "v";

#if JUCE_DEBUG
        versionString = "DEBUG - v";
#endif
        versionString.append(JucePlugin_VersionString, 6);

        g.drawText (versionString, 0, 0, bounds.getWidth() - 8, bounds.getHeight() - 2, Justification::bottomRight);
    }

    void resized () override
    {
        iemLogo.setBounds (0, 0, 40, getHeight());
    }

private:
    IEMLogo iemLogo;
};

#ifdef JUCE_OSC_H_INCLUDED
class  OSCFooter :  public Component
{
public:
    OSCFooter (OSCParameterInterface& oscInterface) : oscStatus (oscInterface)
    {
        addAndMakeVisible (footer);
        addAndMakeVisible (oscStatus);
    }

    void resized () override
    {
        auto bounds = getLocalBounds();
        footer.setBounds (bounds);

        bounds.removeFromBottom (2);
        bounds = bounds.removeFromBottom (16);
        bounds.removeFromLeft (50);
        oscStatus.setBounds (bounds);
    }

private:
    OSCStatus oscStatus;
    Footer footer;
};
#endif
