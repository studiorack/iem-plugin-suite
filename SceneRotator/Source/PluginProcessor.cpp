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
SceneRotatorAudioProcessor::SceneRotatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels (64), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels (64), true)
                     #endif
                       ),
#endif
parameters (*this, nullptr), oscParams (parameters)
{
    oscParams.createAndAddParameter ("orderSetting", "Ambisonics Order", "",
                                    NormalisableRange<float>(0.0f, 8.0f, 1.0f), 0.0f,
                                    [](float value) {
                                        if (value >= 0.5f && value < 1.5f) return "0th";
                                        else if (value >= 1.5f && value < 2.5f) return "1st";
                                        else if (value >= 2.5f && value < 3.5f) return "2nd";
                                        else if (value >= 3.5f && value < 4.5f) return "3rd";
                                        else if (value >= 4.5f && value < 5.5f) return "4th";
                                        else if (value >= 5.5f && value < 6.5f) return "5th";
                                        else if (value >= 6.5f && value < 7.5f) return "6th";
                                        else if (value >= 7.5f) return "7th";
                                        else return "Auto";
                                    }, nullptr);

    oscParams.createAndAddParameter ("useSN3D", "Normalization", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value)
                                     {
                                         if (value >= 0.5f ) return "SN3D";
                                         else return "N3D";
                                     }, nullptr);

    oscParams.createAndAddParameter ("yaw", "Yaw Angle", CharPointer_UTF8 (R"(°)"),
                                                 NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true);

    oscParams.createAndAddParameter ("pitch", "Pitch Angle", CharPointer_UTF8 (R"(°)"),
                                                 NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true);

    oscParams.createAndAddParameter ("roll", "Roll Angle", CharPointer_UTF8 (R"(°)"),
                                                 NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true);

    oscParams.createAndAddParameter ("qw", "Quaternion W", "",
                                                 NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 1.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true);

    oscParams.createAndAddParameter ("qx", "Quaternion X", "",
                                                 NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true);

    oscParams.createAndAddParameter ("qy", "Quaternion Y", "",
                                                 NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true);

    oscParams.createAndAddParameter ("qz", "Quaternion Z", "",
                                                 NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true);



    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("SceneRotator"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened


    // get pointers to the parameters
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    yaw = parameters.getRawParameterValue ("yaw");
    pitch = parameters.getRawParameterValue ("pitch");
    roll = parameters.getRawParameterValue ("roll");
    qw = parameters.getRawParameterValue ("qw");
    qx = parameters.getRawParameterValue ("qx");
    qy = parameters.getRawParameterValue ("qy");
    qz = parameters.getRawParameterValue ("qz");


    // add listeners to parameter changes
    parameters.addParameterListener ("orderSetting", this);
    parameters.addParameterListener ("useSN3D", this);

    parameters.addParameterListener ("yaw", this);
    parameters.addParameterListener ("pitch", this);
    parameters.addParameterListener ("roll", this);
    parameters.addParameterListener ("qw", this);
    parameters.addParameterListener ("qx", this);
    parameters.addParameterListener ("qy", this);
    parameters.addParameterListener ("qz", this);


    oscReceiver.addListener (this);
}

SceneRotatorAudioProcessor::~SceneRotatorAudioProcessor()
{
}

//==============================================================================
const String SceneRotatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SceneRotatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SceneRotatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SceneRotatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SceneRotatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SceneRotatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SceneRotatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SceneRotatorAudioProcessor::setCurrentProgram (int index)
{
}

const String SceneRotatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void SceneRotatorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SceneRotatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *orderSetting, *orderSetting, true);

    // Use this method as the place to do any pre-playback
    // initialisation that you need..



}

void SceneRotatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SceneRotatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void SceneRotatorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *orderSetting, *orderSetting, false);
    ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

   if (rotationParamsHaveChanged.get())
       calcRotationMatrix();
}


void SceneRotatorAudioProcessor::calcRotationMatrix()
{
    rotationParamsHaveChanged = false;
    Vector3D<float> rotAngles { Conversions<float>::degreesToRadians (*yaw), Conversions<float>::degreesToRadians (*pitch), Conversions<float>::degreesToRadians (*roll)};
    Matrix3D<float> rotMat = Matrix3D<float>::rotation (rotAngles);

    DBG (rotMat.mat[0] << "\t" << rotMat.mat[4] << "\t" << rotMat.mat[8]);
    DBG (rotMat.mat[1] << "\t" << rotMat.mat[5] << "\t" << rotMat.mat[9]);
    DBG (rotMat.mat[2] << "\t" << rotMat.mat[6] << "\t" << rotMat.mat[10]);

    DBG ("");
}


//==============================================================================
bool SceneRotatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SceneRotatorAudioProcessor::createEditor()
{
    return new SceneRotatorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void SceneRotatorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    state.setProperty ("OSCPort", var (oscReceiver.getPortNumber()), nullptr);
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}


void SceneRotatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort"))
            {
                oscReceiver.connect (parameters.state.getProperty ("OSCPort", var (-1)));
            }
        }
}

//==============================================================================
void SceneRotatorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG ("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (! updatingParams.get())
    {
        if (parameterID == "qw" || parameterID == "qx" || parameterID == "qy" || parameterID == "qz")
        {
            yprInput = false;
            updateEuler();
            rotationParamsHaveChanged = true;
        }
        else if (parameterID == "yaw" || parameterID == "pitch" || parameterID == "roll")
        {
            yprInput = true;
            updateQuaternions();
            rotationParamsHaveChanged = true;
        }
    }



    if (parameterID == "orderSetting")
        userChangedIOSettings = true;
}

inline void SceneRotatorAudioProcessor::updateQuaternions ()
{
    float ypr[3];
    ypr[0] = Conversions<float>::degreesToRadians (*yaw);
    ypr[1] = Conversions<float>::degreesToRadians (*pitch);
    ypr[2] = Conversions<float>::degreesToRadians (*roll);


    //updating not active params
    iem::Quaternion<float> quaternionDirection;
    quaternionDirection.fromYPR (ypr);

    
    updatingParams = true;
    parameters.getParameter ("qw")->setValue (parameters.getParameterRange ("qw").convertTo0to1 (quaternionDirection.w));
    parameters.getParameter ("qx")->setValue (parameters.getParameterRange ("qx").convertTo0to1 (quaternionDirection.x));
    parameters.getParameter ("qy")->setValue (parameters.getParameterRange ("qy").convertTo0to1 (quaternionDirection.y));
    parameters.getParameter ("qz")->setValue (parameters.getParameterRange ("qz").convertTo0to1 (quaternionDirection.z));
    updatingParams = false;
}

void SceneRotatorAudioProcessor::updateEuler()
{
    float ypr[3];
    auto quaternionDirection = iem::Quaternion<float> (*qw, *qx, *qy, *qz);
    quaternionDirection.normalize();
    quaternionDirection.toYPR(ypr);

    //updating not active params
    updatingParams = true;
    parameters.getParameter ("yaw")->setValue (parameters.getParameterRange ("yaw").convertTo0to1 (Conversions<float>::radiansToDegrees (ypr[0])));
    parameters.getParameter ("pitch")->setValue (parameters.getParameterRange ("pitch").convertTo0to1 (Conversions<float>::radiansToDegrees (ypr[1])));
    parameters.getParameter ("roll")->setValue( parameters.getParameterRange ("roll").convertTo0to1 (Conversions<float>::radiansToDegrees (ypr[2])));
    updatingParams = false;
}



void SceneRotatorAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());
}

//==============================================================================
pointer_sized_int SceneRotatorAudioProcessor::handleVstPluginCanDo (int32 index,
                                                                     pointer_sized_int value, void* ptr, float opt)
{
    auto text = (const char*) ptr;
    auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

    if (matches ("wantsChannelCountNotifications"))
        return 1;
    return 0;
}

//==============================================================================
void SceneRotatorAudioProcessor::oscMessageReceived (const OSCMessage &message)
{
    String prefix ("/" + String (JucePlugin_Name));
    if (! message.getAddressPattern().toString().startsWith (prefix))
        return;

    OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring (String (JucePlugin_Name).length() + 1));

    oscParams.processOSCMessage (msg);
}

void SceneRotatorAudioProcessor::oscBundleReceived (const OSCBundle &bundle)
{
    for (int i = 0; i < bundle.size(); ++i)
    {
        auto elem = bundle[i];
        if (elem.isMessage())
            oscMessageReceived (elem.getMessage());
        else if (elem.isBundle())
            oscBundleReceived (elem.getBundle());
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SceneRotatorAudioProcessor();
}
