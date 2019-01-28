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
                            public IOHelper<inputType, outputType>
{
public:

    AudioProcessorBase () : AudioProcessor(),
                            parameters (*this, nullptr, String (JucePlugin_Name), {}),
                            oscParameterInferface (parameters)
    {

    };

    AudioProcessorBase (ParameterList parameterLayout) :
                            AudioProcessor(),
    parameters (*this, nullptr, String (JucePlugin_Name), {parameterLayout.begin(), parameterLayout.end()}),
                            oscParameterInferface (parameters)
    {};

    AudioProcessorBase (const BusesProperties& ioLayouts, ParameterList parameterLayout) : AudioProcessor (ioLayouts),
    parameters (*this, nullptr, String (JucePlugin_Name), { parameterLayout.begin(), parameterLayout.end() }),
                                                            oscParameterInferface (parameters)
    {};

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

    /*
     This method is exptected to return true, if the OSCMessage is considered to have been consumed, and should not be passed on.
     */
    virtual inline bool interceptOscMessage (const OSCMessage &message)
    {
        return false; // not processed
    };

    /* This method will be called if the osc */;

    virtual inline void processNotYetConsumedOscMessage (const OSCMessage &message) {};

    void oscMessageReceived (const OSCMessage &message) override
    {
        if (! interceptOscMessage (message))
        {
            String prefix ("/" + String (JucePlugin_Name));
            if (message.getAddressPattern().toString().startsWith (prefix))
            {
                OSCMessage msg (message);
                msg.setAddressPattern (message.getAddressPattern().toString().substring (String (JucePlugin_Name).length() + 1));

                if (oscParameterInferface.processOSCMessage (msg))
                    return;
            }

            processNotYetConsumedOscMessage (message);
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
    OSCParameterInterface oscParameterInferface;
    OSCReceiverPlus oscReceiver;

private:

};

