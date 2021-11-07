
juce_add_plugin (MultiEncoder
    PLUGIN_MANUFACTURER_CODE IEMA
    PLUGIN_CODE MuEn
    ICON_BIG "../resources/Standalone/IEMPluginSuiteSmall.png"
    ICON_SMALL "../resources/Standalone/IEMPluginSuiteSmall.png"
    COMPANY_NAME "IEM"
    PRODUCT_NAME "MultiEncoder"
    FORMATS ${IEM_FORMATS}
    VERSION "1.0.1")


juce_generate_juce_header(MultiEncoder)

target_sources(MultiEncoder PRIVATE
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

target_compile_definitions(MultiEncoder PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_include_directories(MultiEncoder PRIVATE Source)

target_link_libraries(MultiEncoder PRIVATE
    LAF_fonts
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_osc
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags)
