cmake_minimum_required(VERSION 3.15)

project(AudioPlugin02 VERSION 0.2.0)

# ============================================================
# C++ Standard
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Add JUCE Submodule
# ============================================================

add_subdirectory(external/JUCE)

# ============================================================
# Fetch JUCE (self-contained)
# ============================================================

# include(FetchContent)

# FetchContent_Declare(
    # JUCE
    # GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    # GIT_TAG 8.0.4
   #  GIT_SHALLOW TRUE
# )

# FetchContent_MakeAvailable(JUCE)

# ============================================================
# Plugin Definition
# ============================================================

juce_add_plugin(AudioPlugin02
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Ycmp
    PLUGIN_CODE Ap02
    FORMATS Standalone # VST3 AU
    PRODUCT_NAME "Audio Plugin 02"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    COPY_PLUGIN_AFTER_BUILD TRUE

    VST3_CATEGORIES Fx
    AU_MAIN_TYPE kAudioUnitType_Effect
)

juce_generate_juce_header(AudioPlugin02)

# ============================================================
# Source Files
# ============================================================

target_sources(AudioPlugin02 PRIVATE
    Source/Plugin/PluginEditor.cpp
    Source/Plugin/PluginEditor.h
    Source/Plugin/PluginProcessor.cpp
    Source/Plugin/PluginProcessor.h
    Source/Plugin/WebviewBridge.h
    Source/DSP/saw.h
    Source/DSP/saw.cpp
    Source/DSP/utils.h
    Source/Parameters/ParameterIDs.h
    Source/Parameters/ParameterLayout.h
)

target_include_directories(AudioPlugin02 PRIVATE
    Source/Plugin
    Source/DSP
    Source/Parameters
)


# ============================================================
# Embed Web Resources Into Binary
# ============================================================

juce_add_binary_data(AudioPlugin02_Resources
    SOURCES
        WebUI/dist/index.html
)


target_link_libraries(AudioPlugin02 PRIVATE
    AudioPlugin02_Resources
)

# ============================================================
# Compile Definitions
# ============================================================

target_compile_definitions(AudioPlugin02 PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

if(APPLE)
    target_compile_options(AudioPlugin02 PRIVATE
        -Wno-deprecated-declarations
    )
endif()

# ============================================================
# JUCE Modules
# ============================================================

target_link_libraries(AudioPlugin02 PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ============================================================
# Linux WebKit Dependency
# ============================================================

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.0)

    target_include_directories(AudioPlugin02 PRIVATE ${WEBKIT2_INCLUDE_DIRS})
    target_link_libraries(AudioPlugin02 PRIVATE ${WEBKIT2_LIBRARIES})
endif()


# ============================================================
# Copy Resources to Bundle
# ============================================================

# Helper macro to copy resources for a specific target
macro(copy_resources_to_target target_name)
    if(TARGET ${target_name})
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/WebUI/dist"
                "$<TARGET_FILE_DIR:${target_name}>/../Resources"
        )
    endif()
endmacro()

# Apply to all generated targets
copy_resources_to_target(AudioPlugin02)
copy_resources_to_target(AudioPlugin02_Standalone)
# copy_resources_to_target(AudioPlugin02_VST3)
# copy_resources_to_target(AudioPlugin02_AU)

# ============================================================
# Summary
# ============================================================

message(STATUS "")
message(STATUS "==============================================")
message(STATUS "Audio Plugin 02 - Embedded WebView Version")
message(STATUS "==============================================")
message(STATUS "Web resources embedded via BinaryData AND copied to bundle")
message(STATUS "==============================================")
message(STATUS "")
