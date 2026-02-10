cmake_minimum_required(VERSION 3.15)

# Project name and version
project(WebViewPlugin VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE from local directory
# Assuming JUCE folder is in the same directory as this CMakeLists.txt
add_subdirectory(JUCE)

# Add the plugin target
juce_add_plugin(WebViewPlugin
    # Basic plugin settings
    COMPANY_NAME "YourCompany"                      # Change this to your company name
    PLUGIN_MANUFACTURER_CODE Yoco                   # 4 characters, at least one uppercase
    PLUGIN_CODE Wvpl                                # 4 characters, at least one uppercase
    
    # Plugin formats to build
    FORMATS AU VST3 Standalone
    
    # Product name (displayed in DAW)
    PRODUCT_NAME "WebView Filter"
    
    # Plugin characteristics
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    
    # Copy plugin after build
    COPY_PLUGIN_AFTER_BUILD TRUE
    
    # VST3 settings
    VST3_CATEGORIES Fx Filter
    
    # AU settings (macOS only)
    AU_MAIN_TYPE kAudioUnitType_Effect
    
    # Standalone settings
    PLUGIN_NAME "WebView Filter"
    PLUGIN_DESCRIPTION "Audio filter plugin with WebView interface"
)

# Add source files AFTER juce_add_plugin
target_sources(WebViewPlugin
    PRIVATE
        PluginProcessor.cpp
        PluginEditor.cpp
        PluginProcessor.h
        PluginEditor.h
        noise.cpp
        pulse.cpp
        saw.cpp
        KarplusStrong.cpp
)

# Include Resources directory for the .h files
target_include_directories(WebViewPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Resources
)

# Preprocessor definitions
target_compile_definitions(WebViewPlugin
    PUBLIC
        # JUCE settings
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        
        # Enable WebView2 on Windows with static linking
        JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
        
        # Display splash screen (set to 0 if you have a JUCE license)
        JUCE_DISPLAY_SPLASH_SCREEN=1
)

# Link JUCE modules - DO THIS BEFORE target_link_libraries with juce::juce_recommended_*
target_link_libraries(WebViewPlugin
    PRIVATE
        # JUCE modules required for audio plugin
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra              # Required for WebBrowserComponent
)

# Add recommended flags AFTER all JUCE modules
target_link_libraries(WebViewPlugin
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Platform-specific settings
if(APPLE)
    # macOS specific settings
    target_compile_definitions(WebViewPlugin
        PRIVATE
            JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
    )
    
    # Set minimum macOS version (adjust as needed)
    set_target_properties(WebViewPlugin PROPERTIES
        XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "10.13"
    )
endif()

if(WIN32)
    # Windows specific settings
    target_compile_definitions(WebViewPlugin
        PRIVATE
            JUCE_USE_WINRT_MIDI=1
    )
endif()

if(UNIX AND NOT APPLE)
    # Linux specific settings
    # Add any Linux-specific settings here if needed
endif()

# Copy Resources folder to build directory
add_custom_command(TARGET WebViewPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Resources
        $<TARGET_FILE_DIR:WebViewPlugin>/Resources
    COMMENT "Copying Resources folder to build directory"
)