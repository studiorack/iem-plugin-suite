#  IEM Plug-in Suite
## Overview
The IEM Plug-in Suite is a free and Open-Source audio plug-in suite including Ambisonic plug-ins up to 7th order created by staff and students of the Institute of Electronic Music and Acoustics.

The suite provides plug-ins for a full Ambisonic production: encoders, reverbs, dynamics including limiter and multi-band compression, rotators, and decoders for both headphones and arbitrary loudspeaker layouts, and many more. The plug-ins are created with the JUCE framework and can be compiled to any major plug-in format (VST, VST3, AU, AAX).


For more information, installation guides and plug-in descriptions see:
- Website: https://plugins.iem.at
- Repository: https://git.iem.at/audioplugins/IEMPluginSuite/


## Compilation Guide
All you need for compiling the IEM Plug-in Suite is the [JUCE framework](https://juce.com) with version 5.4.5, an IDE (eg. Xcode, Microsoft Visual Studio), and the [fftw3 library](http://fftw.org) (for some of the plug-ins).

- Clone/download the IEMPluginSuite repository
- Install the fftw3 library (you might want add the paths to the Projucer projects)
- Open all the .jucer-files with the Projucer (part of JUCE)
- Set your global paths within the Projucer
- If necessary: add additional exporters for your IDE
- Save the project to create the exporter projects
- Open the created projects with your IDE
- Build
- Enjoy ;-)

The *.jucer projects are configured to build VST2,  and standalone versions. In order to build the VST2 versions of the plug-ins, you need to have a copy of the Steinberg VST2-SDK which no longer comes with JUCE. If you want to build VST3 versions, you'll have to enable it in the Projucer Project Settings -> Plugin Formats.

#### Batch processing
Instead of building each plug-in separately, you can also use the provided shell-scripts to start a batch processing:
- **macOS**:
    - open terminal
    - change the directory to the repository (e.g. `cd IEMPluginSuite-master`)
    - execute the shell script with `./macOS_buildAll.sh`
- **windows**:
    - open the *Developer Command Prompt for Visual Studio*
    - change the directoy to the repository (e.g. `cd IEMPluginSuite-master`)
    - execute the batch script with `win_buildAll.bat <pathToProjucer.exe>`
- **linux**:
    - copy the `JUCE` directory into the IEM Plug-in Suite repository
    - open terminal
    - change the directory to the repository (e.g. `cd IEMPluginSuite-master`)
    - execute the shell script with `./linux_buildAll.sh`

###  JACK support
Both on macOS and linux, the plug-in standalone version will be built with JACK support. You can disable the JACK support by adding `DONT_BUILD_WITH_JACK_SUPPORT=1` to the *Preprocessor Definitions*-field in the Projucer projects.

## Known issues
- There's an issue with the channel-layout behavior of the VST3 versions of the plug-ins. This issue comes down to the VST3 SDK and has to be fixed by Steinberg. Already reported at their developer forum.

## Related repositories
- https://git.iem.at/pd/vstplugin/releases: to use the plug-ins in PD and SuperCollider
- https://git.iem.at/ressi/iempluginosc: to control the plug-ins using the OSC interface but without an OSC connection
