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


static constexpr int filterTypePresets[] = {1, 1, 1, 1, 1, 1};
static constexpr float filterFrequencyPresets[] = {80.0f, 120.0f, 1600.0f, 2200.0f, 8000.0f, 16000.0f};

//==============================================================================
MultiEQAudioProcessor::MultiEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels (10), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels (64), true)
                     #endif
                       ),
#endif
parameters (*this, nullptr), oscParams (parameters)
{
    oscParams.createAndAddParameter ("inputChannelsSetting", "Number of input channels ", "",
                                     NormalisableRange<float> (0.0f, 64.0f, 1.0f), 0.0f,
                                     [](float value) {return value < 0.5f ? "Auto" : String (value);}, nullptr);


    int i = 0;
    oscParams.createAndAddParameter ("filterEnabled" + String (i), "Filter Enablement " + String (i + 1), "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) { return value < 0.5 ? String ("OFF") : String ("ON");}, nullptr);

    oscParams.createAndAddParameter ("filterType" + String (i), "Filter Type " + String (i + 1), "",
                                     NormalisableRange<float> (0.0f, 3.0f, 1.0f),  filterTypePresets[i],
                                     [](float value) {
                                         if (value < 0.5f) return "HP (6dB/oct)";
                                         else if (value >= 0.5f && value < 1.5f) return "HP (12dB/oct)";
                                         else if (value >= 1.5f && value < 2.5f) return "HP (24dB/oct)";
                                         else return "Low-shelf";},
                                     nullptr);

    oscParams.createAndAddParameter ("filterFrequency" + String (i), "Filter Frequency " + String (i + 1), "Hz",
                                     NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.4f), filterFrequencyPresets[i],
                                     [](float value) { return String(value, 0); }, nullptr);

    oscParams.createAndAddParameter ("filterQ" + String (i), "Filter Q " + String (i+1), "",
                                     NormalisableRange<float> (0.05f, 8.0f, 0.05f), 0.7f,
                                     [](float value) { return String (value, 2); },
                                     nullptr);

    oscParams.createAndAddParameter ("filterGain" + String (i), "Filter Gain " + String (i + 1), "dB",
                                     NormalisableRange<float> (-60.0f, 15.0f, 0.1f), 0.0f,
                                     [](float value) { return String (value, 1); },
                                     nullptr);


    for (int i = 1; i < numFilterBands - 1; ++i)
    {
        oscParams.createAndAddParameter ("filterEnabled" + String (i), "Filter Enablement " + String (i + 1), "",
                                         NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                         [](float value) { return value < 0.5 ? String ("OFF") : String ("ON");}, nullptr);

        oscParams.createAndAddParameter ("filterType" + String (i), "Filter Type " + String (i + 1), "",
                                        NormalisableRange<float> (0.0f, 2.0f, 1.0f),  filterTypePresets[i],
                                        [](float value) {
                                            if (value < 0.5f) return "Low-shelf";
                                            else if (value >= 0.5f && value < 1.5f) return "Peak";
                                            else return "High-shelf";},
                                        nullptr);

        oscParams.createAndAddParameter ("filterFrequency" + String (i), "Filter Frequency " + String (i + 1), "Hz",
                                        NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.4f), filterFrequencyPresets[i],
                                        [](float value) { return String(value, 0); }, nullptr);

        oscParams.createAndAddParameter ("filterQ" + String (i), "Filter Q " + String (i+1), "",
                                        NormalisableRange<float> (0.05f, 8.0f, 0.05f), 0.7f,
                                        [](float value) { return String (value, 2); },
                                        nullptr);

        oscParams.createAndAddParameter ("filterGain" + String (i), "Filter Gain " + String (i + 1), "dB",
                                        NormalisableRange<float> (-60.0f, 15.0f, 0.1f), 0.0f,
                                        [](float value) { return String (value, 1); },
                                        nullptr);
    }

    i = numFilterBands - 1;

    oscParams.createAndAddParameter ("filterEnabled" + String (i), "Filter Enablement " + String (i + 1), "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) { return value < 0.5 ? String ("OFF") : String ("ON");}, nullptr);

    oscParams.createAndAddParameter ("filterType" + String (i), "Filter Type " + String (i + 1), "",
                                     NormalisableRange<float> (0.0f, 3.0f, 1.0f),  filterTypePresets[i],
                                     [](float value) {
                                         if (value < 0.5f) return "LP (6dB/Oct)";
                                         else if (value >= 0.5f && value < 1.5f) return "LP (12dB/oct)";
                                         else if (value >= 1.5f && value < 2.5f) return "LP (24dB/oct)";
                                         else return "High-shelf";},
                                     nullptr);

    oscParams.createAndAddParameter ("filterFrequency" + String (i), "Filter Frequency " + String (i + 1), "Hz",
                                     NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.4f), filterFrequencyPresets[i],
                                     [](float value) { return String(value, 0); }, nullptr);

    oscParams.createAndAddParameter ("filterQ" + String (i), "Filter Q " + String (i+1), "",
                                     NormalisableRange<float> (0.05f, 8.0f, 0.05f), 0.7f,
                                     [](float value) { return String (value, 2); },
                                     nullptr);

    oscParams.createAndAddParameter ("filterGain" + String (i), "Filter Gain " + String (i + 1), "dB",
                                     NormalisableRange<float> (-60.0f, 15.0f, 0.1f), 0.0f,
                                     [](float value) { return String (value, 1); },
                                     nullptr);

    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("MultiEQ"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened


    // get pointers to the parameters
    inputChannelsSetting = parameters.getRawParameterValue ("inputChannelsSetting");

    // add listeners to parameter changes
    parameters.addParameterListener ("inputChannelsSetting", this);

    for (int i = 0; i < numFilterBands; ++i)
    {
        filterEnabled[i] = parameters.getRawParameterValue ("filterEnabled" + String(i));
        filterType[i] = parameters.getRawParameterValue ("filterType" + String(i));
        filterFrequency[i] = parameters.getRawParameterValue ("filterFrequency" + String(i));
        filterQ[i] = parameters.getRawParameterValue ("filterQ" + String(i));
        filterGain[i] = parameters.getRawParameterValue ("filterGain" + String(i));

        parameters.addParameterListener("filterType" + String(i), this);
        parameters.addParameterListener("filterFrequency" + String(i), this);
        parameters.addParameterListener("filterQ" + String(i), this);
        parameters.addParameterListener("filterGain" + String(i), this);
    }

    additionalTempCoefficients[0] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);
    additionalTempCoefficients[1] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);

    for (int i = 0; i < numFilterBands; ++i)
    {
        createFilterCoefficients (i, 48000.0);
    }

    for (int i = 0; i < numFilterBands; ++i)
    {
        processorCoefficients[i] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);
    }

    additionalProcessorCoefficients[0] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);
    additionalProcessorCoefficients[1] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);

    copyFilterCoefficientsToProcessor();

    for (int i = 0; i < numFilterBands; ++i)
    {
        filterArrays[i].clear();
        for (int ch = 0; ch < ceil (64 / IIRfloat_elements()); ++ch)
            filterArrays[i].add (new IIR::Filter<IIRfloat> (processorCoefficients[i]));
    }

    additionalFilterArrays[0].clear();
    for (int ch = 0; ch < ceil (64 / IIRfloat_elements()); ++ch)
        additionalFilterArrays[0].add (new IIR::Filter<IIRfloat> (additionalProcessorCoefficients[0]));

    additionalFilterArrays[1].clear();
    for (int ch = 0; ch < ceil (64 / IIRfloat_elements()); ++ch)
        additionalFilterArrays[1].add (new IIR::Filter<IIRfloat> (additionalProcessorCoefficients[1]));


    oscReceiver.addListener (this);
}


MultiEQAudioProcessor::~MultiEQAudioProcessor()
{
}


void MultiEQAudioProcessor::updateGuiCoefficients()
{
    const double sampleRate = getSampleRate();

    // Low band
    const SpecialFilterType lowType = SpecialFilterType (static_cast<int> (*filterType[0]));

    switch (lowType)
    {
        case SpecialFilterType::LinkwitzRileyHighPass:
        {
            auto coeffs = IIR::Coefficients<double>::makeHighPass (sampleRate, *filterFrequency[0]);
            coeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients
            (coeffs->coefficients, coeffs->coefficients);
            guiCoefficients[0] = coeffs;
            break;
        }
        case SpecialFilterType::FirstOrderHighPass:
            guiCoefficients[0] = IIR::Coefficients<double>::makeFirstOrderHighPass (sampleRate, *filterFrequency[0]);
            break;
        case SpecialFilterType::SecondOrderHighPass:
            guiCoefficients[0] = IIR::Coefficients<double>::makeHighPass (sampleRate, *filterFrequency[0], *filterQ[0]);
            break;
        case SpecialFilterType::LowShelf:
            guiCoefficients[0] = IIR::Coefficients<double>::makeLowShelf (sampleRate, *filterFrequency[0], *filterQ[0], Decibels::decibelsToGain (*filterGain[0]));
            break;
        default:
            break;
    }


    // High band
    const SpecialFilterType highType = SpecialFilterType (4 + static_cast<int> (*filterType[numFilterBands - 1]));

    switch (highType)
    {
        case SpecialFilterType::LinkwitzRileyLowPass:
        {
            auto coeffs = IIR::Coefficients<double>::makeLowPass (sampleRate, *filterFrequency[numFilterBands - 1]);
            coeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients
            (coeffs->coefficients, coeffs->coefficients);
            guiCoefficients[numFilterBands - 1] = coeffs;
            break;
        }
        case SpecialFilterType::FirstOrderLowPass:
            guiCoefficients[numFilterBands - 1] = IIR::Coefficients<double>::makeFirstOrderLowPass (sampleRate, *filterFrequency[numFilterBands - 1]);
            break;
        case SpecialFilterType::SecondOrderLowPass:
            guiCoefficients[numFilterBands - 1] = IIR::Coefficients<double>::makeLowPass (sampleRate, *filterFrequency[numFilterBands - 1], *filterQ[numFilterBands - 1]);
            break;
        case SpecialFilterType::HighShelf:
            guiCoefficients[numFilterBands - 1] = IIR::Coefficients<double>::makeHighShelf (sampleRate, *filterFrequency[numFilterBands - 1], *filterQ[numFilterBands - 1], Decibels::decibelsToGain (*filterGain[numFilterBands - 1]));
            break;
        default:
            break;
    }

    // regular bands

    for (int f = 1; f < numFilterBands - 1; ++f)
    {
        const RegularFilterType type = RegularFilterType (2 + static_cast<int>(*filterType[f]));
        switch (type)
        {
            case RegularFilterType::LowShelf:
                guiCoefficients[f] = IIR::Coefficients<double>::makeLowShelf (sampleRate, *filterFrequency[f], *filterQ[f], Decibels::decibelsToGain (*filterGain[f]));
                break;
            case RegularFilterType::PeakFilter:
                guiCoefficients[f] = IIR::Coefficients<double>::makePeakFilter (sampleRate, *filterFrequency[f], *filterQ[f], Decibels::decibelsToGain (*filterGain[f]));
                break;
            case RegularFilterType::HighShelf:
                guiCoefficients[f] = IIR::Coefficients<double>::makeHighShelf (sampleRate, *filterFrequency[f], *filterQ[f], Decibels::decibelsToGain (*filterGain[f]));
                break;
            default:
                break;
        }

    }
}

inline dsp::IIR::Coefficients<float>::Ptr MultiEQAudioProcessor::createFilterCoefficients (const RegularFilterType type, const double sampleRate, const float frequency, const float Q, const float gain)
{
    switch (type)
    {
        case RegularFilterType::FirstOrderHighPass:
            return IIR::Coefficients<float>::makeFirstOrderHighPass (sampleRate, frequency);
            break;
        case RegularFilterType::SecondOrderHighPass:
            return IIR::Coefficients<float>::makeHighPass (sampleRate, frequency, Q);
            break;
        case RegularFilterType::LowShelf:
            return IIR::Coefficients<float>::makeLowShelf (sampleRate, frequency, Q, gain);
            break;
        case RegularFilterType::PeakFilter:
            return IIR::Coefficients<float>::makePeakFilter (sampleRate, frequency, Q, gain);
            break;
        case RegularFilterType::HighShelf:
            return IIR::Coefficients<float>::makeHighShelf (sampleRate, frequency, Q, gain);
            break;
        case RegularFilterType::FirstOrderLowPass:
            return IIR::Coefficients<float>::makeFirstOrderLowPass (sampleRate, frequency);
            break;
        case RegularFilterType::SecondOrderLowPass:
            return IIR::Coefficients<float>::makeLowPass (sampleRate, frequency, Q);
            break;
        default:
            return IIR::Coefficients<float>::makeAllPass (sampleRate, frequency, Q);
            break;
    }
}

inline dsp::IIR::Coefficients<double>::Ptr MultiEQAudioProcessor::createFilterCoefficientsForGui (const RegularFilterType type, const double sampleRate, const float frequency, const float Q, const float gain)
{
    switch (type)
    {
        case RegularFilterType::FirstOrderHighPass:
            return IIR::Coefficients<double>::makeFirstOrderHighPass (sampleRate, frequency);
            break;
        case RegularFilterType::SecondOrderHighPass:
            return IIR::Coefficients<double>::makeHighPass (sampleRate, frequency, Q);
            break;
        case RegularFilterType::LowShelf:
            return IIR::Coefficients<double>::makeLowShelf (sampleRate, frequency, Q, gain);
            break;
        case RegularFilterType::PeakFilter:
            return IIR::Coefficients<double>::makePeakFilter (sampleRate, frequency, Q, gain);
            break;
        case RegularFilterType::HighShelf:
            return IIR::Coefficients<double>::makeHighShelf (sampleRate, frequency, Q, gain);
            break;
        case RegularFilterType::FirstOrderLowPass:
            return IIR::Coefficients<double>::makeFirstOrderLowPass (sampleRate, frequency);
            break;
        case RegularFilterType::SecondOrderLowPass:
            return IIR::Coefficients<double>::makeLowPass (sampleRate, frequency, Q);
            break;
        default:
            return IIR::Coefficients<double>::makeAllPass (sampleRate, frequency, Q);
            break;
    }
}

void MultiEQAudioProcessor::createLinkwitzRileyFilter (const bool isUpperBand)
{
    if (isUpperBand)
    {
        tempCoefficients[numFilterBands - 1] = IIR::Coefficients<float>::makeLowPass (getSampleRate(), *filterFrequency[numFilterBands - 1], *filterQ[numFilterBands - 1]);
        additionalTempCoefficients[1] = processorCoefficients[numFilterBands - 1];
    }
    else
    {
        tempCoefficients[0] = IIR::Coefficients<float>::makeHighPass (getSampleRate(), *filterFrequency[0], *filterQ[0]);
        additionalTempCoefficients[0] = processorCoefficients[0];
    }
}

void MultiEQAudioProcessor::createFilterCoefficients (const int filterIndex, const double sampleRate)
{
    const int type = roundToInt (*filterType[filterIndex]);
    if (filterIndex == 0 && type == 2)
    {
        createLinkwitzRileyFilter (false);
    }
    else if (filterIndex == numFilterBands - 1 && type == 2)
    {
        createLinkwitzRileyFilter (true);
    }
    else
    {
        RegularFilterType filterType = RegularFilterType::NothingToDo;
        switch (filterIndex)
        {
            case 0:
                jassert (type != 2);
                if (type == 0)
                    filterType = RegularFilterType::FirstOrderHighPass;
                else if (type == 3)
                    filterType = RegularFilterType::LowShelf;
                else if (type == 1)
                    filterType = RegularFilterType::SecondOrderHighPass;
                break;

            case numFilterBands - 1:
                jassert (type != 2);
                if (type == 0)
                    filterType = RegularFilterType::FirstOrderLowPass;
                else if (type == 3)
                    filterType = RegularFilterType::HighShelf;
                else if (type == 1)
                    filterType = RegularFilterType::SecondOrderLowPass;
                break;

            default:
                jassert (type < 2);
                switch (type)
                {
                    case 0:
                        filterType = RegularFilterType::LowShelf;
                        break;
                    case 1:
                        filterType = RegularFilterType::PeakFilter;
                        break;
                    case 2:
                        filterType = RegularFilterType::HighShelf;
                        break;
                    default:
                        break;
                }
                break;
        }
        tempCoefficients[filterIndex] = createFilterCoefficients (filterType, sampleRate, *filterFrequency[filterIndex], *filterQ[filterIndex], Decibels::decibelsToGain (*filterGain[filterIndex]));
    }

}

void MultiEQAudioProcessor::copyFilterCoefficientsToProcessor()
{
    for (int b = 0; b < numFilterBands; ++b)
        *processorCoefficients[b] = *tempCoefficients[b];

    *additionalProcessorCoefficients[0] = *additionalTempCoefficients[0];
    *additionalProcessorCoefficients[1] = *additionalTempCoefficients[1];
    
    userHasChangedFilterSettings = false;
}


//==============================================================================
const String MultiEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultiEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultiEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultiEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MultiEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultiEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiEQAudioProcessor::setCurrentProgram (int index)
{
}

const String MultiEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultiEQAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MultiEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *inputChannelsSetting, *inputChannelsSetting, true);

    for (int f = 0; f < numFilterBands; ++f)
    {
        createFilterCoefficients (f, sampleRate);
    }
    copyFilterCoefficientsToProcessor();


    interleavedData.clear();

    for (int i = 0; i < ceil (64 / IIRfloat_elements()); ++i)
    {
        // reset filters
        for (int f = 0; f < numFilterBands; ++f)
        {
            filterArrays[f][i]->reset (IIRfloat (0.0f));
        }

        interleavedData.add (new AudioBlock<IIRfloat> (interleavedBlockData[i], 1, samplesPerBlock));
        interleavedData.getLast()->clear();
    }

    zero = AudioBlock<float> (zeroData, IIRfloat_elements(), samplesPerBlock);
    zero.clear();
}

void MultiEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void MultiEQAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *inputChannelsSetting, *inputChannelsSetting, false);
    ScopedNoDenormals noDenormals;

    const int L = buffer.getNumSamples();

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    const int maxNChIn = jmin (buffer.getNumChannels(), input.getSize());
    const int maxNChOut = jmin (buffer.getNumChannels(), output.getSize());

    const int nSIMDFilters = 1 + (maxNChIn - 1) / IIRfloat_elements();


    // update iir filter coefficients
    if (userHasChangedFilterSettings.get()) copyFilterCoefficientsToProcessor();


    //interleave input data
    int partial = maxNChIn % IIRfloat_elements();
    if (partial == 0)
    {
        for (int i = 0; i<nSIMDFilters; ++i)
        {
            AudioDataConverters::interleaveSamples (buffer.getArrayOfReadPointers() + i* IIRfloat_elements(),
                                                   reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)), L,
                                                   static_cast<int> (IIRfloat_elements()));
        }
    }
    else
    {
        int i;
        for (i = 0; i<nSIMDFilters-1; ++i)
        {
            AudioDataConverters::interleaveSamples (buffer.getArrayOfReadPointers() + i* IIRfloat_elements(),
                                                   reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)), L,
                                                   static_cast<int> (IIRfloat_elements()));
        }

        const float* addr[IIRfloat_elements()];
        int ch;
        for (ch = 0; ch < partial; ++ch)
        {
            addr[ch] = buffer.getReadPointer (i * IIRfloat_elements() + ch);
        }
        for (; ch < IIRfloat_elements(); ++ch)
        {
            addr[ch] = zero.getChannelPointer(ch);
        }
        AudioDataConverters::interleaveSamples (addr,
                                               reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)), L,
                                               static_cast<int> (IIRfloat_elements()));
    }



    // apply filters
    for (int f = 0; f <numFilterBands; ++f)
    {
        if (*filterEnabled[f] > 0.5f)
        {
            for (int i = 0; i < nSIMDFilters; ++i)
            {
                ProcessContextReplacing<IIRfloat> context (*interleavedData[i]);
                filterArrays[f][i]->process (context);
            }
        }
    }

    // check and apply additional filters (Linkwitz Riley -> two BiQuads)
    if (static_cast<int> (*filterType[0]) == 2 && *filterEnabled[0] > 0.5f)
    {
        for (int i = 0; i < nSIMDFilters; ++i)
        {
            ProcessContextReplacing<IIRfloat> context (*interleavedData[i]);
            additionalFilterArrays[0][i]->process (context);
        }
    }
    if (static_cast<int> (*filterType[numFilterBands - 1]) == 2 && *filterEnabled[numFilterBands - 1] > 0.5f)
    {
        for (int i = 0; i < nSIMDFilters; ++i)
        {
            ProcessContextReplacing<IIRfloat> context (*interleavedData[i]);
            additionalFilterArrays[1][i]->process (context);
        }
    }


    // deinterleave
    if (partial == 0)
    {
        for (int i = 0; i<nSIMDFilters; ++i)
        {
            AudioDataConverters::deinterleaveSamples (reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)),
                                                      buffer.getArrayOfWritePointers() + i * IIRfloat_elements(),
                                                      L,
                                                      static_cast<int> (IIRfloat_elements()));
        }
    }
    else
    {
        int i;
        for (i = 0; i<nSIMDFilters-1; ++i)
        {
            AudioDataConverters::deinterleaveSamples (reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)),
                                                      buffer.getArrayOfWritePointers() + i * IIRfloat_elements(),
                                                      L,
                                                      static_cast<int> (IIRfloat_elements()));
        }

        float* addr[IIRfloat_elements()];
        int ch;
        for (ch = 0; ch < partial; ++ch)
        {
            addr[ch] = buffer.getWritePointer (i * IIRfloat_elements() + ch);
        }
        for (; ch < IIRfloat_elements(); ++ch)
        {
            addr[ch] = zero.getChannelPointer (ch);
        }
        AudioDataConverters::deinterleaveSamples (reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)),
                                                 addr,
                                                 L,
                                                 static_cast<int> (IIRfloat_elements()));
        zero.clear();
    }

}

//==============================================================================
bool MultiEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MultiEQAudioProcessor::createEditor()
{
    return new MultiEQAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void MultiEQAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    state.setProperty ("OSCPort", var (oscReceiver.getPortNumber()), nullptr);
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}


void MultiEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
void MultiEQAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG ("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "inputChannelsSetting")
        userChangedIOSettings = true;
    else if (parameterID.startsWith ("filter"))
    {
        const int i = parameterID.getLastCharacters(1).getIntValue();

        createFilterCoefficients (i, getSampleRate());

        repaintFV = true;
        userHasChangedFilterSettings = true;
    }
}

void MultiEQAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());
}

//==============================================================================
pointer_sized_int MultiEQAudioProcessor::handleVstPluginCanDo (int32 index,
                                                                     pointer_sized_int value, void* ptr, float opt)
{
    auto text = (const char*) ptr;
    auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

    if (matches ("wantsChannelCountNotifications"))
        return 1;
    return 0;
}

//==============================================================================
void MultiEQAudioProcessor::oscMessageReceived (const OSCMessage &message)
{
    String prefix ("/" + String (JucePlugin_Name));
    if (! message.getAddressPattern().toString().startsWith (prefix))
        return;

    OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring (String (JucePlugin_Name).length() + 1));

    oscParams.processOSCMessage (msg);
}

void MultiEQAudioProcessor::oscBundleReceived (const OSCBundle &bundle)
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
    return new MultiEQAudioProcessor();
}
