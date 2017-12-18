/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 http://www.iem.at
 
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

#include "PluginProcessor.h"
#include "PluginEditor.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


//==============================================================================
ProbeDecoderAudioProcessor::ProbeDecoderAudioProcessor()


: AudioProcessor(BusesProperties()
                 .withInput("Input", AudioChannelSet::discreteChannels(64), true)
                 .withOutput("Output", AudioChannelSet::mono(), true)
                 ),
parameters(*this, nullptr) {
    parameters.createAndAddParameter("orderSetting", "Ambisonics Order", "",
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
    parameters.createAndAddParameter("useSN3D", "Normalization", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "SN3D";
                                         else return "N3D";
                                     }, nullptr);
    
    parameters.createAndAddParameter("yaw", "Yaw angle", "deg",
                                     NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) { return String(value); }, nullptr);
    parameters.createAndAddParameter("pitch", "Pitch angle", "deg",
                                     NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
                                     [](float value) { return String(value); }, nullptr);
    
    
    
    parameters.state = ValueTree(Identifier("ProbeDecoder"));
    
    orderSetting = parameters.getRawParameterValue("orderSetting");
    useSN3D = parameters.getRawParameterValue("useSN3D");
    
    parameters.addParameterListener("orderSetting", this);
    
    yaw = parameters.getRawParameterValue("yaw");
    pitch = parameters.getRawParameterValue("pitch");
    
    FloatVectorOperations::clear(previousSH, 64);
}

ProbeDecoderAudioProcessor::~ProbeDecoderAudioProcessor()
= default;

//==============================================================================
const String ProbeDecoderAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool ProbeDecoderAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ProbeDecoderAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double ProbeDecoderAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int ProbeDecoderAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int ProbeDecoderAudioProcessor::getCurrentProgram() {
    return 0;
}

void ProbeDecoderAudioProcessor::setCurrentProgram(int index) {
}

const String ProbeDecoderAudioProcessor::getProgramName(int index) {
    return String();
}

void ProbeDecoderAudioProcessor::changeProgramName(int index, const String &newName) {
}

//==============================================================================
void ProbeDecoderAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    checkOrderUpdateBuffers(roundFloatToInt(*orderSetting - 1));
}

void ProbeDecoderAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations

bool ProbeDecoderAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    return true;
}

#endif

void ProbeDecoderAudioProcessor::processBlock(AudioSampleBuffer &buffer, MidiBuffer &midiMessages) {
    if (userChangedOrderSettings) checkOrderUpdateBuffers(roundFloatToInt(*orderSetting - 1));
    
    
    float yawInRad = degreesToRadians(*yaw);
    float pitchInRad = degreesToRadians(*pitch);
    float cosPitch = cosf(yawInRad);
    Vector3D<float> xyz(cosPitch * cosf(yawInRad), cosPitch * sinf(yawInRad), sinf(-1.0f * pitchInRad));
    
    float sh[64];
    
    SHEval(ambisonicOrder, xyz.x, xyz.y, xyz.z, sh);
    const int nCh = jmin(buffer.getNumChannels(), nChannels);
    const int numSamples = buffer.getNumSamples();
    
    if (*useSN3D > 0.5f)
        FloatVectorOperations::multiply(sh, sh, sn3d2n3d, nChannels);
    
    
    buffer.applyGainRamp(0, 0, numSamples, previousSH[0], sh[0]);
    
    
    for (int i = 1; i < nCh; i++) {
        buffer.addFromWithRamp(0, 0, buffer.getReadPointer(i), numSamples, previousSH[i], sh[i]);
    }
    
    FloatVectorOperations::copy(previousSH, sh, nChannels);
}

//==============================================================================
bool ProbeDecoderAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor *ProbeDecoderAudioProcessor::createEditor() {
    return new ProbeDecoderAudioProcessorEditor(*this, parameters);
}

void ProbeDecoderAudioProcessor::parameterChanged(const String &parameterID, float newValue) {
    if (parameterID == "orderSetting") userChangedOrderSettings = true;
}


//==============================================================================
void ProbeDecoderAudioProcessor::getStateInformation(MemoryBlock &destData) {
    //MemoryOutputStream (destData, true).writeFloat (*qw);
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ProbeDecoderAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    //*qw = MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat();
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new ProbeDecoderAudioProcessor();
}

void ProbeDecoderAudioProcessor::checkOrderUpdateBuffers(int userSetInputOrder) {
    userChangedOrderSettings = false;
    //old values;
    _nChannels = nChannels;
    _ambisonicOrder = ambisonicOrder;
    
    maxPossibleOrder = isqrt(getTotalNumInputChannels()) - 1;
    if (userSetInputOrder == -1 || userSetInputOrder > maxPossibleOrder)
        ambisonicOrder = maxPossibleOrder; // Auto setting or requested order exceeds highest possible order
    else ambisonicOrder = userSetInputOrder;
    
    if (ambisonicOrder != _ambisonicOrder) {
        nChannels = squares[ambisonicOrder + 1];
        DBG("Used order has changed! Order: " << ambisonicOrder << ", numCH: " << nChannels);
        DBG("Now updating filters and buffers.");
    }
}

