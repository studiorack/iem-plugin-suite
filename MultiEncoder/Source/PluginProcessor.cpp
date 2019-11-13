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
MultiEncoderAudioProcessor::MultiEncoderAudioProcessor()
: AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
                 BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                 .withInput  ("Input",  AudioChannelSet::discreteChannels(maxNumberOfInputs), true)
#endif
                 .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                 ,
#endif
createParameterLayout())
{
    // global properties
    PropertiesFile::Options options;
    options.applicationName     = "MultiEncoder";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";

    properties.reset (new PropertiesFile (options));
    lastDir = File (properties->getValue ("presetFolder"));


    parameters.addParameterListener("masterAzimuth", this);
    parameters.addParameterListener("masterElevation", this);
    parameters.addParameterListener("masterRoll", this);
    parameters.addParameterListener("lockedToMaster", this);

    parameters.addParameterListener("orderSetting", this);
    parameters.addParameterListener("inputSetting", this);

    muteMask.clear();
    soloMask.clear();

    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        azimuth[i] = parameters.getRawParameterValue ("azimuth"+String(i));
        elevation[i] = parameters.getRawParameterValue ("elevation"+String(i));
        gain[i] = parameters.getRawParameterValue ("gain"+String(i));
        mute[i] = parameters.getRawParameterValue ("mute"+String(i));
        solo[i] = parameters.getRawParameterValue ("solo"+String(i));

        if (*mute[i] >= 0.5f) muteMask.setBit(i);
        if (*solo[i] >= 0.5f) soloMask.setBit(i);

        parameters.addParameterListener("azimuth"+String(i), this);
        parameters.addParameterListener("elevation"+String(i), this);
        parameters.addParameterListener("mute"+String(i), this);
        parameters.addParameterListener("solo"+String(i), this);
    }

    masterAzimuth = parameters.getRawParameterValue("masterAzimuth");
    masterElevation = parameters.getRawParameterValue("masterElevation");
    masterRoll = parameters.getRawParameterValue("masterRoll");
    lockedToMaster = parameters.getRawParameterValue("locking");



    inputSetting = parameters.getRawParameterValue("inputSetting");
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    processorUpdatingParams = false;

    yprInput = true; //input from ypr

    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        FloatVectorOperations::clear(SH[i], 64);
        _gain[i] = 0.0f;
        //elemActive[i] = *gain[i] >= -59.9f;
        elementColours[i] = Colours::cyan;
    }

    updateQuaternions();
}

MultiEncoderAudioProcessor::~MultiEncoderAudioProcessor()
{
}


int MultiEncoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int MultiEncoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiEncoderAudioProcessor::setCurrentProgram (int index)
{
}

const String MultiEncoderAudioProcessor::getProgramName (int index)
{
    return String();
}

void MultiEncoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MultiEncoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *inputSetting, *orderSetting, true);
}

void MultiEncoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void MultiEncoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputSetting, *orderSetting);

    const int nChOut = jmin(buffer.getNumChannels(), output.getNumberOfChannels());
    const int nChIn = jmin(buffer.getNumChannels(), input.getSize());
    const int ambisonicOrder = output.getOrder();

    for (int i = 0; i < nChIn; ++i){
        bufferCopy.copyFrom(i, 0, buffer.getReadPointer(i), buffer.getNumSamples());
    }

    buffer.clear();

    for (int i = 0; i < nChIn; ++i)
    {
        FloatVectorOperations::copy(_SH[i], SH[i], nChOut);
        float currGain = 0.0f;

        if (!soloMask.isZero()) {
            if (soloMask[i]) currGain = Decibels::decibelsToGain(*gain[i]);
        }
        else
        {
            if (!muteMask[i]) currGain = Decibels::decibelsToGain(*gain[i]);
        }

        const float azimuthInRad = degreesToRadians(*azimuth[i]);
        const float elevationInRad = degreesToRadians(*elevation[i]);

        Vector3D<float> pos {Conversions<float>::sphericalToCartesian(azimuthInRad, elevationInRad)};

        SHEval(ambisonicOrder, pos.x, pos.y, pos.z, SH[i]);

        if (*useSN3D >= 0.5f)
        {
            FloatVectorOperations::multiply(SH[i], SH[i], n3d2sn3d, nChOut);
        }

        const float* inpReadPtr = bufferCopy.getReadPointer(i);
        for (int ch = 0; ch < nChOut; ++ch) {
            buffer.addFromWithRamp(ch, 0, inpReadPtr, buffer.getNumSamples(), _SH[i][ch]*_gain[i], SH[i][ch]*currGain);
        }
        _gain[i] = currGain;
    }


}

//==============================================================================
bool MultiEncoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MultiEncoderAudioProcessor::createEditor()
{
    return new MultiEncoderAudioProcessorEditor (*this, parameters);
}

void MultiEncoderAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "inputSetting" || parameterID == "orderSetting")
    {
        userChangedIOSettings = true;
    }
    else if (parameterID.startsWith("solo"))
    {
        const int id = parameterID.substring(4).getIntValue();
        soloMask.setBit(id,newValue >= 0.5f);
        soloMuteChanged = true;
    }
    else if (parameterID.startsWith("mute"))
    {
        const int id = parameterID.substring(4).getIntValue();
        muteMask.setBit(id,newValue >= 0.5f);
        soloMuteChanged = true;
    }
    else if (parameterID == "lockedToMaster")
    {
        if (newValue >= 0.5f && !locked)
        {
            const int nChIn = input.getSize();
            float ypr[3];
            ypr[2] = 0.0f;
            for (int i = 0; i < nChIn; ++i)
            {
                iem::Quaternion<float> masterQuat;
                float masterypr[3];
                masterypr[0] = degreesToRadians(*masterAzimuth);
                masterypr[1] = degreesToRadians(*masterElevation);
                masterypr[2] = - degreesToRadians(*masterRoll);
                masterQuat.fromYPR(masterypr);
                masterQuat.conjugate();

                ypr[0] = degreesToRadians(*azimuth[i]);
                ypr[1] = degreesToRadians(*elevation[i]);
                quats[i].fromYPR(ypr);
                quats[i] = masterQuat*quats[i];
            }
            locked = true;
        }
        else if (newValue < 0.5f)
            locked = false;
    }
    else if (locked && ((parameterID == "masterAzimuth") ||  (parameterID == "masterElevation") ||  (parameterID == "masterRoll")))
    {
        moving = true;
        iem::Quaternion<float> masterQuat;
        float ypr[3];
        ypr[0] = degreesToRadians(*masterAzimuth);
        ypr[1] = degreesToRadians(*masterElevation);
        ypr[2] = - degreesToRadians(*masterRoll);
        masterQuat.fromYPR(ypr);

        const int nChIn = input.getSize();
        for (int i = 0; i < nChIn; ++i)
        {
            iem::Quaternion<float> temp = masterQuat * quats[i];
            temp.toYPR(ypr);
            parameters.getParameter ("azimuth" + String (i))->setValueNotifyingHost (parameters.getParameterRange ("azimuth" + String (i)).convertTo0to1 (radiansToDegrees (ypr[0])));
            parameters.getParameter ("elevation" + String (i))->setValueNotifyingHost (parameters.getParameterRange ("elevation" + String (i)).convertTo0to1 (radiansToDegrees(ypr[1])));
        }
        moving = false;
        updateSphere = true;
    }
    else if (locked && !moving && (parameterID.startsWith("azimuth") || parameterID.startsWith("elevation")))
    {
        float ypr[3];
        ypr[2] = 0.0f;
        const int nChIn = input.getSize();
        for (int i = 0; i < nChIn; ++i)
        {
            iem::Quaternion<float> masterQuat;
            float masterypr[3];
            masterypr[0] = degreesToRadians(*masterAzimuth);
            masterypr[1] = degreesToRadians(*masterElevation);
            masterypr[2] = - degreesToRadians(*masterRoll);
            masterQuat.fromYPR(masterypr);
            masterQuat.conjugate();

            ypr[0] = degreesToRadians(*azimuth[i]);
            ypr[1] = degreesToRadians(*elevation[i]);
            quats[i].fromYPR(ypr);
            quats[i] = masterQuat*quats[i];
        }
        updateSphere = true;
    }
    else if (parameterID.startsWith("azimuth") || parameterID.startsWith("elevation") || (parameterID == "masterAzimuth") ||  (parameterID == "masterElevation"))
    {
        updateSphere = true;
    }
}


//==============================================================================
void MultiEncoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = parameters.copyState();
    for (int i = 0; i < maxNumberOfInputs; ++i)
        state.setProperty ("colour" + String(i), elementColours[i].toString(), nullptr);

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MultiEncoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.state = ValueTree::fromXml (*xmlState);
            updateQuaternions();
            for (int i = 0; i < maxNumberOfInputs; ++i)
                if (parameters.state.getProperty("colour" + String(i)).toString() != "0")
                    elementColours[i] = Colour::fromString(parameters.state.getProperty("colour" + String(i)).toString());
                else elementColours[i] = Colours::cyan;
            updateColours = true;

            if (parameters.state.hasProperty ("OSCPort")) // legacy
            {
                oscParameterInterface.getOSCReceiver().connect (parameters.state.getProperty ("OSCPort", var (-1)));
                parameters.state.removeProperty ("OSCPort", nullptr);
            }

            auto oscConfig = parameters.state.getChildWithName ("OSCConfig");
            if (oscConfig.isValid())
                oscParameterInterface.setConfig (oscConfig);
        }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiEncoderAudioProcessor();
}

void MultiEncoderAudioProcessor::updateBuffers() {
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());

    const int nChIn = input.getSize();
    const int _nChIn = input.getPreviousSize();

    bufferCopy.setSize(nChIn, getBlockSize());

    // disable solo and mute for deleted input channels
    for (int i = nChIn; i < _nChIn; ++i)
    {
        parameters.getParameter ("mute" + String(i))->setValueNotifyingHost (0.0f);
        parameters.getParameter ("solo" + String(i))->setValueNotifyingHost (0.0f);
    }
};

void MultiEncoderAudioProcessor::updateQuaternions()
{

    float ypr[3];
    ypr[2] = 0.0f;

    iem::Quaternion<float> masterQuat;
    float masterypr[3];
    masterypr[0] = degreesToRadians(*masterAzimuth);
    masterypr[1] = degreesToRadians(*masterElevation);
    masterypr[2] = - degreesToRadians(*masterRoll);
    masterQuat.fromYPR(masterypr);
    masterQuat.conjugate();

    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        ypr[0] = degreesToRadians(*azimuth[i]);
        ypr[1] = degreesToRadians(*elevation[i]);
        quats[i].fromYPR(ypr);
        quats[i] = masterQuat*quats[i];
    }
}


//==============================================================================
std::vector<std::unique_ptr<RangedAudioParameter>> MultiEncoderAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay("inputSetting", "Number of input channels ", "",
                                    NormalisableRange<float> (0.0f, maxNumberOfInputs, 1.0f), startNnumberOfInputs,
                                    [](float value) {return String(value);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("orderSetting", "Ambisonics Order", "",
                                     NormalisableRange<float> (0.0f, 8.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value >= 0.5f && value < 1.5f) return "0th";
                                         else if (value >= 1.5f && value < 2.5f) return "1st";
                                         else if (value >= 2.5f && value < 3.5f) return "2nd";
                                         else if (value >= 3.5f && value < 4.5f) return "3rd";
                                         else if (value >= 4.5f && value < 5.5f) return "4th";
                                         else if (value >= 5.5f && value < 6.5f) return "5th";
                                         else if (value >= 6.5f && value < 7.5f) return "6th";
                                         else if (value >= 7.5f) return "7th";
                                         else return "Auto";},
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("useSN3D", "Normalization", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value)
                                     {
                                         if (value >= 0.5f ) return "SN3D";
                                         else return "N3D";
                                     }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay("masterAzimuth", "Master azimuth angle", CharPointer_UTF8 (R"(°)"),
                                    NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0f,
                                    [](float value) {return String(value, 2);}, nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay("masterElevation", "Master elevation angle", CharPointer_UTF8 (R"(°)"),
                                    NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0f,
                                    [](float value) {return String(value, 2);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay("masterRoll", "Master roll angle", CharPointer_UTF8 (R"(°)"),
                                    NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0f,
                                    [](float value) {return String(value, 2);}, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay("lockedToMaster", "Lock Directions relative to Master", "",
                                    NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                    [](float value) {return (value >= 0.5f) ? "locked" : "not locked";}, nullptr));

    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        params.push_back (OSCParameterInterface::createParameterTheOldWay("azimuth" + String(i), "Azimuth angle " + String(i + 1), CharPointer_UTF8 (R"(°)"),
                                        NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                        [](float value) {return String(value, 2);}, nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay("elevation" + String(i), "Elevation angle " + String(i + 1), CharPointer_UTF8 (R"(°)"),
                                        NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                        [](float value) {return String(value, 2);}, nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay("gain" + String(i), "Gain " + String(i + 1), "dB",
                                        NormalisableRange<float> (-60.0f, 10.0f, 0.1f), 0.0f,
                                        [](float value) {return (value >= -59.9f) ? String(value, 1) : "-inf";},
                                        nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay("mute" + String(i), "Mute input " + String(i + 1), "",
                                        NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                        [](float value) {return (value >= 0.5f) ? "muted" : "not muted";}, nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay("solo" + String(i), "Solo input " + String(i + 1), "",
                                        NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                        [](float value) {return (value >= 0.5f) ? "soloed" : "not soloed";}, nullptr));
    }


    return params;
}

//==============================================================================

Result MultiEncoderAudioProcessor::loadConfiguration (const File& configFile)
{
    ValueTree newSources ("NewSources");

    Result result = ConfigurationHelper::parseFileForLoudspeakerLayout (configFile, newSources, nullptr);

    if (result.wasOk())
    {
        int nSrc = 0;
        const auto nElements = newSources.getNumChildren();

        for (int i = 0; i < nElements; ++i)
        {
            auto src = newSources.getChild (i);
            const int ch = src.getProperty ("Channel");
            const bool isImaginary = src.getProperty ("Imaginary");
            if (! isImaginary && ch > nSrc)
                nSrc = ch;
        }

        DBG (nSrc << " Sources!");
        parameters.getParameterAsValue ("inputSetting").setValue (nSrc);

        for (int s = 0; s < nSrc; ++s)
            parameters.getParameterAsValue ("mute" + String (s)).setValue (1);

        for (int e = 0; e < nElements; ++e)
        {
            const auto src = newSources.getChild (e);
            const int ch = static_cast<int> (src.getProperty ("Channel", 0)) - 1;
            const bool isImaginary = src.getProperty ("Imaginary");

            if (isImaginary || ch < 0 || ch >= 64)
                continue;


            parameters.getParameterAsValue ("mute" + String (ch)).setValue (0);

            auto azi = src.getProperty ("Azimuth", 0.0f);
            parameters.getParameterAsValue ("azimuth" + String (ch)).setValue (azi);
            auto ele = src.getProperty ("Elevation", 0.0f);
            parameters.getParameterAsValue ("elevation" + String (ch)).setValue (ele);
        }
    }

    return result;
}


void MultiEncoderAudioProcessor::setLastDir (File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue ("presetFolder", v);
}
