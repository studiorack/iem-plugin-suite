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
EnergyVisualizerAudioProcessor::EnergyVisualizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels(64), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
                     #endif
                       ),
#endif
parameters(*this, nullptr), oscParams (parameters)
{
    oscParams.createAndAddParameter ("orderSetting", "Ambisonics Order", "",
                                      NormalisableRange<float> (0.0f, 8.0f, 1.0f), 0.0f,
                                      [](float value)
                                      {
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

    oscParams.createAndAddParameter("peakLevel", "Peak level", "dB",
                                     NormalisableRange<float> (-50.0f, 10.0f, 0.1f), 0.0,
                                     [](float value) {return String(value, 1);}, nullptr);


    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    peakLevel = parameters.getRawParameterValue ("peakLevel");

    parameters.addParameterListener ("orderSetting", this);

    parameters.state = ValueTree (Identifier ("EnergyVisualizer"));

    Eigen::Matrix<float,64,nSamplePoints> Y;
    // calc Y and YH
    for (int point=0; point<nSamplePoints; ++point)
    {
        SHEval(7, hammerAitovSampleX[point], hammerAitovSampleY[point], hammerAitovSampleZ[point], Y.data() + point * 64, false);
        FloatVectorOperations::multiply(Y.data()+point*64, Y.data()+point*64, sn3d2n3d, 64); //expecting sn3d normalization -> converting it to n3d
    }
    Y *= 1.0f / decodeCorrection(7); // revert 7th order correction
    YH = Y.transpose();

//    DBG(hammerAitovSampleX[218] << " - " << hammerAitovSampleY[218] << " - " << hammerAitovSampleZ[218]);
    rms.resize(nSamplePoints);

    oscReceiver.addListener (this);
}

EnergyVisualizerAudioProcessor::~EnergyVisualizerAudioProcessor()
{
}

//==============================================================================
const String EnergyVisualizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EnergyVisualizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EnergyVisualizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EnergyVisualizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EnergyVisualizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EnergyVisualizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EnergyVisualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EnergyVisualizerAudioProcessor::setCurrentProgram (int index)
{
}

const String EnergyVisualizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void EnergyVisualizerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void EnergyVisualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *orderSetting, 0, true);

    timeConstant = exp(-1.0/(sampleRate*0.1/samplesPerBlock)); //multiplicated value after sampleRate is rms time
    sampledSignals.setSize(nSamplePoints, samplesPerBlock);
}

void EnergyVisualizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EnergyVisualizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void EnergyVisualizerAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    checkInputAndOutput(this, *orderSetting, 0);

    //const int nCh = buffer.getNumChannels();
    const int L = buffer.getNumSamples();
    const int workingOrder = jmin(isqrt(buffer.getNumChannels())-1, input.getOrder());

    const int nCh = squares[workingOrder+1];
    //DBG(buffer.getNumChannels() << " - " << workingOrder << " - " << nCh);


    Eigen::Map<const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> inpMatrix (buffer.getReadPointer(0),nCh,L);

    Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> outMatrix (sampledSignals.getWritePointer(0), nSamplePoints,L);

    //outMatrix = YH.block(0,0,tDesignN,nCh) * inpMatrix;
    FloatVectorOperations::clear(&maxReWeights.diagonal()[0],64);
    copyMaxRE(workingOrder, &maxReWeights.diagonal()[0]);
    FloatVectorOperations::multiply(&maxReWeights.diagonal()[0], maxRECorrection[workingOrder] * decodeCorrection(workingOrder), nCh);

    if (*useSN3D < 0.5f)
    {
        FloatVectorOperations::multiply(&maxReWeights.diagonal()[0], n3d2sn3d, 64);
    }

    workingMatrix = YH * maxReWeights;
    //workingMatrix = YH; // * maxReWeights;
    outMatrix = workingMatrix.block(0, 0, nSamplePoints, nCh) * inpMatrix;

    float* pRms = rms.getRawDataPointer();
    float oneMinusTimeConstant = 1.0f - timeConstant;
    for (int i = 0; i < nSamplePoints; ++i)
    {
        pRms[i] = timeConstant * pRms[i] + oneMinusTimeConstant * ((Decibels::gainToDecibels(sampledSignals.getRMSLevel(i, 0, L)) - *peakLevel) / 35.0f + 1.0f);
    }
    FloatVectorOperations::clip(pRms, rms.getRawDataPointer(), 0.0f, 1.0f, nSamplePoints);
}

//==============================================================================
bool EnergyVisualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* EnergyVisualizerAudioProcessor::createEditor()
{
    return new EnergyVisualizerAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void EnergyVisualizerAudioProcessor::setStateInformation (const void *data, int sizeInBytes)
{
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

void EnergyVisualizerAudioProcessor::getStateInformation (MemoryBlock &destData)
{
    auto state = parameters.copyState();
    state.setProperty ("OSCPort", var(oscReceiver.getPortNumber()), nullptr);
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

//==============================================================================
void EnergyVisualizerAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "orderSetting") userChangedIOSettings = true;
}

//==============================================================================
pointer_sized_int EnergyVisualizerAudioProcessor::handleVstPluginCanDo (int32 index,
                                                                     pointer_sized_int value, void* ptr, float opt)
{
    auto text = (const char*) ptr;
    auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

    if (matches ("wantsChannelCountNotifications"))
        return 1;
    return 0;
}

//==============================================================================
void EnergyVisualizerAudioProcessor::oscMessageReceived (const OSCMessage &message)
{
    String prefix ("/" + String(JucePlugin_Name));
    if (! message.getAddressPattern().toString().startsWith (prefix))
        return;

    OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring(String(JucePlugin_Name).length() + 1));

    oscParams.processOSCMessage (msg);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnergyVisualizerAudioProcessor();
}
