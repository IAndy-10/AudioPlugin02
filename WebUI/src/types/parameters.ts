export type ParameterId = 'gain' | 'playing';

export interface PluginParameter {
    id: ParameterId;
    name: string;
    value: number; // 0-100
    min: number;
    max: number;
    defaultValue: number;
}

export type ParameterState = Record<ParameterId, PluginParameter>;
