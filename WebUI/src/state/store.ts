import { ParameterState } from '../types/parameters';


class Store {
    private state: ParameterState = {
        gain: {
            id: 'gain',
            name: 'Gain',
            value: 50,
            min: 0,
            max: 100,
            defaultValue: 50
        }
    };

    private listeners: Set<() => void> = new Set();

    getState() {
        return this.state;
    }

    setParameterValue(id: keyof ParameterState, value: number) {
        if (this.state[id]) {
            this.state[id].value = value;
            this.notify();
        }
    }

    subscribe(listener: () => void) {
        this.listeners.add(listener);
        return () => this.listeners.delete(listener);
    }

    private notify() {
        this.listeners.forEach(l => l());
    }
}

export const store = new Store();
