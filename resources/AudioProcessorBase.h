/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2019 - Institute of Electronic Music and Acoustics (IEM)
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

#include "../../resources/OSCInputStream.h"
#include "../../resources/OSCParameterInterface.h"
#include "../../resources/OSCReceiverPlus.h"
#include "../../resources/IOHelper.h"

typedef std::vector<std::unique_ptr<RangedAudioParameter>> ParameterList;

template <class inputType, class outputType>

class AudioProcessorBase :  public AudioProcessor,
                            public VSTCallbackHandler,
                            public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
                            public IOHelper<inputType, outputType>,
                            public AudioProcessorValueTreeState::Listener
{
public:

    AudioProcessorBase () : AudioProcessor(),
                            oscParameterInterface (parameters),
                            parameters (*this, nullptr, String (JucePlugin_Name), {})
    {
    };

    AudioProcessorBase (ParameterList parameterLayout) :
                            AudioProcessor(),
                            parameters (*this, nullptr, String (JucePlugin_Name), {parameterLayout.begin(), parameterLayout.end()}),
                            oscParameterInterface (parameters)
    {
    };

    AudioProcessorBase (const BusesProperties& ioLayouts, ParameterList parameterLayout) :
                            AudioProcessor (ioLayouts),
                            parameters (*this, nullptr, String (JucePlugin_Name), { parameterLayout.begin(), parameterLayout.end() }),
                            oscParameterInterface (parameters)
    {
    };

    ~AudioProcessorBase() {};



    //======== AudioProcessor stuff  =======================================================

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return true;
    };
#endif


    const String getName() const override
    {
        return JucePlugin_Name;
    }


    bool acceptsMidi() const override
    {
#if JucePlugin_WantsMidiInput
        return true;
#else
        return false;
#endif
    }

    bool producesMidi() const override
    {
#if JucePlugin_ProducesMidiOutput
        return true;
#else
        return false;
#endif
    }

    double getTailLengthSeconds() const override
    {
        return 0.0;
    }

    //======== VSTCallbackHandler =======================================================
    pointer_sized_int handleVstManufacturerSpecific (int32 index, pointer_sized_int value,
                                                     void* ptr, float opt) override
    {
        if (index == 0) // let's make this the OSCMessage selector for now
        {
            try
            {
                size_t size = static_cast<size_t> (value); // let's make this the data size

                MyOSCInputStream inputStream (ptr, size);
                auto inMessage = inputStream.readMessage();

                oscMessageReceived (inMessage);
                return 1;
            }
            catch (const OSCFormatError&)
            {
                return -1;
            }
        }

        return 0;
    };

    pointer_sized_int handleVstPluginCanDo (int32 index, pointer_sized_int value,
                                            void* ptr, float opt) override
    {
        auto text = (const char*) ptr;
        auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

        if (matches ("wantsChannelCountNotifications"))
            return 1;
        return 0;
    };
    //==============================================================================



    // ========================= OSC ===============================================

    /**
     This method is exptected to return true, if the OSCMessage is considered to have been consumed, and should not be passed on.
     */
    virtual inline const bool interceptOSCMessage (OSCMessage &message)
    {
        return false; // not consumed
    };


    /**
     This method will be called if the OSC message wasn't consumed by both 'interceptOscMessage(...)' and the oscParameterInterface.processOSCmessage(...)' method.
     The method is expected to return true, if the SOCMessage is considered to have been consumed, and should not be passed on.
     */

    virtual inline const bool processNotYetConsumedOSCMessage (const OSCMessage &message)
    {
        return false;
    };

    void oscMessageReceived (const OSCMessage &message) override
    {
        OSCMessage messageCopy (message);
        if (! interceptOSCMessage (messageCopy))
        {
            String prefix ("/" + String (JucePlugin_Name));
            if (message.getAddressPattern().toString().startsWith (prefix))
            {
                OSCMessage msg (message);
                msg.setAddressPattern (message.getAddressPattern().toString().substring (String (JucePlugin_Name).length() + 1));

                if (oscParameterInterface.processOSCMessage (msg))
                    return;
            }

            if (processNotYetConsumedOSCMessage (message))
                return;

            // open/change osc port
            if (message.getAddressPattern().toString().equalsIgnoreCase ("/openOSCPort") && message.size() == 1)
            {
                int newPort = -1;
                
                if (message[0].isInt32())
                    newPort = message[0].getInt32();
                else if (message[0].isFloat32())
                    newPort = static_cast<int> (message[0].getFloat32());

                if (newPort > 0)
                {
                    newPortNumber = newPort;
                    MessageManager::callAsync ( [this]() { oscReceiver.connect (newPortNumber); } );
                }
            }
        }
    }

    void oscBundleReceived (const OSCBundle &bundle) override
    {
        for (int i = 0; i < bundle.size(); ++i)
        {
            auto elem = bundle[i];
            if (elem.isMessage())
                oscMessageReceived (elem.getMessage());
            else if (elem.isBundle())
                oscBundleReceived (elem.getBundle());
        }
    };

    OSCReceiverPlus& getOSCReceiver () { return oscReceiver; }

    //==============================================================================


    AudioProcessorValueTreeState parameters;
    OSCParameterInterface oscParameterInterface;
    OSCReceiverPlus oscReceiver;

private:

    bool shouldOpenNewPort = false;
    int newPortNumber = -1;
};

