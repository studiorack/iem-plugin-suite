
juce_add_plugin (DirectionalCompressor
    PLUGIN_MANUFACTURER_CODE IEMA
    PLUGIN_CODE DiCo
    ICON_BIG "../resources/Standalone/IEMPluginSuiteSmall.png"
    ICON_SMALL "../resources/Standalone/IEMPluginSuiteSmall.png"
    COMPANY_NAME "IEM"
    PRODUCT_NAME "DirectionalCompressor"
    FORMATS ${IEM_FORMATS}
    VERSION "1.3.3")


juce_generate_juce_header (DirectionalCompressor)

target_sources (DirectionalCompressor PRIVATE
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h

    ../resources/Standalone/StandaloneApp.cpp
    ../resources/Standalone/MyStandaloneFilterWindow.h
    ../resources/Standalone/IEM_JackAudio.h
    ../resources/Standalone/IEM_AudioDeviceSelectorComponent.cpp
    ../resources/Standalone/IEM_AudioDeviceSelectorComponent.h

    ../resources/OSC/OSCInputStream.h
    ../resources/OSC/OSCParameterInterface.cpp
    ../resources/OSC/OSCParameterInterface.h
    ../resources/OSC/OSCStatus.cpp
    ../resources/OSC/OSCStatus.h
    ../resources/OSC/OSCUtilities.h

    ../resources/efficientSHvanilla.cpp
    )

target_compile_definitions (DirectionalCompressor PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_include_directories (DirectionalCompressor PRIVATE Source)

target_link_libraries (DirectionalCompressor PRIVATE
    LAF_fonts
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_osc
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags)
