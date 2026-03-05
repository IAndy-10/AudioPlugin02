export type ParameterId =
    // Input Filter
    | 'loCutEnabled' | 'hiCutEnabled' | 'loCutFreq' | 'hiCutFreq'
    // Early Reflections
    | 'erEnabled' | 'erAmount' | 'erRate' | 'erShape'
    // FDN Core
    | 'reverbMode' | 'crossoverFreq' | 'diffusion' | 'scale'
    | 'decay' | 'damping' | 'feedback'
    // Chorus
    | 'chorusAmount' | 'chorusRate'
    // Output
    | 'reflectGain' | 'diffuseGain' | 'dryWet'
    // Bottom Utility Row
    | 'predelay' | 'smooth' | 'size' | 'freeze' | 'flatCut' | 'stereo' | 'density';

export interface PluginParameter {
    id: ParameterId;
    name: string;
    value: number; // normalized 0-1 (matches JUCE param->getValue())
    min: number;
    max: number;
    defaultValue: number;
}

export type ParameterState = Record<ParameterId, PluginParameter>;
