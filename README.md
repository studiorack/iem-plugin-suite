#  IEM Plug-in Suite
## Overview
The IEM Plug-in Suite is a free and Open-Source audio plugin suite including Ambisonic plug-ins up to 7th order created by staff and students of the Institute of Electronic Music and Acoustics.

For more information, installation guides and plug-in descriptions see
Website: https://plugins.iem.at
Repository: https://git.iem.at/audioplugins/IEMPluginSuite/


## Compilation Guide
All you need for compiling the IEM Plug-in Suite is the latest version of JUCE (develop branch!) and an IDE (eg. Xcode, Microsoft Visual Studio).

- Clone/download the IEMPluginSuite repository
- Open all the .jucer-files with the Projucer (part of JUCE)
- Set your global paths within the Projucer
- If necessary: add additional exporters for your IDE
- Save the project to create the exporter projects
- Open the created projects with your IDE
- Build
- Enjoy ;-)

Instead of building each plug-in seperately, you can also use the provided shell-scripts to start a batch processing. For Windows: the 'Developer Command Prompt' has to execute the win_compileAll.bat script.

## Known issues
- On Linux: RoomEncoder might only build successfully if using JUCE 5.2.0 with [13d18e40](https://github.com/WeAreROLI/JUCE/commit/13d18e406c33724a9f8870c4602458fc6d94b750) applied.
