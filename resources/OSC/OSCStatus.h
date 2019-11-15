/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
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
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include "../customComponents/SimpleLabel.h"
#include "OSCParameterInterface.h"

class OSCDialogWindow  : public Component, private Timer, private Label::Listener
{
public:
    OSCDialogWindow (OSCParameterInterface& oscInterface, OSCReceiverPlus& oscReceiver, OSCSenderPlus& oscSender);

    void timerCallback() override;

    void updateOSCAddress();

    void labelTextChanged (Label *labelThatHasChanged) override;

    void checkPortAndConnectSender();

    void checkPortAndConnectReceiver();

    void resized() override;

private:
    OSCParameterInterface& interface;
    OSCReceiverPlus& receiver;
    OSCSenderPlus& sender;

    bool isReceiverConnected = false;

    bool isSenderConnected = false;

    GroupComponent receiverGroup, senderGroup;

    SimpleLabel slRecPort, slSendIP, slSendPort, slSendName, slInterval;
    Label lbRPort, lbSPort, lbSHostname, lbSOSCAddress;

    Slider intervalSlider;
    TextButton tbReceiverOpen, tbSenderOpen, tbFlush;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCDialogWindow)
};



//==============================================================================
/*
 */
class OSCStatus : public Component, private Timer
{
public:
    OSCStatus (OSCParameterInterface& oscInterface);

    void timerCallback() override;

    void mouseMove (const MouseEvent &event) override;

    void mouseExit (const MouseEvent &event) override;

    void mouseUp (const MouseEvent &event) override;

    void paint (Graphics& g) override;


private:
    OSCParameterInterface& oscParameterInterface;
    OSCReceiverPlus& oscReceiver;
    OSCSenderPlus& oscSender;

    Rectangle<int> bounds;
    bool mouseOver = false;

    bool isReceiverOpen = false;
    int lastReceiverPort = -1;

    bool isSenderOpen = false;
    int lastSenderPort = -1;
    String lastSenderHostName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCStatus)
};




