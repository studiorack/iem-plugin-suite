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

/**
 An extension to JUCE's OSCReceiver class with some useful methods.
 */

class OSCReceiverPlus : public OSCReceiver
{
public:
    OSCReceiverPlus ()
    {
        connected = false;
    }

    bool connect (const int portNumber)
    {
        port = portNumber;
        if (portNumber == -1)
        {
            disconnect ();
            connected = false;
            return true;
        }

        if (OSCReceiver::connect (port))
        {
            connected = true;
            return true;
        }
        else
            return false;
    }

    bool disconnect()
    {
        if (OSCReceiver::disconnect())
        {
            connected = false;
            return true;
        }
        else
            return false;
    }

    const int getPortNumber()
    {
        return port;
    }

    bool isConnected()
    {
        return connected;
    }


private:
    int port = -1;
    bool connected;
};
