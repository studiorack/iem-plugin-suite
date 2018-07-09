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
 This class can be used to add parameters to a AudioProcessorValueTree and make them controllable via OSC. The used parameterID will be saved in a StringArray. If the OSCPattern of the forwarded OSCMessages matches one of the parameterIDs, that Parameter will be controlled.
 */


class OSCParameterInterface
{
public:
    OSCParameterInterface (AudioProcessorValueTreeState &valueTreeState) : parameters (valueTreeState)
    {
    }

    /**
     Creates and AudioProcessorParameter and adds it to the AudioProcessorValueTreeState. Additionally, the parameterID will added to a StringArray.
     */
    AudioProcessorParameterWithID* createAndAddParameter (const String& parameterID,
                                                 const String& parameterName,
                                                 const String& labelText,
                                                 NormalisableRange<float> valueRange,
                                                 float defaultValue,
                                                 std::function<String (float)> valueToTextFunction,
                                                 std::function<float (const String&)> textToValueFunction,
                                                 bool isMetaParameter = false,
                                                 bool isAutomatableParameter = true,
                                                 bool isDiscrete = false,
                                                 AudioProcessorParameter::Category category
                                                 = AudioProcessorParameter::genericParameter,
                                                 bool isBoolean = false)
    {
        parameterIDs.add (parameterID);
        return parameters.createAndAddParameter (parameterID, parameterName, labelText, valueRange, defaultValue,
                                          valueToTextFunction, textToValueFunction,
                                          isMetaParameter, isAutomatableParameter, isDiscrete,
                                                 category, isBoolean);
    }

    /**
     Checks whether the OSCAdressPattern of the OSCMessage matches one of the ParameterID's and changes the parameter on success. Returns true, if there is a match. Make sure the plugin-name-prefix was trimmed.
     */
    bool processOSCMessage (OSCMessage oscMessage)
    {
        String address = oscMessage.getAddressPattern().toString().substring(1); // trimming forward slash
        if (parameterIDs.contains (address))
        {
            if (oscMessage.size() > 0)
            {
                auto arg = oscMessage[0];
                float value = 0.0f;
                if (arg.isInt32())
                    value = arg.getInt32();
                else if (arg.isFloat32())
                    value = arg.getFloat32();
                else
                    return true;

                setValue (address, value);
            }
            return true;
        }
        else
            return false;
    }

    void setValue ( String paramID, float value)
    {
        auto range (parameters.getParameterRange (paramID));
        parameters.getParameter(paramID)->setValue (range.convertTo0to1 (value));
    }

private:
    AudioProcessorValueTreeState &parameters;
    StringArray parameterIDs;

};
