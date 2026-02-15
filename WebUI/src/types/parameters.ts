/**
How does this file map the name of the parameters?


 */

export type ParameterId = 'gain';

export interface PluginParameter {
    id: ParameterId;
    name: string;
    value: number; // 0-100
    min: number;
    max: number;
    defaultValue: number;
}

export type ParameterState = Record<ParameterId, PluginParameter>;
