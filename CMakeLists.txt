cmake_minimum_required(VERSION 3.15)

project(SimpleGainPlugin VERSION 1.0.0)

# Add JUCE as a subdirectory (you need to download JUCE first)
add_subdirectory(JUCE)

# Add binary data for HTML/CSS/JS files
juce_add_binary_data(PluginBinaryData
    SOURCES
        interface.html
        styles.css
        script.js)

# Create the plugin
juce_add_plugin(SimpleGainPlugin
    COMPANY_NAME "YourCompany"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Sgp1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Simple Gain")

# Add source files
target_sources(SimpleGainPlugin
    PRIVATE
        PluginProcessor.cpp
        PluginEditor.cpp)

# Link binary data
target_link_libraries(SimpleGainPlugin
    PRIVATE
        PluginBinaryData
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Compile definitions
target_compile_definitions(SimpleGainPlugin
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)