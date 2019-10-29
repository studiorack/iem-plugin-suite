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

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cfloat>

const StringArray AllRADecoderAudioProcessor::weightsStrings = StringArray ("basic", "maxrE", "inphase");

//==============================================================================
AllRADecoderAudioProcessor::AllRADecoderAudioProcessor()
: AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
                      BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::discreteChannels (64), true)
#endif
                  .withOutput ("Output", AudioChannelSet::discreteChannels (64), true)
#endif
                  ,
#endif
createParameterLayout()),
energyDistribution (Image::PixelFormat::ARGB, 200, 100, true), rEVector (Image::PixelFormat::ARGB, 200, 100, true)
{
    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue ("inputOrderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    decoderOrder = parameters.getRawParameterValue ("decoderOrder");
    exportDecoder = parameters.getRawParameterValue ("exportDecoder");
    exportLayout = parameters.getRawParameterValue ("exportLayout");
    weights = parameters.getRawParameterValue ("weights");

    // add listeners to parameter changes
    parameters.addParameterListener ("inputOrderSetting", this);
    parameters.addParameterListener ("useSN3D", this);

    // global properties
    PropertiesFile::Options options;
    options.applicationName     = "AllRADecoder";
    options.filenameSuffix      = "settings";
    options.folderName          = "IEM";
    options.osxLibrarySubFolder = "Preferences";

    properties.reset (new PropertiesFile (options));
    lastDir = File(properties->getValue("presetFolder"));

    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float> (1.0f, 0.0f, 0.0f), 1), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float> (1.0f, 45.0f, 0.0f), 2, true), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float> (1.0f, 90.0f, 0.0f), 3), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(1.0f, 135.0f, 0.0f), 4), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(1.0f, 180.0f, 0.0f), 5), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(1.0f, -135.0f, 0.0f), 6), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float> (1.0f, -90.0f, 0.0f), 7), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float> (1.0f, -45.0f, 0.0f), 8), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(1.0f, 22.5f, 40.0f), 9), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(1.0f, 142.5f, 40.0f), 10), &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(1.0f, -97.5f, 40.0f), 11), &undoManager);

    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(1.0f, 0.0f, -90.0f), 12), &undoManager);

    loudspeakers.addListener(this);
    prepareLayout();
}

AllRADecoderAudioProcessor::~AllRADecoderAudioProcessor()
{
}


int AllRADecoderAudioProcessor::getNumPrograms()
{
    return 2;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AllRADecoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AllRADecoderAudioProcessor::setCurrentProgram (int index)
{
    if (index == 1)
    {
        loudspeakers.removeListener(this);

        undoManager.beginNewTransaction();
        loudspeakers.removeAllChildren(&undoManager);

        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(4.63f, 0.0f, 0.0f), 1), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.0252f, -23.7f, 0.0f), 2), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(6.1677f, -48.17f, 0.0f), 3), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.26f, -72.17f, 0.0f), 4), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.7f, -103.0f, 0.0f), 5), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.84f, -138.0f, 0.0f), 6), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(4.63f, -180.0f, 0.0f), 7), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.8f, 138.0f, 0.0f), 8), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.63f, 101.0f, 0.0f), 9), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.3f, 70.0f, 0.0f), 10), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(6.3f, 45.0f, 0.0f), 11), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.1f, 21.0f, 0.0f), 12), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.2f, -22.0f, 28.0f), 13), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.4f, -68.0f, 28.0f), 14), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.1f, -114.0f, 28.0f), 15), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(4.43f, -158.0f, 28.0f), 16), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(4.45f, 156.0f, 28.0f), 17), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.9f, 113.0f, 28.0f), 18), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.2f, 65.0f, 28.0f), 19), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(5.2f, 22.0f, 28.0f), 20), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(3.6f, -47.0f, 56.0f), 21), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(3.3f, -133.0f, 56.0f), 22), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(3.3f, 133.0f, 56.0f), 23), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(3.55f, 43.0f, 56.0f), 24), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(1.0f, 0.0f, -90.0f), 24, true, 0.0f), &undoManager);
        loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float>(1.0f, 0.0f, 45.0f), 25, true, 1.0f), &undoManager);

        loudspeakers.addListener(this);
        prepareLayout();
        updateTable = true;
    }
}

const String AllRADecoderAudioProcessor::getProgramName (int index)
{
    if (index == 1)
        return "IEM CUBE";
    else
        return "default";
}

void AllRADecoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void AllRADecoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *inputOrderSetting, 64, true);

    ProcessSpec specs;
    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = samplesPerBlock;
    specs.numChannels = 64;

    decoder.prepare(specs);
    noiseBurst.prepare(specs);
    ambisonicNoiseBurst.prepare(specs);
}

void AllRADecoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


void AllRADecoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputOrderSetting, 64, false);
    ScopedNoDenormals noDenormals;

    // ====== is a decoder loaded? stop processing if not ===========
    decoder.checkIfNewDecoderAvailable();
    if (decoder.getCurrentDecoder() == nullptr)
    {
        buffer.clear();
    }
    else
    {
        ambisonicNoiseBurst.processBuffer (buffer);

        const int nChIn = jmin(decoder.getCurrentDecoder()->getNumInputChannels(), buffer.getNumChannels(), input.getNumberOfChannels());
        const int nChOut = jmin(decoder.getCurrentDecoder()->getNumOutputChannels(), buffer.getNumChannels());

        for (int ch = jmax(nChIn, nChOut); ch < buffer.getNumChannels(); ++ch) // clear all not needed channels
            buffer.clear(ch, 0, buffer.getNumSamples());

        decoder.setInputNormalization(*useSN3D >= 0.5f ? ReferenceCountedDecoder::Normalization::sn3d : ReferenceCountedDecoder::Normalization::n3d);

        const int L = buffer.getNumSamples();
        AudioBlock<float> inputAudioBlock = AudioBlock<float>(buffer.getArrayOfWritePointers(), nChIn, L);
        AudioBlock<float> outputAudioBlock = AudioBlock<float>(buffer.getArrayOfWritePointers(), nChOut, L);

        decoder.process (inputAudioBlock, outputAudioBlock);

        for (int ch = nChOut; ch < buffer.getNumChannels(); ++ch) // clear all not needed channels
            buffer.clear (ch, 0, buffer.getNumSamples());

    }
    noiseBurst.processBuffer (buffer);
}

//==============================================================================
bool AllRADecoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AllRADecoderAudioProcessor::createEditor()
{
    return new AllRADecoderAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void AllRADecoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    if (parameters.state.getChildWithName("Loudspeakers").isValid() && parameters.state.getChildWithName("Loudspeakers") != loudspeakers)
    {
        parameters.state.removeChild(parameters.state.getChildWithName("Loudspeakers"), nullptr);
    }
    parameters.state.appendChild (loudspeakers, nullptr);

    auto state = parameters.copyState();
    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}



void AllRADecoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort")) // legacy
            {
                oscParameterInterface.getOSCReceiver().connect (parameters.state.getProperty ("OSCPort", var (-1)));
                parameters.state.removeProperty ("OSCPort", nullptr);
            }

            auto oscConfig = parameters.state.getChildWithName ("OSCConfig");
            if (oscConfig.isValid())
                oscParameterInterface.setConfig (oscConfig);
        }

        XmlElement* lsps (xmlState->getChildByName("Loudspeakers"));
        if (lsps != nullptr)
        {
            loudspeakers.removeListener(this);
            loudspeakers.removeAllChildren(nullptr);
            const int nChilds = lsps->getNumChildElements();
            for (int i = 0; i < nChilds; ++i)
            {
                XmlElement* lsp (lsps->getChildElement(i));
                if (lsp->getTagName() == "Element" || lsp->getTagName() == "Loudspeaker")
                    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float> (lsp->getDoubleAttribute("Radius", 1.0),
                                                                                             lsp->getDoubleAttribute("Azimuth"),
                                                                                             lsp->getDoubleAttribute("Elevation")),
                                                                            lsp->getIntAttribute("Channel", -1),
                                                                            lsp->getBoolAttribute("Imaginary", false),
                                                                            lsp->getDoubleAttribute("Gain", 1.0)
                                                                            ), &undoManager);
            }
            undoManager.clearUndoHistory();
            loudspeakers.addListener(this);
            prepareLayout();
            updateTable = true;
            calculateDecoder();
        }
    }

}

//==============================================================================
void AllRADecoderAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "inputOrderSetting" || parameterID == "outputOrderSetting" )
        userChangedIOSettings = true;
    else if (parameterID == "useSN3D")
    {
        decoder.setInputNormalization(*useSN3D >= 0.5f ? ReferenceCountedDecoder::Normalization::sn3d : ReferenceCountedDecoder::Normalization::n3d);
    }
}

void AllRADecoderAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AllRADecoderAudioProcessor();
}

Result AllRADecoderAudioProcessor::verifyLoudspeakers()
{
    const int nLsps = loudspeakers.getNumChildren();
    if (nLsps == 0)
        return Result::fail("There are no loudspeakers.");

    for (int i = 0; i < nLsps; ++i)
    {
        ValueTree lsp = loudspeakers.getChild(i);
        if (! lsp.isValid())
            return Result::fail("Something went wrong. Try again!");


        if (lsp.hasProperty("Azimuth"))
        {
            const var azimuth = lsp.getProperty("Azimuth", var());
            if (! azimuth.isDouble() && ! azimuth.isInt())
                return Result::fail("Loudspeaker #" + String(i+1) + ": 'Azimuth' value is neither a double nor an integer.");
        }
        else
            return Result::fail("Loudspeaker #" + String(i+1) + ": Missing 'Azumith' attribute.");


        if (lsp.hasProperty("Elevation")) //mandatory
        {
            const var elevation = lsp.getProperty("Elevation", var());
            if (! elevation.isDouble() && ! elevation.isInt())
                return Result::fail("Loudspeaker #" + String(i+1) + ": 'Elevation' value is neither a double nor an integer.");
        }
        else
            return Result::fail("Loudspeaker #" + String(i+1) + ": Missing 'Elevation' attribute.");


        if (lsp.hasProperty("Radius")) //optional
        {
            const var radius = lsp.getProperty("Radius", var());
            if (! radius.isDouble() && ! radius.isInt())
                return Result::fail("Loudspeaker #" + String(i+1) + ": 'Radius' value is neither a double nor an integer.");
            if ((float) radius < FLT_EPSILON)
                return Result::fail("Loudspeaker #" + String(i+1) + ": Radius has to be greater than zero.");
        }

        bool isImaginary = false;
        if (lsp.hasProperty("IsImaginary")) //optional
        {
            const var imaginary = lsp.getProperty("IsImaginary", var());
            if (! imaginary.isBool())
                return Result::fail("Loudspeaker #" + String(i+1) + ": 'IsImaginary' value is not a bool.");
            isImaginary = imaginary;
        }

        if (! isImaginary)
        {
            if (lsp.hasProperty("Channel")) //mandatory
            {
                const var channel = lsp.getProperty("Channel", var());
                if (! channel.isInt())
                    return Result::fail("Loudspeaker #" + String(i+1) + ": 'Channel' value is not an integer.");
            }
            else
                return Result::fail("Loudspeaker #" + String(i+1) + ": Missing 'Channel' attribute.");
        }
    }

    return Result::ok();
}



Result AllRADecoderAudioProcessor::calculateTris()
{
    return Result::ok();
}

ValueTree AllRADecoderAudioProcessor::createLoudspeakerFromCartesian (Vector3D<float> cartCoordinates, int channel, bool isImaginary, float gain)
{
    Vector3D<float> sphericalCoordinates = cartesianToSpherical(cartCoordinates);
    return createLoudspeakerFromSpherical(sphericalCoordinates, channel, isImaginary, gain);
}

ValueTree AllRADecoderAudioProcessor::createLoudspeakerFromSpherical (Vector3D<float> sphericalCoordinates, int channel, bool isImaginary, float gain)
{
    return ConfigurationHelper::createElement (sphericalCoordinates.y, sphericalCoordinates.z, sphericalCoordinates.x, channel, isImaginary, gain);
}

Vector3D<float> AllRADecoderAudioProcessor::cartesianToSpherical(Vector3D<float> cartvect)
{
    const float r = cartvect.length();
    return Vector3D<float>(
                           r, // radius
                           radiansToDegrees(atan2(cartvect.y, cartvect.x)), // azimuth
                           radiansToDegrees(atan2(cartvect.z, sqrt(cartvect.x * cartvect.x + cartvect.y * cartvect.y))) // elevation
                           );
}



Vector3D<float> AllRADecoderAudioProcessor::sphericalToCartesian(Vector3D<float> sphervect)
{
    return Vector3D<float>(
                           sphervect.x * cos(degreesToRadians(sphervect.z)) * cos(degreesToRadians(sphervect.y)),
                           sphervect.x * cos(degreesToRadians(sphervect.z)) * sin(degreesToRadians(sphervect.y)),
                           sphervect.x * sin(degreesToRadians(sphervect.z))
                           );
}

Vector3D<float> AllRADecoderAudioProcessor::sphericalInRadiansToCartesian(Vector3D<float> sphervect)
{
    return Vector3D<float>(
                           sphervect.x * cos(sphervect.z) * cos(sphervect.y),
                           sphervect.x * cos(sphervect.z) * sin(sphervect.y),
                           sphervect.x * sin(sphervect.z)
                           );
}

void AllRADecoderAudioProcessor::playNoiseBurst (const int channel)
{
    noiseBurst.setChannel(channel);
}

void AllRADecoderAudioProcessor::playAmbisonicNoiseBurst (const float azimuthInDegrees, const float elevationInDegrees)
{
    auto dec = decoder.getCurrentDecoder();
    if (dec != nullptr)
    {
        ambisonicNoiseBurst.setOrder (decoder.getCurrentDecoder()->getOrder());
        ambisonicNoiseBurst.setNormalization (*useSN3D >= 0.5f);
        ambisonicNoiseBurst.play (azimuthInDegrees, elevationInDegrees);
    }
}

void AllRADecoderAudioProcessor::addImaginaryLoudspeakerBelow()
{
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromCartesian(Vector3D<float>(0.0f, 0.0f, -1.0f), highestChannelNumber + 1, true, 0.0f), &undoManager);
}

void AllRADecoderAudioProcessor::addRandomPoint()
{
    undoManager.beginNewTransaction();
    loudspeakers.appendChild(createLoudspeakerFromSpherical(Vector3D<float> (1.0f, (rand() * 360.0f) / RAND_MAX, (rand() * 180.0f) / RAND_MAX - 90.0f), highestChannelNumber + 1), &undoManager);
}

void AllRADecoderAudioProcessor::convertLoudspeakersToArray()
{
    imaginaryFlags.clear();
    highestChannelNumber = 0;
    int i = 0;
    int imaginaryCount = 0;
    for (ValueTree::Iterator it = loudspeakers.begin() ; it != loudspeakers.end(); ++it)
    {
        const bool isImaginary = (*it).getProperty("Imaginary");
        Vector3D<float> spherical;
        spherical.x = isImaginary ? (float) (*it).getProperty("Radius") : 1.0f;
        spherical.y = (*it).getProperty("Azimuth");
        spherical.z = (*it).getProperty("Elevation");

        Vector3D<float> cart = sphericalToCartesian(spherical);

        R3 newPoint {cart.x, cart.y, cart.z};
        newPoint.lspNum = i;

        if (isImaginary) {
            imaginaryFlags.setBit(i);
            ++imaginaryCount;
            newPoint.realLspNum = -1;
        }
        else
            newPoint.realLspNum = i - imaginaryCount;

        newPoint.azimuth = (*it).getProperty("Azimuth");
        newPoint.elevation = (*it).getProperty("Elevation");
        newPoint.radius = (*it).getProperty("Radius");

        newPoint.isImaginary = isImaginary;
        newPoint.gain = (*it).getProperty("Gain");
        newPoint.channel = (*it).getProperty("Channel");

        if (newPoint.channel > highestChannelNumber)
            highestChannelNumber = newPoint.channel;

        DBG(newPoint.lspNum << " \t " << newPoint.realLspNum << " \t " << newPoint.gain << " \t " << newPoint.x << " \t " << newPoint.y << " \t " << newPoint.z);
        points.push_back(newPoint);
        ++i;
    }
}

void AllRADecoderAudioProcessor::prepareLayout()
{
    isLayoutReady = false;
    Result res = checkLayout();
    if (res.failed())
    {
        DBG(res.getErrorMessage());
        MailBox::Message newMessage;
        newMessage.messageColour = Colours::red;
        newMessage.headline = "Improper layout";
        newMessage.text = res.getErrorMessage();
        messageToEditor = newMessage;
        updateMessage = true;
    }
    else
    {
        DBG("Layout is ready for creating a decoder!");
        MailBox::Message newMessage;
        newMessage.messageColour = Colours::cornflowerblue;
        newMessage.headline = "Suitable layout";
        newMessage.text = "The layout is ready to calculate a decoder.";
        messageToEditor = newMessage;
        updateMessage = true;

        isLayoutReady = true;
    }
}

Result AllRADecoderAudioProcessor::checkLayout()
{
    points.clear();
    triangles.clear();
    normals.clear();

    // Check if loudspeakers are stored properly
    Result res = verifyLoudspeakers();
    if (res.failed())
    {
        updateLoudspeakerVisualization = true;
        return Result::fail(res.getErrorMessage());
    }

    convertLoudspeakersToArray();

    // Check for duplicates
    std::vector<int> outx;
    std::vector<R3> noDuplicates;
    const int nDuplicates = de_duplicateR3(points, outx, noDuplicates);

    if (nDuplicates > 0)
    {
        updateLoudspeakerVisualization = true;
        return Result::fail("ERROR 1: There are duplicate loudspeakers.");
    }

    const int nLsps = loudspeakers.getNumChildren();
    if (nLsps < 4)
    {
        updateLoudspeakerVisualization = true;
        return Result::fail("ERROR 2: There are less than 4 loudspeakers! Add some more!");
    }

    // calculate convex hull
    const int result = NewtonApple_hull_3D(points, triangles);
    if (result != 1)
    {
        return Result::fail("ERROR: An error occurred! The layout might be broken somehow.");
    }

    // normalise normal vectors
    for (int i = 0; i < triangles.size(); ++i)
    {
        const Tri tri = triangles[i];
        normals.push_back(Vector3D<float>(tri.er, tri.ec, tri.ez).normalised());
    }

    updateLoudspeakerVisualization = true;

    Array<int> usedIndices;
    // calculate centroid
    Vector3D<float> centroid {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < nLsps; ++i)
    {
        R3 lsp = points[i];
        centroid += Vector3D<float>(lsp.x, lsp.y, lsp.z);
        usedIndices.add(i);
    }
    centroid /= nLsps;

    DBG("centroid: x: " << centroid.x << " y: " << centroid.y << " z: " << centroid.z);
    for (int i = 0; i < triangles.size(); ++i)
    {
        Tri tri = triangles[i];
        Vector3D<float> a {points[tri.a].x, points[tri.a].y, points[tri.a].z};

        usedIndices.removeFirstMatchingValue(tri.a);
        usedIndices.removeFirstMatchingValue(tri.b);
        usedIndices.removeFirstMatchingValue(tri.c);

        const float dist = normals[i] * (a - centroid);
        if (dist < 0.001f) // too flat!
        {
            return Result::fail("ERROR 3: Convex hull is too flat. Check coordinates and/or try adding imaginary loudspeakers.");
        }

        if (normals[i] * a < 0.001f) // origin is not within hull
        {
            return Result::fail("ERROR 4: Point of origin is not within the convex hull. Try adding imaginary loudspeakers.");
        }

        const int numberOfImaginaryLspsInTriangle = (int) imaginaryFlags[points[tri.a].lspNum] + (int) imaginaryFlags[points[tri.b].lspNum] + (int) imaginaryFlags[points[tri.c].lspNum];
        if (numberOfImaginaryLspsInTriangle > 1)
        {
            return Result::fail("ERROR 5: There is a triangle with more than one imaginary loudspeaker.");
        }
    }

    if (usedIndices.size() > 0)
        return Result::fail("ERROR 6: There is at least one loudspeaker which is not part of the convex hull.");


    if (imaginaryFlags.countNumberOfSetBits() == nLsps)
        return Result::fail("ERROR 7: There are only imaginary loudspeakers.");

    Array<int> routing;
    for (int i = 0; i < nLsps; ++i)
    {
        if (! points[i].isImaginary)
        {
            const int channel = points[i].channel;
            if (channel < 1 || channel > 64)
                return Result::fail("ERROR 8: A channel number is smaller than 1 or greater than 64.");

            if (routing.contains(channel))
                return Result::fail("ERROR 9: Channel number duplicates: a channel number may occur only once.");
            else
                routing.add(channel);
        }
    }

    return Result::ok();
}


Result AllRADecoderAudioProcessor::calculateDecoder()
{
    if (! isLayoutReady)
        return Result::fail("Layout not ready!");

    const int N = roundToInt(*decoderOrder) + 1;
    const auto ambisonicWeights = ReferenceCountedDecoder::Weights (roundToInt (*weights));
    const int nCoeffs = square(N+1);
    const int nLsps = (int) points.size();
    const int nRealLsps = nLsps - imaginaryFlags.countNumberOfSetBits();
    DBG("Number of loudspeakers: " << nLsps << ". Number of real loudspeakers: " << nRealLsps);
    Matrix<float> decoderMatrix(nRealLsps, nCoeffs);

    Array<Matrix<float>> inverseArray;
    for (int t = 0; t < triangles.size(); ++t) //iterate over each triangle
    {
        Tri tri = triangles[t];
        Array<int> triangleIndices (tri.a, tri.b, tri.c);
        Matrix<float> L (3, 3);
        for (int i = 0; i < 3; ++i)
        {
            L(0, i) = points[triangleIndices[i]].x;
            L(1, i) = points[triangleIndices[i]].y;
            L(2, i) = points[triangleIndices[i]].z;
            if (points[triangleIndices[i]].isImaginary)
            {
                const float factor = 1.0f / sqrt(square(points[triangleIndices[i]].x) + square(points[triangleIndices[i]].y) + square(points[triangleIndices[i]].z));
                L(0, i) *= factor;
                L(1, i) *= factor;
                L(2, i) *= factor;
            }
        }

        inverseArray.add(getInverse(L));
    }


    std::vector<float> sh;
    sh.resize(nCoeffs);

    for (int i = 0; i < 5200; ++i) //iterate over each tDesign point
    {
        const Matrix<float> source (3, 1, tDesign5200[i]);
        SHEval(N, source(0,0), source(1,0), source(2,0), &sh[0], false);

        const Matrix<float> gains (3, 1);

        int t = 0;
        for (; t < triangles.size(); ++t) //iterate over each triangle
        {
            Tri tri = triangles[t];
            Array<int> triangleIndices (tri.a, tri.b, tri.c);
            Array<bool> imagFlags (points[tri.a].isImaginary, points[tri.b].isImaginary, points[tri.c].isImaginary);
            Matrix<float> gains (3, 1);
            gains = inverseArray.getUnchecked(t) * source;

            if (gains(0,0) >= -FLT_EPSILON && gains(1,0) >= -FLT_EPSILON && gains(2,0) >= -FLT_EPSILON)
            {
                // we found the corresponding triangle!
                const float foo = 1.0f / sqrt(square(gains(0,0)) + square(gains(1,0)) + square(gains(2,0)));
                gains = gains * foo;

                if (imagFlags.contains(true))
                {
                    const int imagGainIdx = imagFlags.indexOf(true); // which of the three corresponds to the imaginary loudspeaker
                    const int imaginaryLspIdx = triangleIndices[imagGainIdx];

                    Array<int> realGainIndex (0, 1, 2);
                    realGainIndex.remove(imagGainIdx);

                    Array<int> connectedLsps;
                    connectedLsps.add(imaginaryLspIdx);
                    for (int k = 0; k < triangles.size(); ++k) //iterate over each triangle
                    {
                        Tri probe = triangles[k];
                        Array<int> probeTriangleIndices (probe.a, probe.b, probe.c);
                        if (probeTriangleIndices.contains(imaginaryLspIdx))
                        {  // found searched imaginaryLspIdx in that triangle
                            for (int j = 0; j < 3; ++j)
                            {
                                if (! connectedLsps.contains(probeTriangleIndices[j]))
                                    connectedLsps.add(probeTriangleIndices[j]);
                            }
                        }
                    }

                    connectedLsps.remove(0); // remove imaginary loudspeaker again
                    Array<float> gainVector;
                    gainVector.resize(connectedLsps.size());

                    const float kappa = getKappa(gains(imagGainIdx, 0), gains(realGainIndex[0], 0), gains(realGainIndex[1], 0), connectedLsps.size());

                    gainVector.fill(gains(imagGainIdx, 0) * (points[imaginaryLspIdx].gain) * kappa);

                    for (int j = 0; j < 2; ++j)
                    {
                        const int idx = connectedLsps.indexOf(triangleIndices[realGainIndex[j]]);
                        gainVector.set(idx, gainVector[idx] + gains(realGainIndex[j], 0));
                    }

                    for (int n = 0; n < connectedLsps.size(); ++n)
                        FloatVectorOperations::addWithMultiply(&decoderMatrix(points[connectedLsps[n]].realLspNum, 0), &sh[0], gainVector[n], nCoeffs);

                }
                else
                {
                    FloatVectorOperations::addWithMultiply(&decoderMatrix(points[tri.a].realLspNum, 0), &sh[0], gains(0, 0), nCoeffs);
                    FloatVectorOperations::addWithMultiply(&decoderMatrix(points[tri.b].realLspNum, 0), &sh[0], gains(1, 0), nCoeffs);
                    FloatVectorOperations::addWithMultiply(&decoderMatrix(points[tri.c].realLspNum, 0), &sh[0], gains(2, 0), nCoeffs);
                }
                break;
            }
        }
        jassert(t < triangles.size());
    }

    // calculate max lsp gain
    float maxGain = 0.0f;
    for (int i = 0; i < nLsps; ++i)
    {
        const R3 point = points[i];
        if (! point.isImaginary)
        {
            SHEval(N, point.x, point.y, point.z, &sh[0]); // encoding at loudspeaker position
            float sumOfSquares = 0.0f;
            for (int m = 0; m < nRealLsps; ++m)
            {
                float sum = 0.0f;
                for (int n = 0; n < nCoeffs; ++n)
                    sum += sh[n] * decoderMatrix(m, n);
                sumOfSquares += square(sum);
            }
            sumOfSquares = sqrt(sumOfSquares);
            if (sumOfSquares > maxGain)
                maxGain = sumOfSquares;
        }
    }

    decoderMatrix = decoderMatrix * (1.0f / maxGain);

    // calculate energy and rE vector
    Array<Vector3D<float>> realLspsCoordinates;
    realLspsCoordinates.resize(nRealLsps);
    for (int i = 0; i < nLsps; ++i)
    {
        if (! points[i].isImaginary)
            realLspsCoordinates.set(points[i].realLspNum, Vector3D<float>(points[i].x, points[i].y, points[i].z).normalised()); // zero count
    }

    const int w = energyDistribution.getWidth();
    const float wHalf = w / 2;
    const int h = energyDistribution.getHeight();
    const float hHalf = h / 2;
    float minLvl = 0.0f;
    float maxLvl = 0.0f;
    float sumLvl = 0.0f;
    auto levelValues = Matrix<float> (w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            Vector3D<float> spher (1.0f, 0.0f, 0.0f);
            HammerAitov::XYToSpherical((x - wHalf) / wHalf, (hHalf - y) / hHalf, spher.y, spher.z);
            Vector3D<float> cart = sphericalInRadiansToCartesian(spher);
            SHEval(N, cart.x, cart.y, cart.z, &sh[0]); // encoding a source

            if (ambisonicWeights == ReferenceCountedDecoder::Weights::maxrE)
                multiplyMaxRE (N, sh.data());
            else if (ambisonicWeights == ReferenceCountedDecoder::Weights::inPhase)
                multiplyInPhase (N, sh.data());


            Vector3D<float> rE (0.0f, 0.0f, 0.0f);
            float sumOfSquares = 0.0f;
            for (int m = 0; m < nRealLsps; ++m)
            {
                float sum = 0.0f;
                for (int n = 0; n < nCoeffs; ++n)
                    sum += (sh[n] * decoderMatrix(m, n));
                const float sumSquared = square(sum);
                rE += realLspsCoordinates[m] * sumSquared;
                sumOfSquares += sumSquared;
            }

            const float lvl = 0.5f * Decibels::gainToDecibels (sumOfSquares);
            levelValues(x, y) = lvl;
            sumLvl += lvl;

            if (lvl > maxLvl)
                maxLvl = lvl;
            if (lvl < minLvl)
                minLvl = lvl;

            rE /= sumOfSquares + FLT_EPSILON;
            const float width = 2.0f * acos(jmin(1.0f, rE.length()));
            const float reMap = jlimit (0.0f, 1.0f, width / MathConstants<float>::pi);

            const Colour rEPixelColour = Colours::limegreen.withMultipliedAlpha(reMap);
            rEVector.setPixelAt (x, y, rEPixelColour);
        }

    const float meanLvl = sumLvl / (w * h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            constexpr float plusMinusRange = 1.5f;
            const float map = (jlimit (-plusMinusRange, plusMinusRange, levelValues(x, y) - meanLvl) + plusMinusRange) / (2 * plusMinusRange);

            const Colour pixelColour = Colours::red.withMultipliedAlpha (map);
            energyDistribution.setPixelAt (x, y, pixelColour);
        }

    DBG("min: " << minLvl << " max: " << maxLvl);

    updateLoudspeakerVisualization = true;


    ReferenceCountedDecoder::Ptr newDecoder = new ReferenceCountedDecoder("Decoder", "A " + getOrderString(N) + " order Ambisonics decoder using the AllRAD approach.", (int) decoderMatrix.getSize()[0], (int) decoderMatrix.getSize()[1]);
    newDecoder->getMatrix() = decoderMatrix;
    ReferenceCountedDecoder::Settings newSettings;
    newSettings.expectedNormalization = ReferenceCountedDecoder::Normalization::n3d;
    newSettings.weights = ambisonicWeights;
    newSettings.weightsAlreadyApplied = false;

    newDecoder->setSettings(newSettings);

    Array<int>& routing = newDecoder->getRoutingArrayReference();
    routing.resize(nRealLsps);
    for (int i = 0; i < nLsps; ++i)
    {
        if (! points[i].isImaginary)
            routing.set(points[i].realLspNum, points[i].channel - 1); // zero count
    }

    decoder.setDecoder(newDecoder);
    decoderConfig = newDecoder;

    updateChannelCount = true;

    DBG("finished");

    MailBox::Message newMessage;
    newMessage.messageColour = Colours::green;
    newMessage.headline = "Decoder created";
    newMessage.text = "The decoder was calculated successfully.";
    messageToEditor = newMessage;
    updateMessage = true;


    return Result::ok();
}

float AllRADecoderAudioProcessor::getKappa(float gIm, float gRe1, float gRe2, int N)
{
    const float p = gIm * (gRe1 + gRe2) / (N * square(gIm));
    const float q = (square(gRe1) + square(gRe2) - 1.0f) / (N * square(gIm));
    return - p + sqrt(jmax(square(p) - q, 0.0f));
}

Matrix<float> AllRADecoderAudioProcessor::getInverse(Matrix<float> A)
{
    const float det = A (0, 0) * (A (1, 1) * A (2, 2) - A (1, 2) * A (2, 1))
    + A (0, 1) * (A (1, 2) * A (2, 0) - A (1, 0) * A (2, 2))
    + A (0, 2) * (A (1, 0) * A (2, 1) - A (1, 1) * A (2, 0));

    const float factor = 1.0 / det;

    Matrix<float> inverse(3, 3);

    inverse(0, 0) = (A (1, 1) * A (2, 2) - A (1, 2) * A (2, 1)) * factor;
    inverse(0, 1) = (-A (0, 1) * A (2, 2) + A (0, 2) * A (2, 1)) * factor;
    inverse(0, 2) = (A (0, 1) * A (1, 2) - A (0, 2) * A (1, 1)) * factor;

    inverse(1, 0) = (-A (1, 0) * A (2, 2) + A (1, 2) * A (2, 0)) * factor;
    inverse(1, 1) = (A (0, 0) * A (2, 2) - A (0, 2) * A (2, 0)) * factor;
    inverse(1, 2) = (-A (0, 0) * A (1, 2) + A (0, 2) * A (1, 0)) * factor;

    inverse(2, 0) = ( A (1, 0) * A (2, 1) - A (1, 1) * A (2, 0)) * factor;
    inverse(2, 1) = (-A (0, 0) * A (2, 1) + A (0, 1) * A (2, 0)) * factor;
    inverse(2, 2) = ( A (0, 0) * A (1, 1) - A (0, 1) * A (1, 0)) * factor;


    return inverse;
}

void AllRADecoderAudioProcessor::saveConfigurationToFile (File destination)
{
    if (*exportDecoder < 0.5f && *exportLayout < 0.5f)
    {
        DBG("nothing to export");
        MailBox::Message newMessage;
        newMessage.messageColour = Colours::red;
        newMessage.headline = "Nothing to export.";
        newMessage.text = "Please select at least one of the export options.";
        messageToEditor = newMessage;
        updateMessage = true;
        return;
    }

    DynamicObject* jsonObj = new DynamicObject();
    jsonObj->setProperty("Name", var("All-Round Ambisonic decoder (AllRAD) and loudspeaker layout"));
    char versionString[10];
    strcpy(versionString, "v");
    strcat(versionString, JucePlugin_VersionString);
    jsonObj->setProperty("Description", var("This configuration file was created with the IEM AllRADecoder " + String(versionString) + " plug-in. " + Time::getCurrentTime().toString(true, true)));

    if (*exportDecoder >= 0.5f)
    {
        if (decoderConfig != nullptr)
            jsonObj->setProperty ("Decoder", ConfigurationHelper::convertDecoderToVar (decoderConfig));
        else
        {
            DBG("No decoder available");
            MailBox::Message newMessage;
            newMessage.messageColour = Colours::red;
            newMessage.headline = "No decoder available for export.";
            newMessage.text = "Please calculate a decoder first.";
            messageToEditor = newMessage;
            updateMessage = true;
            return;
        }

    }
    if (*exportLayout >= 0.5f)
        jsonObj->setProperty ("LoudspeakerLayout", ConfigurationHelper::convertLoudspeakersToVar (loudspeakers, "A loudspeaker layout"));

    Result result = ConfigurationHelper::writeConfigurationToFile (destination, var (jsonObj));
    if (result.wasOk())
    {
        DBG("Configuration successfully written to file.");
        MailBox::Message newMessage;
        newMessage.messageColour = Colours::green;
        newMessage.headline = "Configuration exported successfully";
        newMessage.text = "The decoder was successfully written to " + destination.getFileName() + ".";
        messageToEditor = newMessage;
        updateMessage = true;
    }
    else
        DBG("Could not write configuration file.");
}

void AllRADecoderAudioProcessor::setLastDir(File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue("presetFolder", v);
}

void AllRADecoderAudioProcessor::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    DBG("valueTreePropertyChanged");
    prepareLayout();
    updateTable = true;
}

void AllRADecoderAudioProcessor::valueTreeChildAdded (ValueTree &parentTree, ValueTree &childWhichHasBeenAdded)
{
    DBG("valueTreeChildAdded");
    prepareLayout();
    updateTable = true;
}

void AllRADecoderAudioProcessor::valueTreeChildRemoved (ValueTree &parentTree, ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
{
    DBG("valueTreeChildRemoved");
    prepareLayout();
    updateTable = true;
}

void AllRADecoderAudioProcessor::valueTreeChildOrderChanged (ValueTree &parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex)
{
    DBG("valueTreeChildOrderChanged");
    prepareLayout();
}

void AllRADecoderAudioProcessor::valueTreeParentChanged (ValueTree &treeWhoseParentHasChanged)
{
    DBG("valueTreeParentChanged");
}

void AllRADecoderAudioProcessor::loadConfiguration (const File& configFile)
{
    undoManager.beginNewTransaction();
    loudspeakers.removeAllChildren(&undoManager);

    Result result = ConfigurationHelper::parseFileForLoudspeakerLayout (configFile, loudspeakers, &undoManager);
    if (!result.wasOk())
    {
        DBG("Configuration could not be loaded.");
        MailBox::Message newMessage;
        newMessage.messageColour = Colours::red;
        newMessage.headline = "Error loading configuration";
        newMessage.text = result.getErrorMessage() ;
        messageToEditor = newMessage;
        updateMessage = true;
    }
}

void AllRADecoderAudioProcessor::rotate (const float degreesAddedToAzimuth)
{
    loudspeakers.removeListener (this);
    undoManager.beginNewTransaction();

    bool amountIsPositive = degreesAddedToAzimuth > 0;
    const int nLsps = loudspeakers.getNumChildren();
    for (int i = 0; i < nLsps; ++i)
    {
        auto lsp = loudspeakers.getChild (i);
        float val = lsp.getProperty ("Azimuth");
        val += degreesAddedToAzimuth;
        if (amountIsPositive && val > 360.0f)
            val -= 360.0f;
        else if (! amountIsPositive && val < -360.0f)
            val += 360.0f;
        lsp.setProperty ("Azimuth", val, &undoManager);
    }
    loudspeakers.addListener (this);
    prepareLayout();
    updateTable = true;
}


//==============================================================================
const bool AllRADecoderAudioProcessor::interceptOSCMessage (OSCMessage &message)
{
    if (message.getAddressPattern().toString().equalsIgnoreCase ("/" + String (JucePlugin_Name) + "/decoderOrder") && message.size() >= 1)
    {
        if (message[0].isInt32())
        {
            auto value = message[0].getInt32() - 1;
            message.clear();
            message.addInt32 (value);
        }
        else if (message[0].isFloat32())
        {
            auto value = message[0].getFloat32() - 1;
            message.clear();
            message.addFloat32 (value);
        }
    }

    return false;
}

const bool AllRADecoderAudioProcessor::processNotYetConsumedOSCMessage (const OSCMessage &message)
{
    const String prefix ("/" + String (JucePlugin_Name));
    if (message.getAddressPattern().toString().startsWith (prefix))
    {
        OSCMessage msg (message);
        msg.setAddressPattern (message.getAddressPattern().toString().substring (String (JucePlugin_Name).length() + 1));

        if (msg.getAddressPattern().toString().equalsIgnoreCase ("/loadFile") && msg.size() >= 1)
        {
            if (msg[0].isString())
            {
                File fileToLoad (msg[0].getString());
                loadConfiguration (fileToLoad);
                return true;
            }
        }
        else if (msg.getAddressPattern().toString().equalsIgnoreCase ("/calculate") ||
                 msg.getAddressPattern().toString().equalsIgnoreCase ("/calculateDecoder"))
        {
            calculateDecoder();
            return true;
        }
        else if (msg.getAddressPattern().toString().equalsIgnoreCase ("/export") && msg.size() >= 1)
        {
            if (msg[0].isString())
            {
                File file (msg[0].getString());
                saveConfigurationToFile (file);
                return true;
            }
        }
        else if (msg.getAddressPattern().toString().equalsIgnoreCase ("/playNoise") && msg.size() >= 1)
        {
            if (msg[0].isInt32())
            {
                const int channel = msg[0].getInt32();
                if (channel <= 64)
                {
                    playNoiseBurst (channel);
                    return true;
                }
            }
        }
        else if (msg.getAddressPattern().toString().equalsIgnoreCase ("/playEncodedNoise") && msg.size() >= 2)
        {
            float azimuth = 0.0f;
            float elevation = 0.0f;

            if (msg[0].isInt32())
                azimuth = msg[0].getInt32();
            else if (msg[0].isFloat32())
                azimuth = msg[0].getFloat32();
            else
                return false;

            if (msg[1].isInt32())
                elevation = msg[1].getInt32();
            else if (msg[1].isFloat32())
                elevation = msg[1].getFloat32();
            else
                return false;

            playAmbisonicNoiseBurst (azimuth, elevation);
            return true;
        }
    }

    return false;
}


// ==============================================================================
std::vector<std::unique_ptr<RangedAudioParameter>> AllRADecoderAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("inputOrderSetting", "Input Ambisonic Order", "",
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

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("useSN3D", "Input Normalization", "",
                                    NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                    [](float value) {
                                        if (value >= 0.5f) return "SN3D";
                                        else return "N3D";
                                    }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("decoderOrder", "Decoder Order", "",
                                     NormalisableRange<float> (0.0f, 6.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value >= 0.5f && value < 1.5f) return "2nd";
                                         else if (value >= 1.5f && value < 2.5f) return "3rd";
                                         else if (value >= 2.5f && value < 3.5f) return "4th";
                                         else if (value >= 3.5f && value < 4.5f) return "5th";
                                         else if (value >= 4.5f && value < 5.5f) return "6th";
                                         else if (value >= 5.5f ) return "7th";
                                         else return "1st";},
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("exportDecoder", "Export Decoder", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "Yes";
                                         else return "No";
                                     }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("exportLayout", "Export Layout", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "Yes";
                                         else return "No";
                                     }, nullptr));

    params.push_back (std::make_unique<AudioParameterChoice> ("weights", "Ambisonic Weights", weightsStrings, 1));

    return params;
}
