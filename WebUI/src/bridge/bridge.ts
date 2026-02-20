import { setParameterValue as updateStore } from '../state/store'; // Import our new helper
import { ParameterId } from '../types/parameters';
import {
    SetParameterValueMessage,
    isSetParameterValueMessage,
    isBackendMessage,
    describeMessage,
    BackendMessage
} from './types';

export class ParameterBridge {
    private debugMode = true;

    constructor() {
        this.initializeGlobalCallbacks();
        this.logBridgeReady();
    }

    private initializeGlobalCallbacks() {
        // C++ → JS: Main entry point
        (window as any).setParameterValue = (id: ParameterId, value: number) => {
            this.handleParameterValueFromBackend(id, value);
        };

        // C++ → JS: Generic message handler
        (window as any).onBackendMessage = (message: BackendMessage) => {
            this.handleBackendMessage(message);
        };

        this.log('Global callbacks initialized');
    }

    private handleParameterValueFromBackend(id: ParameterId, value: number) {
        const message: SetParameterValueMessage = {
            type: 'setParameterValue',
            parameterId: id,
            value
        };

        if (!isSetParameterValueMessage(message)) {
            this.error(`Invalid parameter message from C++:`, message);
            return;
        }

        this.log(`Received from C++: ${describeMessage(message)}`);

        /**
         * SVELTE INTEGRATION:
         * We call updateStore, which updates the Svelte writable store.
         * Any component using $store[id] will now re-render instantly.
         */
        updateStore(id, value);
    }

    private handleBackendMessage(message: any) {
        if (!isBackendMessage(message)) {
            this.error('Invalid backend message:', message);
            return;
        }

        switch (message.type) {
            case 'setParameterValue':
                this.handleParameterValueFromBackend(message.parameterId, message.value);
                break;
            default:
                this.log(`Received unknown message type: ${message.type}`);
        }

    }

    public sendParameterChange(id: ParameterId, value: number) {
        let finalValue = value;

        if (id === 'gain') {
            finalValue = this.normalizedToDB(value);
            this.log(`Sending to C++: ${id} = ${value}% (${finalValue.toFixed(2)} dB)`);
        } else {
            this.log(`Sending to C++: ${id} = ${value}`);
        }

        /**
         * JS → C++: Communication via JUCE URL scheme
         */
        window.location.href = `juce://setparameter?name=${id}&value=${finalValue}`;
    }

    private normalizedToDB(value: number): number {
        return (value / 100.0) * 60.0 - 60.0;
    }

    private log(message: string, data?: any) {
        if (!this.debugMode) return;
        console.log(`[Bridge] ${message}`, data || '');
    }

    private error(message: string, data?: any) {
        console.error(`[Bridge ERROR] ${message}`, data || '');
    }

    private logBridgeReady() {
        console.log('%c🌉 ParameterBridge initialized', 'color: #4a9eff; font-weight: bold;');
    }
}

export const bridge = new ParameterBridge();