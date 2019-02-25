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
StereoEncoderAudioProcessor::StereoEncoderAudioProcessor()
: AudioProcessorBase (
                      #ifndef JucePlugin_PreferredChannelConfigurations
                      BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                 .withInput ("Input", AudioChannelSet::stereo(), true)
#endif
                 .withOutput ("Output", AudioChannelSet::discreteChannels (64), true)
#endif
                 ,
#endif
createParameterLayout()),
posC(1.0f, 0.0f, 0.0f),
posL(1.0f, 0.0f, 0.0f),
posR(1.0f, 0.0f, 0.0f),
updatedPositionData (true)
{
    parameters.addParameterListener("qw", this);
    parameters.addParameterListener("qx", this);
    parameters.addParameterListener("qy", this);
    parameters.addParameterListener("qz", this);
    parameters.addParameterListener("azimuth", this);
    parameters.addParameterListener("elevation", this);
    parameters.addParameterListener("roll", this);
    parameters.addParameterListener("width", this);
    parameters.addParameterListener("orderSetting", this);

    orderSetting = parameters.getRawParameterValue("orderSetting");
    useSN3D = parameters.getRawParameterValue("useSN3D");
    qw = parameters.getRawParameterValue("qw");
    qx = parameters.getRawParameterValue("qx");
    qy = parameters.getRawParameterValue("qy");
    qz = parameters.getRawParameterValue("qz");
    azimuth = parameters.getRawParameterValue("azimuth");
    elevation = parameters.getRawParameterValue("elevation");
    roll = parameters.getRawParameterValue("roll");
    width = parameters.getRawParameterValue("width");
    highQuality = parameters.getRawParameterValue("highQuality");

    processorUpdatingParams = false;

    sphericalInput = true; //input from ypr

    oscReceiver.addListener (this);

    FloatVectorOperations::clear(SHL, 64);
    FloatVectorOperations::clear(SHR, 64);

}

StereoEncoderAudioProcessor::~StereoEncoderAudioProcessor()
= default;

//==============================================================================

int StereoEncoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int StereoEncoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StereoEncoderAudioProcessor::setCurrentProgram (int index)
{
}

const String StereoEncoderAudioProcessor::getProgramName (int index)
{
    return String();
}

void StereoEncoderAudioProcessor::changeProgramName (int index, const String &newName)
{
}

//==============================================================================
void StereoEncoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, 2, *orderSetting, true);

    bufferCopy.setSize(2, samplesPerBlock);

    smoothAzimuthL.setCurrentAndTargetValue (*azimuth / 180.0f * MathConstants<float>::pi);
    smoothElevationL.setCurrentAndTargetValue (*elevation / 180.0f * MathConstants<float>::pi);

    smoothAzimuthR.setCurrentAndTargetValue (*azimuth / 180.0f * MathConstants<float>::pi);
    smoothElevationR.setCurrentAndTargetValue (*elevation / 180.0f * MathConstants<float>::pi);

    smoothAzimuthL.reset(1, samplesPerBlock);
    smoothElevationL.reset(1, samplesPerBlock);
    smoothAzimuthR.reset(1, samplesPerBlock);
    smoothElevationR.reset(1, samplesPerBlock);

    positionHasChanged = true; // just to be sure
}

void StereoEncoderAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


inline void StereoEncoderAudioProcessor::updateQuaternions ()
{
    float ypr[3];
    ypr[0] = Conversions<float>::degreesToRadians(*azimuth);
    ypr[1] = - Conversions<float>::degreesToRadians(*elevation); // pitch
    ypr[2] = Conversions<float>::degreesToRadians(*roll);

    //updating not active params
    quaternionDirection.fromYPR(ypr);
    processorUpdatingParams = true;
    parameters.getParameter ("qw")->setValueNotifyingHost (parameters.getParameterRange ("qw").convertTo0to1 (quaternionDirection.w));
    parameters.getParameter ("qx")->setValueNotifyingHost (parameters.getParameterRange ("qx").convertTo0to1 (quaternionDirection.x));
    parameters.getParameter ("qy")->setValueNotifyingHost (parameters.getParameterRange ("qy").convertTo0to1 (quaternionDirection.y));
    parameters.getParameter ("qz")->setValueNotifyingHost (parameters.getParameterRange ("qz").convertTo0to1 (quaternionDirection.z));
    processorUpdatingParams = false;
}

void StereoEncoderAudioProcessor::updateEuler()
{
    float ypr[3];
    quaternionDirection = iem::Quaternion<float>(*qw, *qx, *qy, *qz);
    quaternionDirection.normalize();
    quaternionDirection.toYPR(ypr);

    //updating not active params
    processorUpdatingParams = true;
    parameters.getParameter ("azimuth")->setValueNotifyingHost (parameters.getParameterRange ("azimuth").convertTo0to1 (Conversions<float>::radiansToDegrees (ypr[0])));
    parameters.getParameter ("elevation")->setValueNotifyingHost (parameters.getParameterRange ("elevation").convertTo0to1 (- Conversions<float>::radiansToDegrees (ypr[1])));
    parameters.getParameter ("roll")->setValueNotifyingHost (parameters.getParameterRange ("roll").convertTo0to1 (Conversions<float>::radiansToDegrees (ypr[2])));
    processorUpdatingParams = false;
}

void StereoEncoderAudioProcessor::processBlock(AudioSampleBuffer &buffer, MidiBuffer &midiMessages) {
    checkInputAndOutput(this, 2, *orderSetting);

    const int L = buffer.getNumSamples();
    const int totalNumInputChannels = getTotalNumInputChannels() < 2 ? 1 : 2;

    const int ambisonicOrder = *orderSetting < 0.5f ? output.getOrder() : roundToInt (*orderSetting) - 1;
    const int nChOut = jmin (buffer.getNumChannels(), square(ambisonicOrder + 1));

    for (int i = 0; i < totalNumInputChannels; ++i)
        bufferCopy.copyFrom(i, 0, buffer.getReadPointer(i), buffer.getNumSamples());
    buffer.clear();

    float xyz[3];
    float xyzL[3];
    float xyzR[3];
    quaternionDirection.toCartesian(xyz);

    const float widthInRadiansQuarter {Conversions<float>::degreesToRadians(*width) / 4.0f};
    iem::Quaternion<float> quatLRot {iem::Quaternion<float>(cos(widthInRadiansQuarter), 0.0f, 0.0f, sin(widthInRadiansQuarter))};
    iem::Quaternion<float> quatL = quaternionDirection * quatLRot;
    iem::Quaternion<float> quatR = quaternionDirection * (quatLRot.getConjugate());

    quatL.toCartesian(xyzL);
    quatR.toCartesian(xyzR);

    //conversion to spherical for high-quality mode
    float azimuthL, azimuthR, elevationL, elevationR;
    Conversions<float>::cartesianToSpherical(xyzL[0], xyzL[1], xyzL[2], azimuthL, elevationL);
    Conversions<float>::cartesianToSpherical(xyzR[0], xyzR[1], xyzR[2], azimuthR, elevationR);


    if (*highQuality < 0.5f) // no high-quality
    {
        if (positionHasChanged.compareAndSetBool (false, true))
        {
            smoothAzimuthL.setCurrentAndTargetValue (azimuthL);
            smoothElevationL.setCurrentAndTargetValue (elevationL);
            smoothAzimuthR.setCurrentAndTargetValue (azimuthR);
            smoothElevationR.setCurrentAndTargetValue (elevationR);

            SHEval(ambisonicOrder, xyzL[0], xyzL[1], xyzL[2], SHL);
            SHEval(ambisonicOrder, xyzR[0], xyzR[1], xyzR[2], SHR);

            if (*useSN3D > 0.5f)
            {
                FloatVectorOperations::multiply(SHL, SHL, n3d2sn3d, nChOut);
                FloatVectorOperations::multiply(SHR, SHR, n3d2sn3d, nChOut);
            }
        }
        const float *leftIn = bufferCopy.getReadPointer(0);
        const float *rightIn = bufferCopy.getReadPointer(1);
        for (int i = 0; i < nChOut; ++i)
        {
            buffer.copyFromWithRamp(i, 0, leftIn, buffer.getNumSamples(), _SHL[i], SHL[i]);
            buffer.addFromWithRamp(i, 0, rightIn, buffer.getNumSamples(), _SHR[i], SHR[i]);
        }
    }
    else // high-quality sampling
    {
        if (smoothAzimuthL.getTargetValue() - azimuthL > MathConstants<float>::pi)
            smoothAzimuthL.setCurrentAndTargetValue (smoothAzimuthL.getTargetValue() - 2.0f * MathConstants<float>::pi);
        else if (azimuthL - smoothAzimuthL.getTargetValue() > MathConstants<float>::pi)
            smoothAzimuthL.setCurrentAndTargetValue (smoothAzimuthL.getTargetValue() + 2.0f * MathConstants<float>::pi);

        if (smoothElevationL.getTargetValue() - elevationL > MathConstants<float>::pi)
            smoothElevationL.setCurrentAndTargetValue (smoothElevationL.getTargetValue() - 2.0f * MathConstants<float>::pi);
        else if (elevationL - smoothElevationL.getTargetValue() > MathConstants<float>::pi)
            smoothElevationL.setCurrentAndTargetValue (smoothElevationL.getTargetValue() + 2.0f * MathConstants<float>::pi);

        if (smoothAzimuthR.getTargetValue() - azimuthR > MathConstants<float>::pi)
            smoothAzimuthR.setCurrentAndTargetValue (smoothAzimuthR.getTargetValue() - 2.0f * MathConstants<float>::pi);
        else if (azimuthR - smoothAzimuthR.getTargetValue() > MathConstants<float>::pi)
            smoothAzimuthR.setCurrentAndTargetValue (smoothAzimuthR.getTargetValue() + 2.0f * MathConstants<float>::pi);

        if (smoothElevationR.getTargetValue() - elevationR > MathConstants<float>::pi)
            smoothElevationR.setCurrentAndTargetValue (smoothElevationR.getTargetValue() - 2.0f * MathConstants<float>::pi);
        else if (elevationR - smoothElevationR.getTargetValue() > MathConstants<float>::pi)
            smoothElevationR.setCurrentAndTargetValue (smoothElevationR.getTargetValue() + 2.0f * MathConstants<float>::pi);

        smoothAzimuthL.setTargetValue (azimuthL);
        smoothElevationL.setTargetValue (elevationL);
        smoothAzimuthR.setTargetValue (azimuthR);
        smoothElevationR.setTargetValue (elevationR);

        for (int i = 0; i < L; ++i) // left
        {
            const float azimuth = smoothAzimuthL.getNextValue();
            const float elevation = smoothElevationL.getNextValue();
            float sample = bufferCopy.getSample(0, i);

            const Vector3D<float> pos = Conversions<float>::sphericalToCartesian(azimuth, elevation);
            SHEval(ambisonicOrder, pos.x, pos.y, pos.z, SHL);

            for (int ch = 0; ch < nChOut; ++ch)
                buffer.setSample(ch, i, sample * SHL[ch]);
        }

        for (int i = 0; i < L; ++i) // right
        {
            const float azimuth = smoothAzimuthR.getNextValue();
            const float elevation = smoothElevationR.getNextValue();
            float sample = bufferCopy.getSample(1, i);

            const Vector3D<float> pos = Conversions<float>::sphericalToCartesian(azimuth, elevation);
            SHEval(ambisonicOrder, pos.x, pos.y, pos.z, SHR);

            for (int ch = 0; ch < nChOut; ++ch)
                buffer.addSample(ch, i, sample * SHR[ch]);
        }

        if (*useSN3D > 0.5f)
        {
            for (int ch = 0; ch < nChOut; ++ch)
            {
                buffer.applyGain(ch, 0, L, n3d2sn3d[ch]);
            }

            FloatVectorOperations::multiply(SHL, SHL, n3d2sn3d, nChOut);
            FloatVectorOperations::multiply(SHR, SHR, n3d2sn3d, nChOut);
        }
    }
    FloatVectorOperations::copy(_SHL, SHL, nChOut);
    FloatVectorOperations::copy(_SHR, SHR, nChOut);
}

//==============================================================================
bool StereoEncoderAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor *StereoEncoderAudioProcessor::createEditor() {
    return new StereoEncoderAudioProcessorEditor(*this, parameters);
}

void StereoEncoderAudioProcessor::parameterChanged (const String &parameterID, float newValue) {
    if (!processorUpdatingParams) {
        if (parameterID == "qw" || parameterID == "qx" || parameterID == "qy" || parameterID == "qz")
        {
            sphericalInput = false;
            updateEuler();
            updatedPositionData = true;
            positionHasChanged = true;
        }
        else if (parameterID == "azimuth" || parameterID == "elevation" || parameterID == "roll")
        {
            sphericalInput = true;
            updateQuaternions();
            updatedPositionData = true;
            positionHasChanged = true;
        }
        else if (parameterID == "width")
        {
            updatedPositionData = true;
            positionHasChanged = true;
        }
    }
    if (parameterID == "orderSetting")
    {
        userChangedIOSettings = true;
        positionHasChanged = true;
    }
}


//==============================================================================
void StereoEncoderAudioProcessor::getStateInformation (MemoryBlock &destData)
{
    auto state = parameters.copyState();
    state.setProperty ("OSCPort", var(oscReceiver.getPortNumber()), nullptr);
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void StereoEncoderAudioProcessor::setStateInformation (const void *data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort"))
            {
                oscReceiver.connect (parameters.state.getProperty ("OSCPort", var(-1)));
            }
        }
}


//==============================================================================


const bool StereoEncoderAudioProcessor::processNotYetConsumedOSCMessage (const OSCMessage &message)
{
    String prefix ("/" + String (JucePlugin_Name));
    if (! message.getAddressPattern().toString().startsWith (prefix))
        return false;

    OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring (String (JucePlugin_Name).length() + 1));

    if (msg.getAddressPattern().toString().equalsIgnoreCase ("/quaternions") && msg.size() == 4)
    {
        float qs[4];
        for (int i = 0; i < 4; ++i)
            if (msg[i].isFloat32())
                qs[i] = msg[i].getFloat32();
            else if (msg[i].isInt32())
                qs[i] = msg[i].getInt32();

        oscParameterInterface.setValue ("qw", qs[0]);
        oscParameterInterface.setValue ("qx", qs[1]);
        oscParameterInterface.setValue ("qy", qs[2]);
        oscParameterInterface.setValue ("qz", qs[3]);

        return true;
    }

    return false;
}



//==============================================================================
std::vector<std::unique_ptr<RangedAudioParameter>> StereoEncoderAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<RangedAudioParameter>> params;



    params.push_back (OSCParameterInterface::createParameterTheOldWay ("orderSetting", "Ambisonics Order", "",
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
                                                 }, nullptr));


    params.push_back (OSCParameterInterface::createParameterTheOldWay ("useSN3D", "Normalization", "",
                                                 NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                                 [](float value) {
                                                     if (value >= 0.5f) return "SN3D";
                                                     else return "N3D";
                                                 }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("qw", "Quaternion W", "",
                                                 NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 1.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("qx", "Quaternion X", "",
                                                 NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("qy", "Quaternion Y", "",
                                                 NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("qz", "Quaternion Z", "",
                                                 NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("azimuth", "Azimuth Angle", CharPointer_UTF8 (R"(°)"),
                                                 NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("elevation", "Elevation Angle", CharPointer_UTF8 (R"(°)"),
                                                 NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("roll", "Roll Angle", CharPointer_UTF8 (R"(°)"),
                                                 NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr, true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("width", "Stereo Width", CharPointer_UTF8 (R"(°)"),
                                                 NormalisableRange<float>(-360.0f, 360.0f, 0.01f), 0.0,
                                                 [](float value) { return String(value, 2); }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("highQuality", "Sample-wise Panning", "",
                                                 NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f,
                                                 [](float value) { return value < 0.5f ? "OFF" : "ON"; }, nullptr));


    return params;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new StereoEncoderAudioProcessor();
}
