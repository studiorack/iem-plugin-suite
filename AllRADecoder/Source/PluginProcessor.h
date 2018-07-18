/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich, Franz Zotter
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

#pragma once

#ifndef M_PI
    #define M_PI 3.141592654
#endif

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/IOHelper.h"
#include "../../resources/customComponents/MailBox.h"
#include "../../resources/NewtonApple/NewtonApple_hull3D.h"
#include "tDesign5200.h"
#include "../../resources/efficientSHvanilla.h"
#include "../../resources/ReferenceCountedDecoder.h"
#include "../../resources/AmbisonicDecoder.h"

#define CONFIGURATIONHELPER_ENABLE_DECODER_METHODS 1
#define CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS 1
#include "../../resources/ConfigurationHelper.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/HammerAitov.h"
#include "NoiseBurst.h"
#include "AmbisonicNoiseBurst.h"

// ==== OSC
#include "../../resources/OSCParameterInterface.h"
#include "../../resources/OSCReceiverPlus.h"


//==============================================================================
/**
 Use the IOHelper to detect which amount of channels or which Ambisonic order is possible with the current bus layout.
 The available IOTypes are:
    - AudioChannels<maxChannelCount>
    - Ambisonics<maxOrder> (can also be used for directivity signals)
 You can leave `maxChannelCount` and `maxOrder` empty for default values (64 channels and 7th order)
*/

using namespace dsp;
class AllRADecoderAudioProcessor  : public AudioProcessor,
                                        public AudioProcessorValueTreeState::Listener,
                                        public IOHelper<IOTypes::Ambisonics<7>, IOTypes::AudioChannels<64>>,
                                        public ValueTree::Listener, public VSTCallbackHandler,
                                        private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
    //==============================================================================
    AllRADecoderAudioProcessor();
    ~AllRADecoderAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect () const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void parameterChanged (const String &parameterID, float newValue) override;
    void updateBuffers() override; // use this to implement a buffer update method

    //======== PluginCanDo =========================================================
    pointer_sized_int handleVstManufacturerSpecific (int32 index, pointer_sized_int value,
                                                     void* ptr, float opt) override { return 0; };
    pointer_sized_int handleVstPluginCanDo (int32 index, pointer_sized_int value,
                                            void* ptr, float opt) override;


    //==============================================================================
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;
    void valueTreeChildAdded (ValueTree &parentTree, ValueTree &childWhichHasBeenAdded) override;
    void valueTreeChildRemoved (ValueTree &parentTree, ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
    void valueTreeChildOrderChanged (ValueTree &parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;
    void valueTreeParentChanged (ValueTree &treeWhoseParentHasChanged) override;

    void playNoiseBurst (const int channel);
    void playAmbisonicNoiseBurst (const float azimuth, const float elevation);
    void addRandomPoint();
    void addImaginaryLoudspeakerBelow();

    void undo() { undoManager.undo(); }
    void redo() { undoManager.redo(); }

    void rotate (const float degreesAddedToAzimuth);

    void saveConfigurationToFile (File destination);
    void loadConfiguration (const File& presetFile);

    ValueTree& getLoudspeakersValueTree() { return loudspeakers; }

    var lsps;
    Atomic<bool> updateLoudspeakerVisualization = false;
    Atomic<bool> updateTable = true;
    Atomic<bool> updateMessage = true;
    Atomic<bool> updateChannelCount = true;

    ReferenceCountedDecoder::Ptr getCurrentDecoder() {return decoderConfig;}

    std::vector<R3> points;
    std::vector<Tri> triangles;
    std::vector<Vector3D<float>> normals;

    BigInteger imaginaryFlags;
    UndoManager undoManager;

    Result calculateDecoder();

    void setLastDir(File newLastDir);
    File getLastDir() {return lastDir;};

    Image energyDistribution;
    Image rEVector;

    MailBox::Message messageToEditor;

    //======== OSC =================================================================
    void oscMessageReceived (const OSCMessage &message) override;
    OSCReceiverPlus& getOSCReceiver () { return oscReceiver; }
    //==============================================================================

private:
    // ====== parameters and osc
    AudioProcessorValueTreeState parameters;
    OSCParameterInterface oscParams;
    OSCReceiverPlus oscReceiver;

    // list of used audio parameters
    float* inputOrderSetting;
    float* useSN3D;
    float* decoderOrder;
    float* exportDecoder;
    float* exportLayout;

    ValueTree loudspeakers {"Loudspeakers"};

    AmbisonicDecoder decoder;
    ReferenceCountedDecoder::Ptr decoderConfig {nullptr};

    bool isLayoutReady = false;

    int highestChannelNumber;

    File lastDir;
    ScopedPointer<PropertiesFile> properties;

    // ========== METHODS
    void prepareLayout();
    Result checkLayout();
    Result verifyLoudspeakers();
    Result calculateTris();
    void convertLoudspeakersToArray();

    float getKappa(float gIm, float gRe1, float gRe2, int N);
    Matrix<float> getInverse(Matrix<float> A);

    ValueTree createLoudspeakerFromCartesian (Vector3D<float> cartesianCoordinates, int channel, bool isImaginary = false, float gain = 1.0f);
    ValueTree createLoudspeakerFromSpherical (Vector3D<float> sphericalCoordinates, int channel, bool isImaginary = false, float gain = 1.0f);
    Vector3D<float> cartesianToSpherical (Vector3D<float> cartvect);
    Vector3D<float> sphericalToCartesian (Vector3D<float> sphervect);
    Vector3D<float> sphericalInRadiansToCartesian (Vector3D<float> sphervect);

    NoiseBurst noiseBurst;
    AmbisonicNoiseBurst ambisonicNoiseBurst;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AllRADecoderAudioProcessor)
};
