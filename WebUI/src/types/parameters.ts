export type ParameterId =
    // Input Filter (5)
    | 'loCutEnabled' | 'hiCutEnabled' | 'loCutFreq' | 'hiCutFreq' | 'hiCutQ'
    // Early Reflections (4)
    | 'erEnabled' | 'erAmount' | 'erRate' | 'erShape'
    // FDN Core (8)
    | 'reverbMode' | 'crossoverFreq' | 'diffusion' | 'scale'
    | 'decay' | 'damping' | 'feedback' | 'highFilterType'
    // Chorus (2)
    | 'chorusAmount' | 'chorusRate'
    // Output (3)
    | 'reflectGain' | 'diffuseGain' | 'dryWet'
    // Bottom Utility Row (8)
    | 'predelay' | 'smooth' | 'size' | 'freeze'
    | 'flatEnabled' | 'cutEnabled' | 'stereo' | 'density';
// Total: 30 unique IDs, matching ParameterIDs.h exactly.
// Note: the Step 02 spec formula (24 − 1 + 3 = 26) was a miscalculation;
// the actual pre-Step-01 count was 28, yielding 28 − 1 + 3 = 30.

export interface PluginParameter {
    id: ParameterId;
    name: string;
    value: number; // normalized 0-1 (matches JUCE param->getValue())
    min: number;
    max: number;
    defaultValue: number;
}

export type ParameterState = Record<ParameterId, PluginParameter>;
