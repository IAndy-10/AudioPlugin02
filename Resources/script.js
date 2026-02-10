// Audio Plugin 01 - JavaScript Controller
// Handles UI interactions and communication with the JUCE plugin

// State management
let state = {
    gain: 0.5,
    frequency: 60,
    virtualFilter: 1.0,
    pulseWidth: 0.5,
    noiseColor: 1,
    noiseFadeIn: 200,
    waveformType: 0,
    pulseFilter: 1.0
};

// Noise color names
const noiseColorNames = ['White', 'Pink', 'Brown'];

// DOM Elements
const waveformBox = document.getElementById('waveformBox');
const pluckButton = document.getElementById('pluckButton');

const gainSlider = document.getElementById('gainSlider');
const gainValue = document.getElementById('gainValue');

const freqSlider = document.getElementById('freqSlider');
const freqValue = document.getElementById('freqValue');

const virtualFilterSlider = document.getElementById('virtualFilterSlider');
const virtualFilterValue = document.getElementById('virtualFilterValue');

const pulseWidthSlider = document.getElementById('pulseWidthSlider');
const pulseWidthValue = document.getElementById('pulseWidthValue');

const noiseColorSlider = document.getElementById('noiseColorSlider');
const noiseColorValue = document.getElementById('noiseColorValue');

const noiseFadeSlider = document.getElementById('noiseFadeSlider');
const noiseFadeValue = document.getElementById('noiseFadeValue');

// Initialize the UI
function init() {
    console.log('Audio Plugin 01 UI Initialized');

    // Set up event listeners
    setupEventListeners();

    // Initialize UI state
    updateAllKnobs();
}

// Set up all event listeners
function setupEventListeners() {
    // Waveform selection
    waveformBox.addEventListener('change', handleWaveformChange);
    
    // Pluck button
    pluckButton.addEventListener('click', handlePluckClick);

    // Sliders
    gainSlider.addEventListener('input', handleGainChange);
    freqSlider.addEventListener('input', handleFrequencyChange);
    virtualFilterSlider.addEventListener('input', handleVirtualFilterChange);
    pulseWidthSlider.addEventListener('input', handlePulseWidthChange);
    noiseColorSlider.addEventListener('input', handleNoiseColorChange);
    noiseFadeSlider.addEventListener('input', handleNoiseFadeChange);
}

// Update knob rotation based on value
function updateKnobRotation(slider, value, min, max) {
    const percentage = ((value - min) / (max - min)) * 100;
    slider.style.setProperty('--value', percentage);
}

// Update all knobs
function updateAllKnobs() {
    updateKnobRotation(gainSlider, gainSlider.value, 0, 100);
    updateKnobRotation(freqSlider, freqSlider.value, 36, 96);
    updateKnobRotation(virtualFilterSlider, virtualFilterSlider.value, 0, 100);
    updateKnobRotation(pulseWidthSlider, pulseWidthSlider.value, 0, 100);
    updateKnobRotation(noiseColorSlider, noiseColorSlider.value, 0, 2);
    updateKnobRotation(noiseFadeSlider, noiseFadeSlider.value, 0, 1000);
}

// Handle waveform type changes
function handleWaveformChange(event) {
    const type = parseInt(event.target.value);
    state.waveformType = type;

    console.log('Waveform changed to:', type);

    // Send to plugin if available
    if (typeof window.setWaveformType === 'function') {
        window.setWaveformType(type);
    }
}

// Handle pluck button click
function handlePluckClick() {
    console.log('Pluck triggered');

    // Send to plugin if available
    if (typeof window.triggerPluck === 'function') {
        window.triggerPluck();
    }
}

let lastUpdateTime = 0;
const throttleRate = 16; // ms

function handleGainChange(event) {
    const value = parseFloat(event.target.value) / 100.0;
    state.gain = value;
    
    // Immediate UI Update (Keep this fast!)
    gainValue.textContent = Math.round(value * 100) + ' %';
    updateKnobRotation(gainSlider, event.target.value, 0, 100);

    // Throttled Plugin Update
    const now = Date.now();
    if (now - lastUpdateTime > throttleRate) {
        if (typeof window.setGain === 'function') {
            window.setGain(value);
        }
        lastUpdateTime = now;
    }
}

// Handle frequency changes
function handleFrequencyChange(event) {
    const value = parseFloat(event.target.value);
    state.frequency = value;

    // Update display
    freqValue.textContent = Math.round(value);
    updateKnobRotation(freqSlider, value, 36, 96);

    // Send to plugin if available
    if (typeof window.setFrequency === 'function') {
        window.setFrequency(value);
    }
}

// Handle virtual filter changes
function handleVirtualFilterChange(event) {
    const value = parseFloat(event.target.value) / 100.0;
    state.virtualFilter = value;

    // Update display
    virtualFilterValue.textContent = Math.round(value * 100) + ' %';
    updateKnobRotation(virtualFilterSlider, event.target.value, 0, 100);

    // Send to plugin if available
    if (typeof window.setVirtualFilter === 'function') {
        window.setVirtualFilter(value);
    }
}

// Handle pulse width changes
function handlePulseWidthChange(event) {
    const value = parseFloat(event.target.value) / 100.0;
    state.pulseWidth = value;

    // Update display
    pulseWidthValue.textContent = Math.round(value * 100) + ' %';
    updateKnobRotation(pulseWidthSlider, event.target.value, 0, 100);

    // Send to plugin if available
    if (typeof window.setPulseWidth === 'function') {
        window.setPulseWidth(value);
    }
}

// Handle noise color changes
function handleNoiseColorChange(event) {
    const value = parseInt(event.target.value);
    state.noiseColor = value;

    // Update display
    noiseColorValue.textContent = noiseColorNames[value];
    updateKnobRotation(noiseColorSlider, value, 0, 2);

    // Send to plugin if available
    if (typeof window.setNoiseColor === 'function') {
        window.setNoiseColor(value);
    }
}

// Handle noise fade changes
function handleNoiseFadeChange(event) {
    const value = parseFloat(event.target.value);
    state.noiseFadeIn = value;

    // Update display
    noiseFadeValue.textContent = Math.round(value) + ' ms';
    updateKnobRotation(noiseFadeSlider, value, 0, 1000);

    // Send to plugin if available
    if (typeof window.setNoiseFadeIn === 'function') {
        window.setNoiseFadeIn(value);
    }
}

// Called from plugin to update UI with current parameter values
function updateFromPlugin(gain, frequency, filterType, pulseWidth, noiseColor, 
                         noiseFadeIn, waveformType, pulseFilter) {
    // Update state
    state.gain = gain;
    state.frequency = frequency;
    state.pulseWidth = pulseWidth;
    state.noiseColor = noiseColor;
    state.noiseFadeIn = noiseFadeIn;
    state.waveformType = waveformType;
    state.pulseFilter = pulseFilter;

    // Update UI elements without triggering callbacks
    waveformBox.value = waveformType;
    gainSlider.value = Math.round(gain * 100);
    freqSlider.value = frequency;
    virtualFilterSlider.value = Math.round(pulseFilter * 100);
    pulseWidthSlider.value = Math.round(pulseWidth * 100);
    noiseColorSlider.value = noiseColor;
    noiseFadeSlider.value = noiseFadeIn;

    // Update displays
    gainValue.textContent = Math.round(gain * 100) + ' %';
    freqValue.textContent = Math.round(frequency);
    virtualFilterValue.textContent = Math.round(pulseFilter * 100) + ' %';
    pulseWidthValue.textContent = Math.round(pulseWidth * 100) + ' %';
    noiseColorValue.textContent = noiseColorNames[noiseColor];
    noiseFadeValue.textContent = Math.round(noiseFadeIn) + ' ms';

    // Update knob rotations
    updateAllKnobs();
}

// Debug logging
function logState() {
    console.log('Current State:', state);
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
} else {
    init();
}

// Export functions for plugin access
window.updateFromPlugin = updateFromPlugin;
window.logState = logState;