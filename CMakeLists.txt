
cmake_minimum_required(VERSION 3.12)

project(IEMPluginSuite VERSION 1.12.1 LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)
set_property(GLOBAL PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD ON)

add_subdirectory (JUCE)

if (DEFINED VST2PATH)
    juce_set_vst2_sdk_path (${VST2PATH})
else()
    message (FATAL_ERROR "You have to specify the VST2PATH variable with the path to the VST2 SDK")
endif()

include_directories (resources/ /usr/local/include)
add_compile_definitions(DONT_SET_USING_JUCE_NAMESPACE=1)

juce_add_binary_data (LAF_fonts SOURCES
    resources/lookAndFeel/Roboto-Bold.ttf
    resources/lookAndFeel/Roboto-Light.ttf
    resources/lookAndFeel/Roboto-Medium.ttf
    resources/lookAndFeel/Roboto-Regular.ttf)

add_subdirectory (AllRADecoder)
add_subdirectory (BinauralDecoder)
add_subdirectory (CoordinateConverter)
add_subdirectory (DirectionalCompressor)
add_subdirectory (DirectivityShaper)
add_subdirectory (DistanceCompensator)
add_subdirectory (DualDelay)
add_subdirectory (EnergyVisualizer)
add_subdirectory (FdnReverb)
add_subdirectory (MatrixMultiplier)
add_subdirectory (MultiBandCompressor)
add_subdirectory (MultiEncoder)
add_subdirectory (MultiEQ)
add_subdirectory (OmniCompressor)
add_subdirectory (ProbeDecoder)
add_subdirectory (RoomEncoder)
add_subdirectory (SceneRotator)
add_subdirectory (SimpleDecoder)
add_subdirectory (StereoEncoder)
add_subdirectory (ToolBox)
