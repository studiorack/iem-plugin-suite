# How to use the PluginTemplate

## Copy and rename files
 - make a copy of the folder "PluginTemplate"
 - rename it and the `PluginTemplate.jucer` file to any desired plug-in name.
     - the plug-in's name should be composed of two words with capitals (eg. NicestPlugin, SuperDecoder, HolyGrail)
     
## Change project settings
There are many project settings which have to be changed. Open the `.jucer` project and apply the following changes:
- General settings (click the gear-wheel button)
    - Project Name: your plug-in's name
    - Company Copyright: "IEM - " + your name
    - Bundle Identifier: "at.iem." + plug-in's name
    - Plugin Name
    - Plugin Description
    - Plugin Code: first two letters of the each of the two words (eg. NiPl, SuDe, HoGr)
    - Plugin AU Export Prefix: add AU to your plug-in's name
    - Plugin AAX Identifier: replace PluginTemplate with your plug-in's name
- Exporters (every configuration: Debug, Release, Debug, Release 64bit, ...)
    - Binary name: your plug-in's file name, add _win32 or _x64 to windows configurations
    
## Change file content
There are four main files in the `Source` directory:
    - `PluginEditor.cpp`
    - `PluginEditor.h`
    - `PluginProcessor.cpp`
    - `PluginProcessor.h`

Each of them has to be changed:
- replace the author name (`Daniel Rudrich`) with your name (or names if several persons are working together)
- replace every appearance of `PluginTemplate` with your plug-in's name (eg. `PluginTemplateAudioProcessor` -> `NicestPluginAudioProcessor`)

Change the TitleText in `PluginEditor.cpp` to your plug-in's name  (eg. `title.setTitle(String("Nicest"),String("Plugin"));`)