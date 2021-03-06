
juce_add_plugin (AllRADecoder
    PLUGIN_MANUFACTURER_CODE IEMA
    PLUGIN_CODE AlRa
    ICON_BIG "../resources/Standalone/IEMPluginSuiteSmall.png"
    ICON_SMALL "../resources/Standalone/IEMPluginSuiteSmall.png"
    COMPANY_NAME "IEM"
    PRODUCT_NAME "AllRADecoder"
    FORMATS ${IEM_FORMATS}
    VERSION "0.9.2")


juce_generate_juce_header (AllRADecoder)

target_sources (AllRADecoder PRIVATE
    Source/AmbisonicNoiseBurst.h
    Source/EnergyDistributionVisualizer.h
    Source/LoudspeakerTableComponent.h
    Source/LoudspeakerVisualizer.h
    Source/NoiseBurst.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/RotateWindow.h
    Source/tDesign5200.h

    ../resources/OSC/OSCInputStream.h
    ../resources/OSC/OSCParameterInterface.cpp
    ../resources/OSC/OSCParameterInterface.h
    ../resources/OSC/OSCStatus.cpp
    ../resources/OSC/OSCStatus.h
    ../resources/OSC/OSCUtilities.h

    ../resources/NewtonApple/NewtonApple_hull3D.h
    ../resources/NewtonApple/NewtonApple_hull3D.cpp

    ../resources/efficientSHvanilla.cpp
    )

target_compile_definitions (AllRADecoder PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_include_directories (AllRADecoder PRIVATE Source)

target_link_libraries (AllRADecoder PRIVATE
    LAF_fonts
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_osc
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags)
