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

#define pi 3.1415926536
#define deg2rad M_PI/180.0
#define rad2deg 180.0/M_PI

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


//==============================================================================
MultiEncoderAudioProcessor::MultiEncoderAudioProcessor()

#ifndef JucePlugin_PreferredChannelConfigurations
:AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                 .withInput  ("Input",  AudioChannelSet::discreteChannels(maxNumberOfInputs), true)
#endif
                 .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                 ),
#endif
parameters (*this, nullptr)

{
    parameters.createAndAddParameter("inputSetting", "Number of input channels ", "",
                                     NormalisableRange<float> (0.0f, maxNumberOfInputs, 1.0f), startNnumberOfInputs,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter ("orderSetting", "Ambisonics Order", "",
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
                                      nullptr);
    parameters.createAndAddParameter ("useSN3D", "Normalization", "",
                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                      [](float value)
                                      {
                                          if (value >= 0.5f ) return "SN3D";
                                          else return "N3D";
                                      }, nullptr);
    
    parameters.createAndAddParameter("masterYaw", "Master yaw angle", CharPointer_UTF8 (R"(°)"),
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("masterPitch", "Master pitch angle", CharPointer_UTF8 (R"(°)"),
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    parameters.createAndAddParameter("masterRoll", "Master roll angle", CharPointer_UTF8 (R"(°)"),
                                     NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0f,
                                     [](float value) {return String(value);}, nullptr);
    
    parameters.createAndAddParameter("lockedToMaster", "Lock Directions relative to Master", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                     [](float value) {return (value >= 0.5f) ? "locked" : "not locked";}, nullptr);
    
    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        parameters.createAndAddParameter("yaw" + String(i), "Yaw angle " + String(i), CharPointer_UTF8 (R"(°)"),
                                         NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                         [](float value) {return String(value, 2);}, nullptr);
        parameters.createAndAddParameter("pitch" + String(i), "Pitch angle " + String(i), CharPointer_UTF8 (R"(°)"),
                                         NormalisableRange<float> (-180.0f, 180.0f, 0.01f), 0.0,
                                         [](float value) {return String(value, 2);}, nullptr);
        parameters.createAndAddParameter("gain" + String(i), "Gain " + String(i), CharPointer_UTF8 (R"(°)"),
                                         NormalisableRange<float> (-60.0f, 10.0f, 0.1f), 0.0f,
                                         [](float value) {return (value >= -59.9f) ? String(value, 2) : "-inf";},
                                         nullptr);
        parameters.createAndAddParameter("mute" + String(i), "Mute input " + String(i), "",
                                         NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                         [](float value) {return (value >= 0.5f) ? "muted" : "not muted";}, nullptr);
        
        parameters.createAndAddParameter("solo" + String(i), "Solo input " + String(i), "",
                                         NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                         [](float value) {return (value >= 0.5f) ? "soloed" : "not soloed";}, nullptr);
    }
    
    
    
    
    parameters.state = ValueTree (Identifier ("MultiEncoder"));
    
    parameters.addParameterListener("masterYaw", this);
    parameters.addParameterListener("masterPitch", this);
    parameters.addParameterListener("masterRoll", this);
    parameters.addParameterListener("lockedToMaster", this);
    
    parameters.addParameterListener("orderSetting", this);
    parameters.addParameterListener("inputSetting", this);
    
    muteMask.clear();
    soloMask.clear();
    
    
    
    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        yaw[i] = parameters.getRawParameterValue ("yaw"+String(i));
        pitch[i] = parameters.getRawParameterValue ("pitch"+String(i));
        gain[i] = parameters.getRawParameterValue ("gain"+String(i));
        mute[i] = parameters.getRawParameterValue ("mute"+String(i));
        solo[i] = parameters.getRawParameterValue ("solo"+String(i));
        
        if (*mute[i] >= 0.5f) muteMask.setBit(i);
        if (*solo[i] >= 0.5f) soloMask.setBit(i);
        
        parameters.addParameterListener("yaw"+String(i), this);
        parameters.addParameterListener("pitch"+String(i), this);
        parameters.addParameterListener("mute"+String(i), this);
        parameters.addParameterListener("solo"+String(i), this);
    }
    
    masterYaw = parameters.getRawParameterValue("masterYaw");
    masterPitch = parameters.getRawParameterValue("masterPitch");
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

//==============================================================================
const String MultiEncoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiEncoderAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool MultiEncoderAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double MultiEncoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
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

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiEncoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

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
        
        float cosPitch = std::cos(*pitch[i] * deg2rad);
        float yawInRad = *yaw[i] * deg2rad;
        float pitchInRad = *pitch[i] * deg2rad;
        Vector3D<float> pos (cosPitch * cos(yawInRad), cosPitch * sin(yawInRad), sin(-1.0f * pitchInRad));
        
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
            DBG("toggled");
            const int nChIn = input.getSize();
            float ypr[3];
            ypr[2] = 0.0f;
            for (int i = 0; i < nChIn; ++i)
            {
                iem::Quaternion<float> masterQuat;
                float masterypr[3];
                masterypr[0] = degreesToRadians(*masterYaw);
                masterypr[1] = degreesToRadians(*masterPitch);
                masterypr[2] = degreesToRadians(*masterRoll);
                masterQuat.fromYPR(masterypr);
                masterQuat.conjugate();
                
                ypr[0] = degreesToRadians(*yaw[i]);
                ypr[1] = degreesToRadians(*pitch[i]);
                quats[i].fromYPR(ypr);
                quats[i] = masterQuat*quats[i];
            }
            locked = true;
        }
        else if (newValue < 0.5f)
            locked = false;
    }
    else if (locked && ((parameterID == "masterYaw") ||  (parameterID == "masterPitch") ||  (parameterID == "masterRoll")))
    {
        DBG("moving");
        moving = true;
        iem::Quaternion<float> masterQuat;
        float ypr[3];
        ypr[0] = degreesToRadians(*masterYaw);
        ypr[1] = degreesToRadians(*masterPitch);
        ypr[2] = degreesToRadians(*masterRoll);
        masterQuat.fromYPR(ypr);
        
        const int nChIn = input.getSize();
        for (int i = 0; i < nChIn; ++i)
        {
            iem::Quaternion<float> temp = masterQuat*quats[i];
            temp.toYPR(ypr);
            parameters.getParameterAsValue("yaw" + String(i)).setValue(radiansToDegrees(ypr[0]));
            parameters.getParameterAsValue("pitch" + String(i)).setValue(radiansToDegrees(ypr[1]));
        }
        moving = false;
    }
    else if (locked && !moving &&  (parameterID.startsWith("yaw") || parameterID.startsWith("pitch")))
    {
        DBG("yawPitch");
        float ypr[3];
        ypr[2] = 0.0f;
        const int nChIn = input.getSize();
        for (int i = 0; i < nChIn; ++i)
        {
            iem::Quaternion<float> masterQuat;
            float masterypr[3];
            masterypr[0] = degreesToRadians(*masterYaw);
            masterypr[1] = degreesToRadians(*masterPitch);
            masterypr[2] = degreesToRadians(*masterRoll);
            masterQuat.fromYPR(masterypr);
            masterQuat.conjugate();
            
            ypr[0] = degreesToRadians(*yaw[i]);
            ypr[1] = degreesToRadians(*pitch[i]);
            quats[i].fromYPR(ypr);
            quats[i] = masterQuat*quats[i];
        }
    }
}


//==============================================================================
void MultiEncoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    for (int i = 0; i < maxNumberOfInputs; ++i)
        parameters.state.setProperty("colour" + String(i), elementColours[i].toString(), nullptr);
    
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MultiEncoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{    
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
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
        parameters.getParameter("mute" + String(i))->setValue(0.0f);
        parameters.getParameter("solo" + String(i))->setValue(0.0f);
    }
};

void MultiEncoderAudioProcessor::updateQuaternions()
{
    
    float ypr[3];
    ypr[2] = 0.0f;
    
    iem::Quaternion<float> masterQuat;
    float masterypr[3];
    masterypr[0] = degreesToRadians(*masterYaw);
    masterypr[1] = degreesToRadians(*masterPitch);
    masterypr[2] = degreesToRadians(*masterRoll);
    masterQuat.fromYPR(masterypr);
    masterQuat.conjugate();
    
    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        ypr[0] = degreesToRadians(*yaw[i]);
        ypr[1] = degreesToRadians(*pitch[i]);
        quats[i].fromYPR(ypr);
        quats[i] = masterQuat*quats[i];
    }
}
