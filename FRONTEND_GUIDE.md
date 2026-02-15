# Frontend Technical Guide: AudioPlugin02

This guide explains the architecture of the web-based UI for AudioPlugin02, its communication with the C++ backend, and how to extend it.

## 🏗 Architecture Overview

The frontend is built with **Vite**, **TypeScript**, and **Tailwind CSS**. It follows a decoupled architecture where logic, state, and rendering are separated.

### 1. Type System (`src/types/`)
All parameters are defined in `parameters.ts`. This ensures type safety across the entire app.
- **`ParameterId`**: A string union of all available parameter IDs (e.g., `'gain'`).
- **`PluginParameter`**: Interface defining the properties of a parameter (value, min, max, name).

### 2. State Management (`src/state/`)
The `store.ts` file manages the reactive state of the plugin.
- It holds the current values of all parameters.
- Components **subscribe** to the store to update their UI when a value changes.
- C++ updates the UI by calling `store.setParameterValue()`.

### 3. The Bridge (`src/bridge/`)
The `bridge.ts` file is the "translator" between JavaScript and C++.

#### **JS ➡️ C++ (Sending Data)**
When you move a knob, the UI calls `bridge.sendParameterChange()`. It uses a custom URL protocol:
```typescript
window.location.href = `juce://setparameter?name=${id}&value=${gainDb}`;
```
The C++ class `WebViewBridge` (inheriting from `juce::WebBrowserComponent`) intercepts this URL in `pageAboutToLoad` and parses the parameters.

#### **C++ ➡️ JS (Receiving Data)**
When the host (DAW) changes a parameter, C++ calls a global JavaScript function exposed on the `window` object:
```typescript
window.setParameterValue(id, value);
```
This updates the store, which in turn triggers UI updates across all subscribed components.

### 4. Controls & Components (`src/controls/` & `src/components/`)
- **Controls**: Handle raw user input (mouse events, touch events) and calculate values. They don't know how to "draw" the UI; they only handle the math.
- **Components**: Handle the visual rendering (HTML/SVG) and use Tailwind for styling. They bridge the Control logic with the State store.

---

## 🚀 How to Add New Features (Knobs/Sliders)

Follow these steps to add a new parameter to your plugin:

### Step 1: Update the C++ Backend
1.  Add a new ID to `Source/Parameters/ParameterIDs.h`.
2.  Register the parameter in `Source/Parameters/ParameterLayout.h`.

### Step 2: Update Frontend Types
In `WebUI/src/types/parameters.ts`, add your new ID to the `ParameterId` type:
```typescript
export type ParameterId = 'gain' | 'myNewParam'; // Add it here
```

### Step 3: Add to Store
In `WebUI/src/state/store.ts`, add the initial state for the new parameter:
```typescript
myNewParam: {
    id: 'myNewParam',
    name: 'New Feature',
    value: 0,
    min: 0,
    max: 100,
    defaultValue: 50
}
```

### Step 4: Add to the UI
In `WebUI/src/main.ts`, initialize the new component:
```typescript
if (app) {
    createKnob(app, 'gain');
    createKnob(app, 'myNewParam'); // Add this line
}
```

### Step 5: Build and Run
1.  Run `npm run build` in the `WebUI` folder.
2.  Rebuild your C++ project in your IDE/CMake.

---

## 🛠 Build System Details
The UI is bundled into a **single `index.html` file** using `vite-plugin-singlefile`. This ensures that even when the plugin is loaded in restricted environments, all CSS and JS are included without needing external file paths.
