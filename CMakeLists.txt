cmake_minimum_required (VERSION 3.12)

#  _____ ______ __  __   _____  _                    _          _____       _ _
# |_   _|  ____|  \/  | |  __ \| |                  (_)        / ____|     (_) |
#   | | | |__  | \  / | | |__) | |_   _  __ _        _ _ __   | (___  _   _ _| |_ ___
#   | | |  __| | |\/| | |  ___/| | | | |/ _` |______| | '_ \   \___ \| | | | | __/ _ \
#  _| |_| |____| |  | | | |    | | |_| | (_| |______| | | | |  ____) | |_| | | ||  __/
# |_____|______|_|  |_| |_|    |_|\__,_|\__, |      |_|_| |_| |_____/ \__,_|_|\__\___|
#                                        __/ |
#                                       |___/


# The following list will hold all the plug-ins which will be built.
# Simply comment them out if you want to build only a subset of the plug-ins.
set (PLUGINS_TO_BUILD
        AllRADecoder
        BinauralDecoder
        CoordinateConverter
        DirectionalCompressor
        DirectivityShaper
        DistanceCompensator
        DualDelay
        EnergyVisualizer
        FdnReverb
        MatrixMultiplier
        MultiBandCompressor
        MultiEncoder
        MultiEQ
        OmniCompressor
        ProbeDecoder
        RoomEncoder
        SceneRotator
        SimpleDecoder
        StereoEncoder
        ToolBox
    )

# The following options let you select the plug-ins formats you want to build.
# You can either change them here or use the command line to switch them on/off.
option (IEM_BUILD_VST2 "Build VST2 version of the plug-ins." ON)
option (IEM_BUILD_VST3 "Build VST3 version of the plug-ins." OFF)
option (IEM_BUILD_STANDALONE "Build standalones of the plug-ins." OFF)


# Actual project configuration
project (IEMPluginSuite VERSION 1.12.1 LANGUAGES C CXX)

set_property (GLOBAL PROPERTY USE_FOLDERS YES)
set_property (GLOBAL PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD ON)

add_subdirectory (JUCE)

# formats
set (IEM_FORMATS "")

if (IEM_BUILD_VST2)
    message ("-- IEM: Building VST2 versions")
    list (APPEND IEM_FORMATS VST)
    # checking VST2 SDK
    if (DEFINED VST2PATH)
        juce_set_vst2_sdk_path (${VST2PATH})
    else()
        message (FATAL_ERROR "You have to specify the VST2PATH variable with the path to the VST2 SDK if you want to build VST2 versions.")
    endif()
endif()

if (IEM_BUILD_VST3)
    message ("-- IEM: Building VST3 versions")
    list (APPEND IEM_FORMATS VST3)
endif()

if (IEM_BUILD_STANDALONE)
    message ("-- IEM: Building Standalone versions")
    list (APPEND IEM_FORMATS Standalone)
endif()

include_directories (resources/ /usr/local/include)
add_compile_definitions (DONT_SET_USING_JUCE_NAMESPACE=1
                         JUCE_MODAL_LOOPS_PERMITTED=1)

juce_add_binary_data (LAF_fonts SOURCES
    resources/lookAndFeel/Roboto-Bold.ttf
    resources/lookAndFeel/Roboto-Light.ttf
    resources/lookAndFeel/Roboto-Medium.ttf
    resources/lookAndFeel/Roboto-Regular.ttf)

foreach (subproject IN LISTS PLUGINS_TO_BUILD)
    add_subdirectory (${subproject})
endforeach()


if (IEM_BUILD_VST2)
    foreach (subproject IN LISTS PLUGINS_TO_BUILD)
        set_target_properties (${subproject}_VST
            PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts/VST2"
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts/VST2"
        )
    endforeach()
endif()

if (IEM_BUILD_VST3)
    foreach (subproject IN LISTS PLUGINS_TO_BUILD)
        set_target_properties (${subproject}_VST3
            PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts/VST3"
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts/VST3"
        )
    endforeach()
endif()

if (IEM_BUILD_STANDALONE)
    foreach (subproject IN LISTS PLUGINS_TO_BUILD)
        set_target_properties (${subproject}_Standalone
            PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts/Standalone"
        )
    endforeach()
endif()
