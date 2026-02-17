# Frontend Technical Guide: AudioPlugin02 (Svelte Edition)

This guide explains the modern Svelte-based architecture of the web UI for AudioPlugin02.

## 🏗 Architecture Overview

The frontend is now fully integrated with **Svelte**, offering a more reactive and declarative way to build the plugin UI.

### 1. Root Component (`src/App.svelte`)
`App.svelte` is the entry point of your UI. It defines the layout and mounts individual components like Knobs and Sliders.

### 2. State management with Svelte Stores (`src/state/store.ts`)
Instead of a custom store, we now use a **Svelte Writable Store**.
- **Subscription**: Components use the `$` prefix (e.g., `$store[parameterId]`) to automatically subscribe to changes.
- **Updates**: The `setParameterValue` helper handles functional updates to the state.

### 3. Svelte Actions for Controls (`src/controls/knob.ts`)
The mouse/touch interaction logic is encapsulated in **Svelte Actions**.
- **Usage**: `<div use:knobControl={parameterId}>`
- **Benefits**: Svelte automatically handles attaching/detaching event listeners when the component is created or destroyed.

### 4. Component Structure (`src/components/knob.svelte`)
Components combine HTML/SVG structure, styling, and reactive logic:
- Props define which parameter the component controls.
- Reactive declarations (`$:`) ensure calculations (like SVG paths) update instantly when the value changes.

### 5. The Bridge (`src/bridge/bridge.ts`)
Bidirectional communication remains robust:
- **JS ➡️ C++**: Uses the `juce://` URL scheme.
- **C++ ➡️ JS**: C++ calls the global `setParameterValue` function, which now updates the Svelte store, triggering a reactive UI update.

---

## 🚀 How to Add New Features

### Step 1: Backend Update
Register your new parameter in `Source/Parameters/ParameterIDs.h` and `ParameterLayout.h`.

### Step 2: Frontend Types
Add the new ID to `ParameterId` in `WebUI/src/types/parameters.ts`.

### Step 3: Initial State
Add the default values to the `initialValue` object in `WebUI/src/state/store.ts`.

### Step 4: UI Placement
Add your component to `WebUI/src/App.svelte`:
```svelte
<div class="flex items-center gap-8">
  <Knob parameterId="gain" />
  <Knob parameterId="myNewFeature" />
</div>
```

---

## 🛠 Build and Integration
1.  **Build**: Run `npm run build` in the `WebUI` directory.
2.  **Plugin Ref**: The build generates a single `index.html` file in `WebUI/dist`.
3.  **C++**: The C++ code in `PluginEditor.cpp` automatically unpacks and loads this single file.
