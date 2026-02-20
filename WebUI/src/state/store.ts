import { writable } from 'svelte/store';
import type { ParameterState } from '../types/parameters';

const initialValue: ParameterState = {
    gain: {
        id: 'gain',
        name: 'Gain',
        value: 50,
        min: 0,
        max: 100,
        defaultValue: 50
    },
    playing: {
        id: 'playing',
        name: 'Playing',
        value: 0, // 0 = Off, 1 = On
        min: 0,
        max: 1,
        defaultValue: 0
    }
};

export const store = writable<ParameterState>(initialValue);

/**
 * Updates the store. Called by the Bridge (C++ -> JS) 
 * or the Knob Action (User -> JS).
 */
export const setParameterValue = (id: keyof ParameterState, value: number) => {
    store.update(state => {
        if (state[id]) {
            return {
                ...state,
                [id]: {
                    ...state[id],
                    value
                }
            };
        }
        return state;
    });
};

export const setPlayingValue = (value: number) => {
    setParameterValue('playing', value);
};