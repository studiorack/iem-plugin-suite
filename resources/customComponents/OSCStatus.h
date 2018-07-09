/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
 http://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include "../../StereoEncoder/JuceLibraryCode/JuceHeader.h"
#include "SimpleLabel.h"

class OSCDialogWindow  : public Component, private Timer
{
public:
    OSCDialogWindow (OSCReceiverPlus& oscReceiver) : receiver (oscReceiver)
    {
        isConnected = oscReceiver.isConnected();
        previousPort = receiver.getPortNumber();

        addAndMakeVisible (headline);
        headline.setText ("OSC Receiver Port Number", false, Justification::centred);

        addAndMakeVisible (lbPort);
        const int port = receiver.getPortNumber();
        lbPort.setText (port == -1 ? "none" : String (port), NotificationType::dontSendNotification);
        lbPort.setEditable (true);
        lbPort.setJustificationType (Justification::centred);

        addAndMakeVisible (tbConnect);
        tbConnect.setButtonText (isConnected ? "DISCONNECT" : "CONNECT");
        tbConnect.setColour(TextButton::buttonColourId, isConnected ? Colours::orangered : Colours::limegreen);
        tbConnect.onClick =  [this] () { checkPortAndConnect(); };

        startTimer (500);
    }

    ~OSCDialogWindow()
    {
    }

    void timerCallback() override
    {
        bool shouldBeConnected = receiver.isConnected();
        if (isConnected != shouldBeConnected)
        {
            isConnected = shouldBeConnected;
            tbConnect.setButtonText (isConnected ? "DISCONNECT" : "CONNECT");
            tbConnect.setColour(TextButton::buttonColourId, isConnected ? Colours::orangered : Colours::limegreen);
            repaint();
        }
    }

    void paint (Graphics& g) override
    {
    }

    void checkPortAndConnect()
    {
        if (receiver.isConnected())
        {
            receiver.disconnect();
        }
        else
        {
            if (lbPort.getText() == "none" || lbPort.getText() == "off")
            {
                receiver.connect (-1);
                lbPort.setText ("none", NotificationType::dontSendNotification);
            }

            auto val = lbPort.getTextValue();
            int v = val.getValue();

            if (v == -1 || (v > 1000 && v < 15000))
            {
                if (! receiver.connect (v))
                {
                    AlertWindow alert ("Connection could ne be established!", "Make sure the desired port is available and not already occupied by other clients.", AlertWindow::NoIcon);
                    alert.setLookAndFeel (&getLookAndFeel());
                    alert.addButton ("OK", 1, KeyPress (KeyPress::returnKey, 0, 0));
                    alert.runModalLoop();
                }
            }
            else
            {
                lbPort.setText (previousPort == -1 ? "none" : String (previousPort), NotificationType::dontSendNotification);
            }
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        headline.setBounds (bounds.removeFromTop(12));

        bounds.removeFromTop (4);

        auto row = bounds.removeFromTop (20);

        lbPort.setBounds (row.removeFromLeft (50));

        row.removeFromLeft (8);
        tbConnect.setBounds(row);
    }

private:
    OSCReceiverPlus& receiver;
    bool isConnected = false;
    int previousPort = -1;
    SimpleLabel headline;
    Label lbPort;
    TextButton tbConnect;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCDialogWindow)
};



//==============================================================================
/*
*/
class OSCStatus : public Component, private Timer
{
public:
    OSCStatus(OSCReceiverPlus& receiver) : oscReceiver (receiver)
    {
        isConnected = oscReceiver.isConnected();
        startTimer(500);
    }

    ~OSCStatus()
    {
    }

    void timerCallback() override
    {
        const int port = oscReceiver.getPortNumber();
        bool shouldBeConnected = oscReceiver.isConnected();
        if (isConnected != shouldBeConnected || lastPort != port)
        {
            lastPort = port;
            isConnected = shouldBeConnected;
            repaint();
        }
    }
    void mouseEnter (const MouseEvent &event) override
    {
        setMouseCursor (MouseCursor::PointingHandCursor);
        repaint();
    }

    void mouseExit (const MouseEvent &event) override
    {
        repaint();
    }

    void mouseUp (const MouseEvent &event) override
    {
        auto* dialogWindow = new OSCDialogWindow (oscReceiver);
        dialogWindow->setSize (130, 38);

        CallOutBox& myBox = CallOutBox::launchAsynchronously (dialogWindow, getScreenBounds().removeFromLeft(14), nullptr);
        myBox.setLookAndFeel(&getLookAndFeel());
    }

    void paint (Graphics& g) override
    {
        Colour statusCol = oscReceiver.getPortNumber() == -1 ? Colours::white.withAlpha(0.1f) : oscReceiver.isConnected() ? Colours::limegreen : Colours::red.withAlpha (0.5f);

        const float alpha = isMouseOver() ? 1.0f : 0.5f;

        auto area = getLocalBounds();
        area = area.removeFromBottom (12);


        auto circleArea = area.removeFromLeft (12).toFloat().reduced(2.0f);
        circleArea.setY (circleArea.getY() - 1.0f);
        g.setColour (statusCol.withAlpha(alpha));
        g.drawRoundedRectangle(circleArea, 6, 1.0f);
        g.setColour (statusCol);
        g.fillRoundedRectangle (circleArea.removeFromLeft(14).reduced(2.0f), 6);


        area.removeFromLeft (2);

        g.setColour (Colours::white.withAlpha(isMouseOver() ? 1.0f : 0.5f));
        g.setFont (getLookAndFeel().getTypefaceForFont (Font (12.0f, 0)));
        g.setFont (14.0f);

        String text = "OSC";
        if (oscReceiver.isConnected())
            text += " (" + String (oscReceiver.getPortNumber()) + ")";
        g.drawText (text, area, Justification::bottomLeft, true);
    }

    void resized() override
    {
    }

private:
    OSCReceiverPlus& oscReceiver;
    bool isConnected = false;
    bool lastPort = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCStatus)
};




