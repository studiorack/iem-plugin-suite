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

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
CoordinateConverterAudioProcessor::CoordinateConverterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels(10), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
                     #endif
                       ),
#endif
parameters(*this, nullptr)
{

    parameters.createAndAddParameter ("azimuth", "Azimuth Angle", CharPointer_UTF8 (R"(°)"),
                                     NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) { return String(value, 2); }, nullptr);

    parameters.createAndAddParameter ("elevation", "Elevation Angle", CharPointer_UTF8 (R"(°)"),
                                     NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) { return String(value, 2); }, nullptr);

    parameters.createAndAddParameter ("radius", "Radius", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0,
                                     [](float value) { return String(value, 3); }, nullptr);

    parameters.createAndAddParameter ("xPos", "X Coordinate", "",
                                     NormalisableRange<float>(-1.0f, 1.0f, 0.0001f), 0.0,
                                     [](float value) { return String(value, 4); }, nullptr);

    parameters.createAndAddParameter ("yPos", "Y Coordinate", "",
                                     NormalisableRange<float>(-1.0f, 1.0f, 0.0001f), 0.0,
                                     [](float value) { return String(value, 4); }, nullptr);

    parameters.createAndAddParameter ("zPos", "Z Coordinate", "",
                                     NormalisableRange<float>(-1.0f, 1.0f, 0.0001f), 0.0,
                                     [](float value) { return String(value, 4); }, nullptr);

    parameters.createAndAddParameter ("xReference", "X Reference", "m",
                                      NormalisableRange<float>(-50.0f, 50.0f, 0.001f), 0.0,
                                      [](float value) { return String(value, 3); }, nullptr);

    parameters.createAndAddParameter ("yReference", "Y Reference", "m",
                                      NormalisableRange<float>(-50.0f, 50.0f, 0.001f), 0.0,
                                      [](float value) { return String(value, 3); }, nullptr);

    parameters.createAndAddParameter ("zReference", "Z Reference", "m",
                                      NormalisableRange<float>(-50.0f, 50.0f, 0.001f), 0.0,
                                      [](float value) { return String(value, 3); }, nullptr);

    parameters.createAndAddParameter ("radiusRange", "Radius Range", "m",
                                     NormalisableRange<float>(0.1f, 50.0f, 0.01f), 1.0,
                                     [](float value) { return String(value, 2); }, nullptr);

    parameters.createAndAddParameter ("xRange", "X Range", "m",
                                     NormalisableRange<float>(0.1f, 50.0f, 0.01f), 1.0,
                                     [](float value) { return String(value, 2); }, nullptr);

    parameters.createAndAddParameter ("yRange", "Y Range", "m",
                                     NormalisableRange<float>(0.1f, 50.0f, 0.01f), 1.0,
                                     [](float value) { return String(value, 2); }, nullptr);

    parameters.createAndAddParameter ("zRange", "Z Range", "m",
                                     NormalisableRange<float>(0.1f, 50.0f, 0.01f), 1.0,
                                     [](float value) { return String(value, 2); }, nullptr);


    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("CoordinateConverter"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened


    // get pointers to the parameters
    azimuth = parameters.getRawParameterValue ("azimuth");
    elevation = parameters.getRawParameterValue ("elevation");
    radius = parameters.getRawParameterValue ("radius");
    xPos = parameters.getRawParameterValue ("xPos");
    yPos = parameters.getRawParameterValue ("yPos");
    zPos = parameters.getRawParameterValue ("zPos");
    xReference = parameters.getRawParameterValue ("xReference");
    yReference = parameters.getRawParameterValue ("yReference");
    zReference = parameters.getRawParameterValue ("zReference");
    radiusRange = parameters.getRawParameterValue ("radiusRange");
    xRange = parameters.getRawParameterValue ("xRange");
    yRange = parameters.getRawParameterValue ("yRange");
    zRange = parameters.getRawParameterValue ("zRange");


    // add listeners to parameter changes
    parameters.addParameterListener ("azimuth", this);
    parameters.addParameterListener ("elevation", this);
    parameters.addParameterListener ("radius", this);
    parameters.addParameterListener ("xPos", this);
    parameters.addParameterListener ("yPos", this);
    parameters.addParameterListener ("zPos", this);
    parameters.addParameterListener ("xReference", this);
    parameters.addParameterListener ("yReference", this);
    parameters.addParameterListener ("zReference", this);
    parameters.addParameterListener ("radiusRange", this);
    parameters.addParameterListener ("xRange", this);
    parameters.addParameterListener ("yRange", this);
    parameters.addParameterListener ("zRange", this);


}

CoordinateConverterAudioProcessor::~CoordinateConverterAudioProcessor()
{
}

//==============================================================================
const String CoordinateConverterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CoordinateConverterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CoordinateConverterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CoordinateConverterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CoordinateConverterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CoordinateConverterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CoordinateConverterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CoordinateConverterAudioProcessor::setCurrentProgram (int index)
{
}

const String CoordinateConverterAudioProcessor::getProgramName (int index)
{
    return {};
}

void CoordinateConverterAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void CoordinateConverterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void CoordinateConverterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CoordinateConverterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void CoordinateConverterAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // nothing to do
}

//==============================================================================
bool CoordinateConverterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* CoordinateConverterAudioProcessor::createEditor()
{
    return new CoordinateConverterAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void CoordinateConverterAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}


void CoordinateConverterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (ValueTree::fromXml (*xmlState));
}

//==============================================================================
void CoordinateConverterAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "azimuth" || parameterID == "elevation" || parameterID == "radius" || parameterID == "radiusRange")
    {
        repaintSphere = true;

        if (! updatingParams.get())
            updateCartesianCoordinates();
    }

    else if (parameterID == "xPos" || parameterID == "yPos" || parameterID == "zPos")
    {
        repaintPositionPlanes = true;

        if (! updatingParams.get())
            updateSphericalCoordinates();
    }

    else if (parameterID == "xReference" || parameterID == "yReference" || parameterID == "zReference")
    {
        if (cartesianWasLastUpdated)
        {
            updateCartesianCoordinates();
        }
        else
        {
            updateSphericalCoordinates();
        }

    }
}

void CoordinateConverterAudioProcessor::updateCartesianCoordinates()
{
    updatingParams = true;

    auto cartesian = Conversions<float>::sphericalToCartesian (degreesToRadians (*azimuth), degreesToRadians (*elevation), *radius * *radiusRange);

    cartesian += {*xReference, *yReference, *zReference};
    cartesian.x /= *xRange;
    cartesian.y /= *yRange;
    cartesian.z /= *zRange;

    parameters.getParameter ("xPos")->setValue (parameters.getParameterRange ("xPos").convertTo0to1 (cartesian.x));
    parameters.getParameter ("yPos")->setValue (parameters.getParameterRange ("yPos").convertTo0to1 (cartesian.y));
    parameters.getParameter ("zPos")->setValue (parameters.getParameterRange ("zPos").convertTo0to1 (cartesian.z));
    repaintPositionPlanes = true;

    cartesianWasLastUpdated = true;
    updatingParams = false;
}

void CoordinateConverterAudioProcessor::updateSphericalCoordinates()
{
    updatingParams = true;

    auto cartesian = Vector3D<float> (*xPos * *xRange , *yPos * *yRange , *zPos * *zRange );
    cartesian -= {*xReference, *yReference, *zReference};
    auto spherical = Conversions<float>::cartesianToSpherical (cartesian);

    spherical.x /= *radiusRange; // radius component

    if (spherical.x >= 1.0f)
        spherical.x = 1;
    
    parameters.getParameter ("radius")->setValue (parameters.getParameterRange ("radius").convertTo0to1 (spherical.x));
    parameters.getParameter ("azimuth")->setValue (parameters.getParameterRange ("azimuth").convertTo0to1  (spherical.y));
    parameters.getParameter ("elevation")->setValue (parameters.getParameterRange ("elevation").convertTo0to1(spherical.z));
    repaintPositionPlanes = true;

    cartesianWasLastUpdated = false;
    updatingParams = false;
}

void CoordinateConverterAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}

//==============================================================================
pointer_sized_int CoordinateConverterAudioProcessor::handleVstPluginCanDo (int32 index,
                                                                     pointer_sized_int value, void* ptr, float opt)
{
    auto text = (const char*) ptr;
    auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

    if (matches ("wantsChannelCountNotifications"))
        return 1;
    return 0;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CoordinateConverterAudioProcessor();
}
