/**
How does this file map the name of the parameters?


 */

export type ParameterId = 'gain' | 'waveform';

export interface PluginParameter {
    id: ParameterId;
    name: string;
    value: number; // 0-100
    min: number;
    max: number;
    defaultValue: number;
}

export interface WaveformParameter extends PluginParameter {
    id: 'waveform';
    choices: string[]; // e.g., ["Saw", "Noise"]
}

export type ParameterState = Record<ParameterId, PluginParameter>;
