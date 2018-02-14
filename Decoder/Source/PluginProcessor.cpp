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
PluginTemplateAudioProcessor::PluginTemplateAudioProcessor()
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
parameters(*this, nullptr)
{
     
    parameters.createAndAddParameter ("inputOrderSetting", "Ambisonic Order", "",
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
    parameters.createAndAddParameter("useSN3D", "Normalization", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "SN3D";
                                         else return "N3D";
                                     }, nullptr);
    

    
    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("PluginTemplate"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened
    
    
    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue ("inputOrderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");

    
    // add listeners to parameter changes
    parameters.addParameterListener ("inputOrderSetting", this);
    parameters.addParameterListener ("useSN3D", this);

    
    //Tests
//    Vector3D<float> testVec ((float) rand(), (float) rand(), (float) rand());
//    testVec /= (float) rand();
//    Vector3D<float> backAndForth = sphericalToCarthesian(carthesianToSpherical(testVec));
//    if (testVec.x != backAndForth.x || testVec.y != backAndForth.y || testVec.z != backAndForth.z)
//        DBG("Conversion failed!");
    

    var Lsps;
    Lsps.append(newSpeakerVarFromSpherical(Vector3D<float> (50.0f, 0.0f, 0.0f), 1));
    Lsps.append(newSpeakerVarFromSpherical(Vector3D<float> (50.0f, -45.0f, 0.0f), 2));
    Lsps.append(newSpeakerVarFromSpherical(Vector3D<float> (50.0f, 45.0f, 0.0f), 3));
    Lsps.append(newSpeakerVarFromSpherical(Vector3D<float> (50.0f, 45.0f, -90.0f), 5));
    Lsps.append(newSpeakerVarFromSpherical(Vector3D<float> (50.0f, 0.0f, 90.0f), 5));
    Lsps.append(newSpeakerVarFromSpherical(Vector3D<float> (50.0f, 180.0f, 0.0f), 6));
    
    lsps = Lsps;
    
    runTris();
}

PluginTemplateAudioProcessor::~PluginTemplateAudioProcessor()
{
}

//==============================================================================
const String PluginTemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginTemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginTemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginTemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginTemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginTemplateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginTemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginTemplateAudioProcessor::setCurrentProgram (int index)
{
}

const String PluginTemplateAudioProcessor::getProgramName (int index)
{
    return {};
}

void PluginTemplateAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void PluginTemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *inputOrderSetting, 64, true);
    
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    
    
}

void PluginTemplateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginTemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void PluginTemplateAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputOrderSetting, 64, false);
    ScopedNoDenormals noDenormals;
    
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool PluginTemplateAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PluginTemplateAudioProcessor::createEditor()
{
    return new PluginTemplateAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void PluginTemplateAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    ScopedPointer<XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}



void PluginTemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.state = ValueTree::fromXml (*xmlState);
}

//==============================================================================
void PluginTemplateAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG("Parameter with ID " << parameterID << " has changed. New value: " << newValue);
    
    if (parameterID == "inputChannelsSetting" || parameterID == "outputOrderSetting" )
        userChangedIOSettings = true;
}

void PluginTemplateAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginTemplateAudioProcessor();
}

Result PluginTemplateAudioProcessor::verifyLoudspeakerVar(var& loudspeakerVar)
{
    if (! loudspeakerVar.isArray())
        return Result::fail("There is no loudspeaker array!");
    
    const Array<var>* lspArray = loudspeakerVar.getArray();
    const int nLsp = lspArray->size();
    
    for (int i = 0; i < nLsp; ++i)
    {
        var lsp = lspArray->getUnchecked(i);
        
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
        }
        
        bool isVirt = false;
        if (lsp.hasProperty("Virtual")) //optional
        {
            const var virt = lsp.getProperty("Virtual", var());
            if (! virt.isBool())
                return Result::fail("Loudspeaker #" + String(i+1) + ": 'Virtual' value is not a bool.");
            isVirt = virt;
        }
        
        if (! isVirt)
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



Result PluginTemplateAudioProcessor::calculateTris()
{
    Result res = verifyLoudspeakerVar(lsps);
    if (res.failed())
        DBG(res.getErrorMessage());
    
    if (! lsps.isArray() || lsps.size() < 3)
        return Result::fail("Not enough loudspeakers available!");
    
    chCoords.clear();
    tris.clear();
    const Array<var>* lspArray = lsps.getArray();
    const int nLsp = lspArray->size();
    
    for (int i = 0; i < nLsp; ++i)
    {
        var tmpLsp = lspArray->getUnchecked(i);
        Vector3D<float> spherical;
        spherical.x = tmpLsp.getProperty("Radius", var(0.0f));
        spherical.y = tmpLsp.getProperty("Azimuth", var(0.0f));
        spherical.z = tmpLsp.getProperty("Elevation", var(0.0f));
        
        Vector3D<float> carth = sphericalToCarthesian(spherical);

        chCoords.push_back(R3(carth.x, carth.y, carth.z));
        DBG(carth.x << " " << carth.y << " " << carth.z);
    }
    
    
    std::vector<int> outx;
    std::vector<R3> noDuplicates;
    const int nx = de_duplicateR3(chCoords, outx, noDuplicates);
    
    if (nx > 0)
        return Result::fail("There was at least one duplicated loudspeaker.");
    
    chCoords = noDuplicates;
    DBG("outx size: " << outx.size());
    const int ts = NewtonApple_hull_3D(noDuplicates, tris);
    return Result::ok();
}

var PluginTemplateAudioProcessor::newSpeakerVarFromCarthesian(Vector3D<float> carthCoordinates, int channel, bool isVirtual, float gain)
{
    Vector3D<float> sphericalCoordinates = carthesianToSpherical(carthCoordinates);
    return newSpeakerVarFromSpherical(sphericalCoordinates, channel, isVirtual, gain);
}

var PluginTemplateAudioProcessor::newSpeakerVarFromSpherical(Vector3D<float> sphericalCoordinates, int channel, bool isVirtual, float gain)
{
    DynamicObject* lspObj = new DynamicObject();
    lspObj->setProperty("Azimuth", sphericalCoordinates.y);
    lspObj->setProperty("Elevation", sphericalCoordinates.z);
    lspObj->setProperty("Radius", sphericalCoordinates.x);
    lspObj->setProperty("Channel", channel);
    lspObj->setProperty("isVirtual", isVirtual);
    lspObj->setProperty("Gain", gain);
    
    var newLsp (lspObj);
    return newLsp;
}

Vector3D<float> PluginTemplateAudioProcessor::carthesianToSpherical(Vector3D<float> carthvect)
{
    const float r = carthvect.length();
    return Vector3D<float>(
                           r, // radius
                           radiansToDegrees(atan2(carthvect.y, carthvect.x)), // azimuth
                           radiansToDegrees(atan2(carthvect.z, sqrt(carthvect.x * carthvect.x + carthvect.y * carthvect.y))) // elevation
                           );
}



Vector3D<float> PluginTemplateAudioProcessor::sphericalToCarthesian(Vector3D<float> sphervect)
{
    return Vector3D<float>(
                           sphervect.x * cos(degreesToRadians(sphervect.z)) * cos(degreesToRadians(sphervect.y)),
                           sphervect.x * cos(degreesToRadians(sphervect.z)) * sin(degreesToRadians(sphervect.y)),
                           sphervect.x * sin(degreesToRadians(sphervect.z))
                           );
}

void PluginTemplateAudioProcessor::addRandomPoint()
{
    lsps.append(newSpeakerVarFromSpherical(Vector3D<float> (50.0f, (rand() * 360.0f) / RAND_MAX, (rand() * 180.0f) / RAND_MAX - 90.0f), -1));
    runTris();
}
void PluginTemplateAudioProcessor::runTris()
{
    Result res = calculateTris();
    if (res.wasOk())
    {
        points.clear();
        triangles.clear();
        normals.clear();
        DBG("num: " << chCoords.size());
        for (int i = 0; i < chCoords.size(); ++i)
        {
            R3 foo = chCoords[i];
            points.push_back(foo.c);
            points.push_back(foo.z);
            points.push_back(foo.r);
            DBG(foo.r << " " << foo.c << " " << foo.z);
        }
        
        for (int i = 0; i < tris.size(); ++i)
        {
            Tri tri = tris[i];
            
            Vector3D<float> a {chCoords[tri.a].r, chCoords[tri.a].c, chCoords[tri.a].z};
            Vector3D<float> b {chCoords[tri.b].r, chCoords[tri.b].c, chCoords[tri.b].z};
            Vector3D<float> c {chCoords[tri.c].r, chCoords[tri.c].c, chCoords[tri.c].z};
            Vector3D<float> n {tri.er, tri.ec, tri.ez};
            a = a.normalised();
            b = b.normalised();
            c = c.normalised();
            n = n.normalised();
            
            Vector3D<float> uN;
            uN = ((b-a)^(c-a)).normalised();
            
            //float dist = n*a;
            float uDist = uN*a;
            
            if (uDist < 0)
            {
                triangles.push_back(tri.a);
                triangles.push_back(tri.c);
                triangles.push_back(tri.b);
                normals.push_back(-uN.x);
                normals.push_back(-uN.y);
                normals.push_back(-uN.z);
            }
            else
            {
                triangles.push_back(tri.a);
                triangles.push_back(tri.b);
                triangles.push_back(tri.c);
                normals.push_back(uN.x);
                normals.push_back(uN.y);
                normals.push_back(uN.z);
            }
            normals.push_back(1.0f);
            
            DBG(tri.a << " " << tri.b << " " << tri.c);
        }
        
        
        DBG(points.size());
        updateLoudspeakerVisualization = true;
    }
}

void PluginTemplateAudioProcessor::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    DBG("valueTreePropertyChanged");
}

void PluginTemplateAudioProcessor::valueTreeChildAdded (ValueTree &parentTree, ValueTree &childWhichHasBeenAdded)
{
    DBG("valueTreeChildAdded");
}

void PluginTemplateAudioProcessor::valueTreeChildRemoved (ValueTree &parentTree, ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
{
    DBG("valueTreeChildRemoved");
}

void PluginTemplateAudioProcessor::valueTreeChildOrderChanged (ValueTree &parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex)
{
    DBG("valueTreeChildOrderChanged");
}

void PluginTemplateAudioProcessor::valueTreeParentChanged (ValueTree &treeWhoseParentHasChanged)
{
    DBG("valueTreeParentChanged");
}
