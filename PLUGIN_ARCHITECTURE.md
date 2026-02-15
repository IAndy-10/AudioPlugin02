# Audio Plugin WebView Architecture Documentation

This document explains the architecture of the Audio Plugin, specifically focusing on how the C++ backend communicates with the Web-based frontend (WebView) and how variables are synchronized between them.

## Core Concept: The WebView Bridge

The plugin uses a hybrid architecture:
- **Backend (C++)**: Handles audio processing (`PluginProcessor`) and hosts the WebView (`PluginEditor`).
- **Frontend (HTML/JS/CSS)**: Renders the UI and handles user interaction.

The **WebViewBridge** is the critical component that sits between these two worlds, allowing them to exchange data.

### 1. How Variables are Set Between Files

The communication is **bidirectional** but uses different mechanisms for each direction:

#### A. JavaScript to C++ (e.g., User turns a knob)
When a user interacts with the UI (e.g., in `script.js`), the variable is sent to C++ by **navigating to a special URL**.

1.  **Action**: The user drags the knob in the web UI.
2.  **JS Function**: `GainKnob.sendToPlugin()` constructs a custom URL:
    ```javascript
    // Format: juce://setparameter?name=PARAM_NAME&value=VALUE
    // Example: juce://setparameter?name=gain&value=-12.5
    const bridgeURL = `juce://setparameter?name=gain&value=${gainDb}`;
    window.location.href = bridgeURL;
    ```
3.  **Interception**: The `WebViewBridge` (C++) intercepts this navigation request in `pageAboutToLoad`. It sees `juce://` and stops the actual navigation.
4.  **Parsing**: `handleBridgeURL` extracts the parameter name (`gain`) and value (`-12.5`).
5.  **Update**: It triggers a callback to `PluginEditor.cpp`, which updates the actual audio parameter in the processor.

#### B. C++ to JavaScript (e.g., Automation or Init)
When the backend needs to update the UI (e.g., initial load or automation), it **injects and executes JavaScript code** directly into the WebView.

1.  **Monitor**: `PluginEditor::timerCallback` constantly checks if the processor's values have changed.
2.  **Detection**: If the gain value changes, it calls `sendGainToJS`.
3.  **Execution**: `WebViewBridge` calls `evaluateJavascript` with a string of code:
    ```cpp
    // Constructs: "if (window.gainKnob) { window.gainKnob.setValue(50.0); }"
    webView->evaluateJavascript(jsCode);
    ```
4.  **Update**: The browser executes this code, calling `GainKnob.setValue()` in `script.js`, which updates the visual knob without triggering a return message to C++ (preventing feedback loops).

---

## Detailed Method Explanation

### 1. `WebViewBridge.h` (C++)
This class inherits from `juce::WebBrowserComponent` and manages the browser instance.

*   **`pageAboutToLoad(const juce::String& newURL)`**
    *   **Purpose**: Intercepts every URL navigation before it happens.
    *   **Logic**: Checks if `newURL` starts with `juce://`.
        *   If **YES**: It calls `handleBridgeURL` and returns `false` (cancelling the navigation so the page doesn't disappear).
        *   If **NO**: It returns `true`, allowing normal loading (like `file://.../index.html`).

*   **`handleBridgeURL(const juce::String& urlStr)`**
    *   **Purpose**: Parses the custom `juce://` commands.
    *   **Logic**:
        1.  Parses the URL parameters using `juce::URL`.
        2.  Looks for keys "name" and "value".
        3.  Converts the value to a float.
        4.  Executing `parameterCallback` (which points to `PluginEditor::onParameterChangedFromJS`) to pass the data up to the Editor.

### 2. `PluginEditor.cpp` (C++)
The Editor manages the WebView and connects it to the Processor.

*   **`timerCallback()`**
    *   **Purpose**: Polls for changes in the audio processor's state.
    *   **Reason**: Audio parameters might change due to host automation, not just user interaction.
    *   **Logic**: Compares `currentGain` with `lastGainValue`. If they differ by more than `0.01f` (to handle float precision), it triggers a UI update via `sendGainToJS`.

*   **`sendGainToJS(float gainDb)`**
    *   **Purpose**: Pushes a specific gain value to the Web UI.
    *   **Logic**:
        1.  Converts dB (-60 to 0) to a percentage (0 to 100) for the UI.
        2.  Formats a standard string of JavaScript code.
        3.  Calls `webView->evaluateJavascript(...)`.

*   **`onParameterChangedFromJS(const juce::String &paramName, float value)`**
    *   **Purpose**: Receives updates *from* the WebView.
    *   **Logic**:
        1.  Identifies the parameter (checks if it is "gain").
        2.  Maps the value from UI range (dB) to normalized range (0.0 - 1.0) required by JUCE's internal `apvts` parameters.
        3.  Calls `param->setValueNotifyingHost(...)` to update the audio engine and record automation.
        4.  Updates `lastGainValue` to prevent the timer from immediately sending the same value back (echo cancellation).

### 3. `script.js` (JavaScript)
Example of the frontend logic.

*   **`sendToPlugin()`**
    *   **Purpose**: Sends the current knob value to the C++ backend.
    *   **Logic**: Formats the `juce://` URL and sets `window.location.href`. This effectively signals the C++ parent to update the parameter.

*   **`setValue(newValue)`**
    *   **Purpose**: Receives a value *from* the C++ backend.
    *   **Logic**: Updates the internal `this.value` and redraws the knob (`updateKnob()`). Crucially, it **does not** call `sendToPlugin()`, avoiding an infinite loop of updates.

---

## Methodology: How to Add New Knobs (Parameters)

To add a new parameter (e.g., Frequency, Filter Cutoff, or Effect Mix), you must modify the code in the following order:

### Step 1: Backend - Define the Parameter (C++)
1.  **`PluginProcessor.cpp`**: Locate the `parameters()` function (where `apvts` is initialized).
2.  Add a new parameter to `parameter_list`.
    ```cpp
    // Example: Adding a Frequency parameter
    parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"freq", 1},  // ID (Must normally match JS name)
        "Frequency",                   // Name
        20.0f,                         // Min
        20000.0f,                      // Max
        440.0f                         // Default
    ));
    ```

### Step 2: Backend - Use the Parameter (C++)
1.  **`PluginProcessor.cpp`**: Inside `processBlock()`, retrieve the value.
    ```cpp
    // Use the ID you defined in Step 1
    float freqValue = apvts.getRawParameterValue("freq")->load();
    ```
2.  Apply `freqValue` to your DSP algorithm (e.g., `osc->frequency(freqValue)`).

### Step 3: Connect Frontend to Backend (C++)
1.  **`PluginEditor.h`**: Add a member variable to cache the last known value (to avoid unnecessary updates).
    ```cpp
    float lastFreqValue = -1.0f;
    ```
2.  **`PluginEditor.cpp - timerCallback()`**: Add a check for your new parameter.
    ```cpp
    float currentFreq = processorRef.apvts.getRawParameterValue("freq")->load();
    if (std::abs(currentFreq - lastFreqValue) > 0.01f) {
        lastFreqValue = currentFreq;
        sendFreqToJS(currentFreq); // You need to create this function!
    }
    ```
3.  **`PluginEditor.cpp - sendFreqToJS(float value)`**: Create a helper to send the value to JS.
    ```cpp
    void AudioPluginAudioProcessorEditor::sendFreqToJS(float value) {
        // Map raw value to UI range (0-100%) if needed, or send raw
        // Example: Sending raw value
        juce::String jsCode = "if (window.freqKnob) { window.freqKnob.setValue(" +
                              juce::String(value) + "); }";
        webView->evaluateJavascript(jsCode);
    }
    ```
4.  **`PluginEditor.cpp - onParameterChangedFromJS`**: Handle incoming changes.
    ```cpp
    if (paramName == "freq") { // Must match ID from Step 1
        auto* param = processorRef.apvts.getParameter("freq");
        if (param) {
            // Normalize if necessary (JUCE params often want 0.0-1.0) or use existing range
            // If using AudioParameterFloat with NormalisableRange, setValueNotifyingHost expects 0.0-1.0 normalization.
            // If you passed the raw value from JS, you might need to map it.
            float normalized = param->convertTo0to1(value); 
            param->setValueNotifyingHost(normalized);
            
            // Update cache to prevent feedback
            lastFreqValue = value; 
        }
    }
    ```

### Step 4: Frontend - Create the UI (HTML/JS)
1.  **`index.html`**: Add the HTML container for the new knob.
    ```html
    <div class="knob-container">
        <!-- SVG content or standard knob markup -->
        <div class="knob-value" id="freq-value">440 Hz</div>
    </div>
    ```
2.  **`script.js`**: Create a new instance of your Knob class (or reuse `GainKnob` logic).
    ```javascript
    // In DOMContentLoaded
    window.freqKnob = new GainKnob('#freq-container'); // implementation depends on your JS structure
    ```
3.  **`script.js`**: Ensure the JS sends the correct name.
    ```javascript
    sendToPlugin() {
       // ... calc value ...
       // 'name=freq' MUST match the ParameterID from Step 1
       const bridgeURL = `juce://setparameter?name=freq&value=${this.value}`;
       window.location.href = bridgeURL;
    }
    ```



---

# Complete Guide: Adding New Features to AudioPlugin02

This guide walks you through adding two new features to your plugin:
1. A new knob for a reverb mix parameter
2. A synchronized visual meter for audio output level

Based on your current architecture, we'll follow the pattern you've already established.

---

## Part 1: Adding a New Knob (Reverb Mix Parameter)

### Frontend Changes

#### Step 1.1: Update Type Definitions
**File**: `src/types/parameters.ts`

```typescript
export type ParameterId = 'gain' | 'reverbMix';  // Add 'reverbMix'

export interface PluginParameter {
    id: ParameterId;
    name: string;
    value: number; // 0-100
    min: number;
    max: number;
    defaultValue: number;
}

export type ParameterState = Record<ParameterId, PluginParameter>;
```

**Why**: This ensures TypeScript knows about the new parameter everywhere in your app. If you forget to handle 'reverbMix' somewhere, TypeScript will error.

---

#### Step 1.2: Update the Store
**File**: `src/state/store.ts`

```typescript
class Store {
    private state: ParameterState = {
        gain: {
            id: 'gain',
            name: 'Gain',
            value: 50,
            min: 0,
            max: 100,
            defaultValue: 50
        },
        reverbMix: {  // Add this new parameter
            id: 'reverbMix',
            name: 'Reverb Mix',
            value: 20,
            min: 0,
            max: 100,
            defaultValue: 20
        }
    };

    // Rest of the store code stays the same
    // ...
}

export const store = new Store();
```

**Why**: The store is your single source of truth. All parameters must be defined here so components can read and subscribe to changes.

---

#### Step 1.3: Create the Reverb Knob HTML Template
**File**: `src/components/reverbKnob.html` (new file)

```html
<div class="flex flex-col items-center gap-4 p-8 bg-plugin-panel rounded-xl shadow-2xl w-64">
    <div class="relative w-48 h-48 cursor-pointer select-none knob-wrapper" data-knob>
        <svg viewBox="0 0 200 200" class="w-full h-full">
            <circle cx="100" cy="100" r="70" class="fill-none stroke-plugin-border stroke-[8]"></circle>
            <path class="knob-progress fill-none stroke-plugin-reverb stroke-[8] stroke-linecap-round transition-[stroke] duration-200 hover:stroke-plugin-reverbHover" data-progress></path>
            <circle cx="100" cy="100" r="55" class="fill-plugin-knob stroke-[#404040] stroke-[2]"></circle>
            <line x1="100" y1="100" x2="100" y2="45" class="knob-indicator stroke-white stroke-[3] stroke-linecap-round" data-indicator></line>
        </svg>
        <div class="knob-value absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 text-2xl font-semibold text-white pointer-events-none drop-shadow-md" data-value></div>
    </div>
    <div class="text-sm font-semibold tracking-widest text-plugin-textMuted uppercase" data-name></div>
</div>
```

**Why**: Same structure as the gain knob, just a separate template. This keeps your HTML organized and reusable.

---

#### Step 1.4: Create the Reverb Knob Component Factory
**File**: `src/components/reverbKnobFactory.ts` (new file)

```typescript
import { store } from '../state/store';
import { ParameterId } from '../types/parameters';
import { KnobControl } from '../controls/knob';
import reverbKnobHTML from './reverbKnob.html?raw';

export async function createReverbKnob(container: HTMLElement, parameterId: ParameterId) {
    const state = store.getState()[parameterId];

    // Use the imported HTML directly
    container.innerHTML = reverbKnobHTML;

    // Query elements by data attributes
    const knobWrapper = container.querySelector('[data-knob]') as HTMLElement;
    const progressPath = container.querySelector('[data-progress]') as SVGPathElement;
    const indicator = container.querySelector('[data-indicator]') as SVGLineElement;
    const valueDisplay = container.querySelector('[data-value]') as HTMLElement;
    const nameDisplay = container.querySelector('[data-name]') as HTMLElement;

    // Initialize with current values
    valueDisplay.textContent = `${state.value}%`;
    nameDisplay.textContent = state.name;

    // Attach drag interaction
    new KnobControl(knobWrapper, parameterId);

    // Define reactive updates
    const updateUI = () => {
        const value = store.getState()[parameterId].value;
        const minAngle = -135;
        const maxAngle = 135;
        const angleRange = maxAngle - minAngle;
        const currentAngle = minAngle + (value / 100) * angleRange;

        // Update SVG Path
        progressPath.setAttribute('d', describeArc(100, 100, 70, minAngle, currentAngle));

        // Update Indicator
        indicator.style.transform = `rotate(${currentAngle}deg)`;
        indicator.style.transformOrigin = '100px 100px';

        // Update Value
        valueDisplay.textContent = `${value}%`;
    };

    // Subscribe to store changes
    store.subscribe(updateUI);
    
    // Initial render
    updateUI();
}

function polarToCartesian(centerX: number, centerY: number, radius: number, angleInDegrees: number) {
    const angleInRadians = (angleInDegrees - 90) * Math.PI / 180.0;
    return {
        x: centerX + (radius * Math.cos(angleInRadians)),
        y: centerY + (radius * Math.sin(angleInRadians))
    };
}

function describeArc(x: number, y: number, radius: number, startAngle: number, endAngle: number) {
    const start = polarToCartesian(x, y, radius, endAngle);
    const end = polarToCartesian(x, y, radius, startAngle);
    const largeArcFlag = endAngle - startAngle <= 180 ? "0" : "1";

    return [
        "M", start.x, start.y,
        "A", radius, radius, 0, largeArcFlag, 0, end.x, end.y
    ].join(" ");
}
```

**Why**: Identical logic to the gain knob, just using the reverb template. This demonstrates how reusable your component factory pattern is.

---

#### Step 1.5: Update the App Initializer
**File**: `src/app.ts`

```typescript
import { createKnob } from './components/knobFactory';
import { createReverbKnob } from './components/reverbKnobFactory';
import { store } from './state/store';

interface ComponentConfig {
    type: 'knob' | 'reverbKnob' | 'slider' | 'meter';
    parameterId?: string;
}

const COMPONENTS: ComponentConfig[] = [
    { type: 'knob', parameterId: 'gain' },
    { type: 'reverbKnob', parameterId: 'reverbMix' },  // Add this
];

export async function initializeApp() {
    const app = document.getElementById('app');
    
    if (!app) {
        throw new Error('App container (#app) not found in DOM');
    }

    console.log('🎛️ Initializing Audio Plugin UI');

    for (const config of COMPONENTS) {
        try {
            const container = document.createElement('div');
            app.appendChild(container);

            if (config.type === 'knob') {
                await createKnob(container, config.parameterId as any);
                console.log(`✓ Mounted knob: ${config.parameterId}`);
            } else if (config.type === 'reverbKnob') {  // Add this
                await createReverbKnob(container, config.parameterId as any);
                console.log(`✓ Mounted reverb knob: ${config.parameterId}`);
            }
        } catch (error) {
            console.error(`✗ Failed to mount ${config.type}/${config.parameterId}:`, error);
        }
    }

    console.log('✓ Plugin UI initialized successfully');
}
```

**Why**: Centralizes component initialization. Easy to add/remove components without touching main.ts.

---

### Backend Changes (C++)

#### Step 2.1: Add Parameter to C++ Definition
**File**: `PluginProcessor.cpp` (in `createParameterLayout()`)

```cpp
// In your parameter creation section
parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"reverbMix", 1},
    "Reverb Mix",
    juce::NormalisableRange<float>(0.0f, 100.0f),  // 0-100%
    20.0f,                                          // Default: 20%
    juce::AudioParameterFloatAttributes()
        .withLabel("%")
));
```

**Why**: Defines the parameter in JUCE's audio processor. This is what the DAW (and your plugin) sees.

---

#### Step 2.2: Use the Parameter in Audio Processing
**File**: `PluginProcessor.cpp` (in `processBlock()`)

```cpp
void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, 
                                             juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    
    // Get the reverb mix parameter (0.0 - 100.0)
    float reverbMixPercent = apvts.getRawParameterValue("reverbMix")->load();
    float reverbMixNormalized = reverbMixPercent / 100.0f;  // Convert to 0.0-1.0
    
    // Apply your reverb effect with the mix amount
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float drySignal = channelData[sample];
            float wetSignal = reverbProcessor.process(drySignal);
            
            // Mix dry and wet signals based on reverbMix parameter
            channelData[sample] = drySignal * (1.0f - reverbMixNormalized) 
                                + wetSignal * reverbMixNormalized;
        }
    }
}
```

**Why**: Actually uses the parameter value in your audio algorithm.

---

#### Step 2.3: Sync UI with Audio Engine
**File**: `PluginEditor.h`

```cpp
private:
    float lastReverbMixValue = -1.0f;
```

**File**: `PluginEditor.cpp` (add to `timerCallback()`)

```cpp
void AudioPluginAudioProcessorEditor::timerCallback() {
    // Existing gain code...
    
    // Add reverb mix monitoring
    float currentReverbMix = processorRef.apvts.getRawParameterValue("reverbMix")->load();
    if (std::abs(currentReverbMix - lastReverbMixValue) > 0.01f) {
        lastReverbMixValue = currentReverbMix;
        sendReverbMixToJS(currentReverbMix);
    }
}

void AudioPluginAudioProcessorEditor::sendReverbMixToJS(float value) {
    juce::String jsCode = "if (window.reverbKnob) { window.reverbKnob.setValue(" 
                        + juce::String(value) + "); }";
    webView->evaluateJavascript(jsCode);
}
```

**Why**: Keeps the UI in sync when external sources change the parameter.

---

#### Step 2.4: Handle UI Changes from JavaScript
**File**: `PluginEditor.cpp` (in `onParameterChangedFromJS()`)

```cpp
void AudioPluginAudioProcessorEditor::onParameterChangedFromJS(
    const juce::String& paramName, 
    float value) {
    
    if (paramName == "gain") {
        // Existing gain handling...
        auto* param = processorRef.apvts.getParameter("gain");
        if (param) {
            param->setValueNotifyingHost(param->convertTo0to1(value));
            lastGainValue = value;
        }
    }
    else if (paramName == "reverbMix") {  // Add this
        auto* param = processorRef.apvts.getParameter("reverbMix");
        if (param) {
            // reverbMix is 0-100%, convert to 0.0-1.0 for JUCE
            float normalized = value / 100.0f;
            param->setValueNotifyingHost(normalized);
            lastReverbMixValue = value;
        }
    }
}
```

**Why**: Receives changes from the UI, updates the audio processor, and records automation.

---

## Part 2: Adding a Visual Meter (Complex Synchronized Visual)

This part adds a real-time audio level meter that updates as audio is processed.

### Frontend Changes

#### Step 3.1: Update State for Meter Data
**File**: `src/state/store.ts`

Add a new store section for meter data:

```typescript
interface MeterState {
    outputLevel: number;  // -60 to 0 dB
    peakLevel: number;    // -60 to 0 dB
}

class Store {
    private state: ParameterState = { /* ... */ };
    private meterState: MeterState = {
        outputLevel: -60,
        peakLevel: -60
    };
    
    private listeners: Set<() => void> = new Set();
    private meterListeners: Set<(meter: MeterState) => void> = new Set();

    // ... existing methods ...

    getMeterState() {
        return this.meterState;
    }

    setMeterData(outputLevel: number, peakLevel: number) {
        this.meterState.outputLevel = outputLevel;
        this.meterState.peakLevel = peakLevel;
        this.notifyMeterListeners();
    }

    subscribeMeter(listener: (meter: MeterState) => void) {
        this.meterListeners.add(listener);
        return () => this.meterListeners.delete(listener);
    }

    private notifyMeterListeners() {
        this.meterListeners.forEach(l => l(this.meterState));
    }
}

export const store = new Store();
```

**Why**: Separates meter data from parameter data. Meters update very frequently.

---

#### Step 3.2: Create Meter HTML Template
**File**: `src/components/meter.html` (new file)

```html
<div class="flex flex-col items-center gap-4 p-8 bg-plugin-panel rounded-xl shadow-2xl w-80">
    <div class="w-full">
        <div class="text-sm font-semibold tracking-widest text-plugin-textMuted uppercase mb-4">Output Level</div>
        
        <!-- Main level meter -->
        <div class="relative w-full h-8 bg-[#1a1a1a] rounded border border-plugin-border overflow-hidden">
            <!-- RMS Level Bar (green) -->
            <div class="meter-level absolute left-0 top-0 h-full bg-gradient-to-r from-plugin-accent to-plugin-accentHover transition-all duration-75" data-level></div>
            
            <!-- dB Scale labels -->
            <div class="absolute inset-0 flex px-1 text-xs text-[#808080]">
                <span class="flex-1">-60dB</span>
                <span class="flex-1">-40dB</span>
                <span class="flex-1">-20dB</span>
                <span class="flex-1">0dB</span>
            </div>
        </div>
        
        <!-- Peak Level Indicator -->
        <div class="mt-4 flex items-center justify-between">
            <span class="text-xs font-semibold text-plugin-textMuted uppercase">Peak</span>
            <div class="relative h-6 w-32 bg-[#1a1a1a] rounded border border-plugin-border flex items-center px-2">
                <div class="meter-peak absolute left-0 top-0 h-full w-1 bg-red-500 rounded-l" data-peak></div>
                <span class="text-xs font-semibold text-white ml-2" data-peak-value>-60dB</span>
            </div>
        </div>
    </div>
    
    <!-- Current Values Display -->
    <div class="w-full grid grid-cols-2 gap-4 pt-4 border-t border-plugin-border">
        <div class="text-center">
            <div class="text-xs text-plugin-textMuted uppercase">RMS</div>
            <div class="text-lg font-semibold text-plugin-accent" data-rms-display>-60dB</div>
        </div>
        <div class="text-center">
            <div class="text-xs text-plugin-textMuted uppercase">Peak</div>
            <div class="text-lg font-semibold text-red-500" data-peak-display>-60dB</div>
        </div>
    </div>
</div>
```

**Why**: Clean, professional meter with RMS level bar and peak indicator.

---

#### Step 3.3: Create Meter Component Factory
**File**: `src/components/meterFactory.ts` (new file)

```typescript
import { store } from '../state/store';
import meterHTML from './meter.html?raw';

export function createMeter(container: HTMLElement) {
    // Load the HTML template
    container.innerHTML = meterHTML;

    // Query meter elements
    const levelBar = container.querySelector('[data-level]') as HTMLElement;
    const peakIndicator = container.querySelector('[data-peak]') as HTMLElement;
    const rmsDisplay = container.querySelector('[data-rms-display]') as HTMLElement;
    const peakDisplay = container.querySelector('[data-peak-display]') as HTMLElement;
    const peakValue = container.querySelector('[data-peak-value]') as HTMLElement;

    // Helper function to convert dB to percentage
    const dbToPercent = (db: number): number => {
        const clamped = Math.max(-60, Math.min(0, db));
        return ((clamped + 60) / 60) * 100;
    };

    // Subscribe to meter updates from C++
    const updateMeter = () => {
        const meterState = store.getMeterState();
        
        // Convert dB (-60 to 0) to percentage (0 to 100) for visual bar
        const levelPercent = dbToPercent(meterState.outputLevel);
        const peakPercent = dbToPercent(meterState.peakLevel);

        // Update level bar width
        levelBar.style.width = `${levelPercent}%`;

        // Update peak indicator position
        peakIndicator.style.left = `${peakPercent}%`;

        // Update text displays
        rmsDisplay.textContent = `${meterState.outputLevel.toFixed(1)}dB`;
        peakDisplay.textContent = `${meterState.peakLevel.toFixed(1)}dB`;
        peakValue.textContent = `${meterState.peakLevel.toFixed(1)}dB`;

        // Change color based on level (green -> yellow -> red)
        if (meterState.outputLevel < -20) {
            levelBar.className = 'meter-level absolute left-0 top-0 h-full bg-gradient-to-r from-plugin-accent to-plugin-accentHover transition-all duration-75';
        } else if (meterState.outputLevel < -3) {
            levelBar.className = 'meter-level absolute left-0 top-0 h-full bg-gradient-to-r from-yellow-500 to-yellow-400 transition-all duration-75';
        } else {
            levelBar.className = 'meter-level absolute left-0 top-0 h-full bg-gradient-to-r from-red-600 to-red-500 transition-all duration-75';
        }
    };

    // Subscribe to meter updates
    store.subscribeMeter(updateMeter);
    
    // Initial update
    updateMeter();
}
```

**Why**: Subscribes to meter data updates and changes the DOM in real-time.

---

#### Step 3.4: Add Meter to App Initializer
**File**: `src/app.ts` (update)

```typescript
import { createKnob } from './components/knobFactory';
import { createReverbKnob } from './components/reverbKnobFactory';
import { createMeter } from './components/meterFactory';

interface ComponentConfig {
    type: 'knob' | 'reverbKnob' | 'meter' | 'slider';
    parameterId?: string;
}

const COMPONENTS: ComponentConfig[] = [
    { type: 'meter' },  // Add meter first so it appears at top
    { type: 'knob', parameterId: 'gain' },
    { type: 'reverbKnob', parameterId: 'reverbMix' },
];

export async function initializeApp() {
    const app = document.getElementById('app');
    
    if (!app) {
        throw new Error('App container (#app) not found in DOM');
    }

    console.log('🎛️ Initializing Audio Plugin UI');

    for (const config of COMPONENTS) {
        try {
            const container = document.createElement('div');
            app.appendChild(container);

            if (config.type === 'knob') {
                await createKnob(container, config.parameterId as any);
                console.log(`✓ Mounted knob: ${config.parameterId}`);
            } else if (config.type === 'reverbKnob') {
                await createReverbKnob(container, config.parameterId as any);
                console.log(`✓ Mounted reverb knob: ${config.parameterId}`);
            } else if (config.type === 'meter') {
                createMeter(container);  // Note: not async
                console.log(`✓ Mounted meter`);
            }
        } catch (error) {
            console.error(`✗ Failed to mount ${config.type}:`, error);
        }
    }

    console.log('✓ Plugin UI initialized successfully');
}
```

**Why**: Integrates the meter into your component system.

---

### Backend Changes (C++)

#### Step 4.1: Add Meter Calculation to Audio Processor
**File**: `PluginProcessor.h`

```cpp
public:
    float getOutputLevelDb() const { return outputLevelDb; }
    float getPeakLevelDb() const { return peakLevelDb; }

private:
    float outputLevelDb = -60.0f;
    float peakLevelDb = -60.0f;
    
    float calculateLevel(const juce::AudioBuffer<float>& buffer);
```

**File**: `PluginProcessor.cpp`

```cpp
float AudioPluginAudioProcessor::calculateLevel(const juce::AudioBuffer<float>& buffer) {
    float maxSample = 0.0f;
    
    // Find the maximum absolute sample value
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const float* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            maxSample = std::max(maxSample, std::abs(channelData[sample]));
        }
    }
    
    // Prevent log(0)
    if (maxSample == 0.0f) {
        return -60.0f;
    }
    
    // Convert to dB (20 * log10(level))
    return 20.0f * std::log10(maxSample);
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, 
                                             juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    
    // ... existing audio processing ...
    
    // Calculate output level at the end of processing
    outputLevelDb = calculateLevel(buffer);
    
    // Simple peak hold (decays over time)
    if (outputLevelDb > peakLevelDb) {
        peakLevelDb = outputLevelDb;
    } else {
        // Decay peak slowly (e.g., -1 dB per second)
        peakLevelDb -= 0.001f; // Adjust decay rate as needed
    }
}
```

**Why**: Calculates levels from the audio buffer. These will be sent to the UI.

---

#### Step 4.2: Send Meter Data to UI
**File**: `PluginEditor.cpp` (in `timerCallback()`)

```cpp
void AudioPluginAudioProcessorEditor::timerCallback() {
    // Existing parameter sync code...
    
    // Send meter data very frequently (every timer tick)
    float currentOutputLevel = processorRef.getOutputLevelDb();
    float currentPeakLevel = processorRef.getPeakLevelDb();
    
    sendMeterDataToJS(currentOutputLevel, currentPeakLevel);
}

void AudioPluginAudioProcessorEditor::sendMeterDataToJS(float outputLevel, float peakLevel) {
    juce::String jsCode = R"(
        if (window.onBackendMessage) {
            window.onBackendMessage({
                type: 'meterData',
                outputLevel: )" + juce::String(outputLevel) + R"(,
                peakLevel: )" + juce::String(peakLevel) + R"(
            });
        }
    )";
    
    webView->evaluateJavascript(jsCode);
}
```

**Why**: Sends meter data to the frontend via the bridge.

---

#### Step 4.3: Update Bridge to Handle Meter Messages
**File**: `src/bridge/bridge.ts` (update `handleBackendMessage`)

```typescript
private handleBackendMessage(message: any) {
    if (!isBackendMessage(message)) {
        this.error('Invalid backend message:', message);
        return;
    }

    switch (message.type) {
        case 'setParameterValue':
            this.handleParameterValueFromBackend(message.parameterId, message.value);
            break;

        case 'meterData':  // Add this
            store.setMeterData(message.outputLevel, message.peakLevel);
            break;

        case 'audioState':
            this.log(`Audio state: playing=${message.isPlaying}, bypassed=${message.isBypassed}`);
            break;

        // ... other cases ...
    }
}
```

**Why**: The bridge receives meter messages and passes them to the store.

---

## Summary: Complete Checklist

### Adding Reverb Mix Knob:

**Frontend:**
- [ ] Add 'reverbMix' to `ParameterId` type in `parameters.ts`
- [ ] Add reverbMix to store initial state in `store.ts`
- [ ] Create `src/components/reverbKnob.html`
- [ ] Create `src/components/reverbKnobFactory.ts`
- [ ] Add to `COMPONENTS` array in `app.ts`
- [ ] Handle 'reverbKnob' type in `initializeApp()`

**Backend (C++):**
- [ ] Add reverbMix parameter to processor parameter layout
- [ ] Use parameter in `processBlock()` for reverb effect
- [ ] Add `lastReverbMixValue` to Editor header
- [ ] Implement `sendReverbMixToJS()` method
- [ ] Add monitoring in `timerCallback()`
- [ ] Handle in `onParameterChangedFromJS()`

**Testing:**
- [ ] Knob responds to drag gestures
- [ ] Value syncs to C++ backend
- [ ] Audio changes with reverb mix
- [ ] External changes (host automation) sync back to UI

---

### Adding Output Meter:

**Frontend:**
- [ ] Add `MeterState` interface to `store.ts`
- [ ] Add `setMeterData()` and `subscribeMeter()` methods
- [ ] Create `src/components/meter.html`
- [ ] Create `src/components/meterFactory.ts`
- [ ] Add to `COMPONENTS` array in `app.ts`
- [ ] Handle 'meter' type in `initializeApp()`
- [ ] Update bridge to handle 'meterData' messages

**Backend (C++):**
- [ ] Add meter variables to processor (`outputLevelDb`, `peakLevelDb`)
- [ ] Implement `calculateLevel()` function
- [ ] Call in `processBlock()` after audio processing
- [ ] Add getters in processor header
- [ ] Implement `sendMeterDataToJS()` in Editor
- [ ] Call from `timerCallback()`

**Testing:**
- [ ] Meter displays -60dB on silence
- [ ] Meter responds to audio in real-time
- [ ] Color changes at -20dB and -3dB thresholds
- [ ] Peak indicator shows peak level
- [ ] No performance issues from frequent updates

---

## Architecture You're Following

```
C++ Backend (Audio Engine)
    ↓ evaluateJavascript()
    ↓ (sends data messages)
    ↓
TypeScript Bridge
    ↓ isBackendMessage()
    ↓ (validates & routes)
    ↓
Store (Single Source of Truth)
    ├─ ParameterState
    └─ MeterState
    ↓ subscribe/subscribeMeter
    ↓ (notifies listeners)
    ↓
Components (UI)
    ├─ Knobs
    ├─ ReverbKnob
    └─ Meter
    ↓ Updates DOM/SVG
```

Every feature follows this: **Type Safety → Store → Components → Bridge ↔ Backend**

This architecture scales beautifully as you add more features! 🚀