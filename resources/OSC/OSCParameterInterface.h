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
#include "../JuceLibraryCode/JuceHeader.h"
#include "OSCUtilities.h"



//#define DEBUG_PARAMETERS_FOR_DOCUMENTATION

/**
 This class can be used to add parameters to a AudioProcessorValueTree and make them controllable via OSC. The used parameterID will be saved in a StringArray. If the OSCPattern of the forwarded OSCMessages matches one of the parameterIDs, that Parameter will be controlled.
 */


class OSCParameterInterface : public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>, private Timer
{
public:
    OSCParameterInterface (OSCMessageInterceptor& interceptor, AudioProcessorValueTreeState &valueTreeState);

    static std::unique_ptr<RangedAudioParameter> createParameterTheOldWay (const String& parameterID,
                                                                 const String& parameterName,
                                                                 const String& labelText,
                                                                 NormalisableRange<float> valueRange,
                                                                 float defaultValue,
                                                                 std::function<String (float)> valueToTextFunction = nullptr,
                                                                 std::function<float (const String&)> textToValueFunction = nullptr,
                                                                 bool isMetaParameter = false,
                                                                 bool isAutomatableParameter = true,
                                                                 bool isDiscrete = false,
                                                                 AudioProcessorParameter::Category category
                                                                 = AudioProcessorParameter::genericParameter,
                                                                           bool isBoolean = false);

    /**
     Checks whether the OSCAdressPattern of the OSCMessage matches one of the ParameterID's and changes the parameter on success. Returns true, if there is a match. Make sure the plugin-name-prefix was trimmed.
     */
    const bool processOSCMessage (OSCMessage oscMessage);

    /**
     Sets the value of an audio-parameter with the specified parameter ID. The provided value will be mapped to a 0-to-1 range.
     */
    void setValue (const String paramID, const float value);

    OSCReceiverPlus& getOSCReceiver() { return oscReceiver; };
    OSCSenderPlus& getOSCSender() { return oscSender; };

    void oscMessageReceived (const OSCMessage &message) override;
    void oscBundleReceived (const OSCBundle &bundle) override;


    void timerCallback() override;

    void sendParameterChanges (const bool forceSend = false);
    void setOSCAddress (const String newAddress);

    const String getOSCAddress() const { return address; };

    void setInterval (const int interValInMilliseconds);
    const int getInterval() const { return getTimerInterval(); }

    ValueTree getConfig() const;
    void setConfig (ValueTree config);

private:
    OSCMessageInterceptor& interceptor;
    AudioProcessorValueTreeState& parameters;

    OSCReceiverPlus oscReceiver;
    OSCSenderPlus oscSender;

    String address;
    Array<float> lastSentValues;
};
