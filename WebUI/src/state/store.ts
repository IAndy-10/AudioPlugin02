import { writable } from 'svelte/store';
import type { ParameterState } from '../types/parameters';

// All values are normalized 0-1 (JUCE param->getValue() range).
const initialState: ParameterState = {
    // Input Filter
    loCutEnabled: { id: 'loCutEnabled', name: 'Lo Cut',      value: 0,    min: 0, max: 1, defaultValue: 0 },
    hiCutEnabled: { id: 'hiCutEnabled', name: 'Hi Cut',      value: 0,    min: 0, max: 1, defaultValue: 0 },
    loCutFreq:    { id: 'loCutFreq',    name: 'Lo Cut Freq', value: 0.35, min: 0, max: 1, defaultValue: 0.35 },
    hiCutFreq:    { id: 'hiCutFreq',    name: 'Hi Cut Freq', value: 0.42, min: 0, max: 1, defaultValue: 0.42 },

    // Early Reflections
    erEnabled: { id: 'erEnabled', name: 'Spin',   value: 0,   min: 0, max: 1, defaultValue: 0 },
    erAmount:  { id: 'erAmount',  name: 'Amount', value: 0.3, min: 0, max: 1, defaultValue: 0.3 },
    erRate:    { id: 'erRate',    name: 'Rate',   value: 0.3, min: 0, max: 1, defaultValue: 0.3 },
    erShape:   { id: 'erShape',   name: 'Shape',  value: 0.5, min: 0, max: 1, defaultValue: 0.5 },

    // FDN Core
    reverbMode:    { id: 'reverbMode',    name: 'Mode',      value: 0,    min: 0, max: 1, defaultValue: 0 },
    crossoverFreq: { id: 'crossoverFreq', name: 'Crossover', value: 0.5,  min: 0, max: 1, defaultValue: 0.5 },
    diffusion:     { id: 'diffusion',     name: 'Diffusion', value: 0.6,  min: 0, max: 1, defaultValue: 0.6 },
    scale:         { id: 'scale',         name: 'Scale',     value: 0.5,  min: 0, max: 1, defaultValue: 0.5 },
    decay:         { id: 'decay',         name: 'Decay',     value: 0.42, min: 0, max: 1, defaultValue: 0.42 },
    damping:       { id: 'damping',       name: 'Damping',   value: 0.5,  min: 0, max: 1, defaultValue: 0.5 },
    feedback:      { id: 'feedback',      name: 'Feedback',  value: 0.75, min: 0, max: 1, defaultValue: 0.75 },

    // Chorus
    chorusAmount: { id: 'chorusAmount', name: 'Amount', value: 0.2,  min: 0, max: 1, defaultValue: 0.2 },
    chorusRate:   { id: 'chorusRate',   name: 'Rate',   value: 0.27, min: 0, max: 1, defaultValue: 0.27 },

    // Output
    reflectGain: { id: 'reflectGain', name: 'Reflect', value: 0.8, min: 0, max: 1, defaultValue: 0.8 },
    diffuseGain: { id: 'diffuseGain', name: 'Diffuse', value: 0.8, min: 0, max: 1, defaultValue: 0.8 },
    dryWet:      { id: 'dryWet',      name: 'Dry/Wet', value: 0.5, min: 0, max: 1, defaultValue: 0.5 },

    // Bottom Utility Row
    predelay: { id: 'predelay', name: 'Predelay', value: 0.08, min: 0, max: 1, defaultValue: 0.08 },
    smooth:   { id: 'smooth',   name: 'Smooth',   value: 0,    min: 0, max: 1, defaultValue: 0 },
    size:     { id: 'size',     name: 'Size',      value: 0.5,  min: 0, max: 1, defaultValue: 0.5 },
    freeze:   { id: 'freeze',   name: 'Freeze',    value: 0,    min: 0, max: 1, defaultValue: 0 },
    flatCut:  { id: 'flatCut',  name: 'Flat/Cut',  value: 0,    min: 0, max: 1, defaultValue: 0 },
    stereo:   { id: 'stereo',   name: 'Stereo',    value: 0.5,  min: 0, max: 1, defaultValue: 0.5 },
    density:  { id: 'density',  name: 'Density',   value: 0.33, min: 0, max: 1, defaultValue: 0.33 },
};

export const store = writable<ParameterState>(initialState);

export const setParameterValue = (id: keyof ParameterState, value: number) => {
    store.update(state => {
        if (state[id]) {
            return { ...state, [id]: { ...state[id], value } };
        }
        return state;
    });
};
