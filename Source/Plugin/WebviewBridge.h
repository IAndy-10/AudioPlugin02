#pragma once

#include <JuceHeader.h> // Ensure you include the main header or specific modules
#include <functional>

class WebViewBridge : public juce::WebBrowserComponent {
public:
    WebViewBridge() = default;
    
    // Callback type for parameter changes from JavaScript
    using ParameterCallback = std::function<void(const juce::String&, float)>;
    
    void setParameterCallback(ParameterCallback callback) {
        parameterCallback = callback;
    }
    
    bool pageAboutToLoad(const juce::String& newURL) override {
        if (newURL.startsWith("juce://")) {
            handleBridgeURL(newURL);
            return false;
        }
        return true;
    }
    
private:
    ParameterCallback parameterCallback;
    
   void handleBridgeURL(const juce::String& urlStr) {
        juce::URL juceURL(urlStr);
        
        if (urlStr.contains("setparameter")) {
            // JUCE URL stores params in two parallel arrays (names and values)
            auto paramNames = juceURL.getParameterNames();
            auto paramValues = juceURL.getParameterValues();
            
            // Find the index of our specific parameters
            int nameIndex = paramNames.indexOf("name");
            int valueIndex = paramNames.indexOf("value");
            
            // Only proceed if both parameters exist
            if (nameIndex >= 0 && valueIndex >= 0) {
                auto paramName = paramValues[nameIndex];
                auto paramValue = paramValues[valueIndex].getFloatValue();
                
                if (parameterCallback) {
                    parameterCallback(paramName, paramValue);
                }
            }
        }
    }
};