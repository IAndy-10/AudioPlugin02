import { store } from '../state/store';
import { ParameterId } from '../types/parameters';
import {
    SetParameterValueMessage,
    isSetParameterValueMessage,
    isBackendMessage,
    describeMessage,
    BackendMessage
} from './types';

/**
 * ParameterBridge
 * 
 * Manages bidirectional communication between the TypeScript frontend
 * and the C++/JUCE backend.
 * 
 * Two main responsibilities:
 * 1. Expose global functions that C++ can call (C++ → JS)
 * 2. Send parameter changes back to C++ via URL scheme (JS → C++)
 */
export class ParameterBridge {
    private debugMode = true; // Set to false in production

    constructor() {
        this.initializeGlobalCallbacks();
        this.logBridgeReady();
    }

    /**
     * Initialize global callbacks that C++ can call
     * These are exposed on the window object so JUCE can invoke them
     */
    private initializeGlobalCallbacks() {
        // Main generic parameter setter (C++ → JS)
        (window as any).setParameterValue = (id: ParameterId, value: number) => {
            this.handleParameterValueFromBackend(id, value);
        };

        // Convenience method for specific parameters (C++ → JS)
        // Useful if C++ code references parameters by UI element names
        (window as any).gainKnob = {
            setValue: (value: number) => {
                this.handleParameterValueFromBackend('gain', value);
            }
        };

        // Generic message handler (for future extensibility)
        (window as any).onBackendMessage = (message: BackendMessage) => {
            this.handleBackendMessage(message);
        };

        this.log('Global callbacks initialized');
    }

    /**
     * Handle parameter value updates from C++
     * Validates the message and updates the store
     */
    private handleParameterValueFromBackend(id: ParameterId, value: number) {
        // Create a message object for validation
        const message: SetParameterValueMessage = {
            type: 'setParameterValue',
            parameterId: id,
            value
        };

        // Validate the message
        if (!isSetParameterValueMessage(message)) {
            this.error(`Invalid parameter message from C++:`, message);
            return;
        }

        this.log(`Received from C++: ${describeMessage(message)}`);

        // Update the store (which notifies all listeners)
        store.setParameterValue(id, value);
    }

    /**
     * Handle any backend message (future-proofing)
     * Validates and processes different message types
     */
    private handleBackendMessage(message: any) {
        if (!isBackendMessage(message)) {
            this.error('Invalid backend message:', message);
            return;
        }

        this.log(`Backend message: ${describeMessage(message)}`);

        // Handle different message types
        switch (message.type) {
            case 'setParameterValue':
                this.handleParameterValueFromBackend(message.parameterId, message.value);
                break;

            case 'audioState':
                this.log(`Audio state changed: playing=${message.isPlaying}, bypassed=${message.isBypassed}`);
                // TODO: Handle audio state changes (e.g., update UI indicators)
                break;

            case 'initialize':
                this.log(`Backend initialization received with ${Object.keys(message.parameters).length} parameters`);
                // TODO: Handle initial parameter values from C++
                break;

            case 'meterData':
                // This comes frequently, so don't log every time
                // this.log(`Meter data: output=${message.outputLevel}dB`);
                // TODO: Update meter displays
                break;

            case 'error':
                this.error(`Backend error [${message.severity}]: ${message.message}`);
                break;

            default:
                const exhaustiveCheck: never = message;
                this.error(`Unknown message type:`, exhaustiveCheck);
        }
    }

    /**
     * Send a parameter change to C++
     * 
     * Called when the user changes a parameter in the UI.
     * Converts the normalized value (0-100) to dB as expected by C++.
     * 
     * @param id - The parameter ID
     * @param value - Normalized value (0-100)
     */
    public sendParameterChange(id: ParameterId, value: number) {
        // Validate input
        if (value < 0 || value > 100) {
            this.error(`Invalid parameter value: ${value} (must be 0-100)`);
            return;
        }

        // Convert normalized value (0-100) to dB (-60 to 0)
        // This matches the C++ backend's expectations
        const gainDb = this.normalizedToDB(value);

        this.log(`Sending to C++: ${id} = ${value}% (${gainDb.toFixed(2)} dB)`);

        // Send via JUCE URL scheme
        const bridgeURL = `juce://setparameter?name=${id}&value=${gainDb}`;
        window.location.href = bridgeURL;
    }

    /**
     * Convert a normalized value (0-100) to decibels (-60 to 0)
     * 
     * Formula:
     * - 0% = -60 dB (minimum)
     * - 50% = 0 dB (middle)
     * - 100% = 0 dB (maximum)
     * 
     * Note: The C++ code expects dB because it maps it back to 0-1 range internally.
     */
    private normalizedToDB(value: number): number {
        return (value / 100.0) * 60.0 - 60.0;
    }

    /**
     * Convert decibels back to normalized value (for reference)
     * Inverse of normalizedToDB
     */
    private dbToNormalized(db: number): number {
        return ((db + 60.0) / 60.0) * 100.0;
    }

    /**
     * Logging utility - shows where messages come from
     */
    private log(message: string, data?: any) {
        if (!this.debugMode) return;
        console.log(`[Bridge] ${message}`, data || '');
    }

    /**
     * Error logging utility
     */
    private error(message: string, data?: any) {
        console.error(`[Bridge ERROR] ${message}`, data || '');
    }

    /**
     * Log that the bridge is ready
     */
    private logBridgeReady() {
        console.log('%c🌉 ParameterBridge initialized', 'color: #4a9eff; font-weight: bold;');
        console.log('%cGlobal functions available:', 'color: #4a9eff;');
        console.log('  - window.setParameterValue(id, value)');
        console.log('  - window.gainKnob.setValue(value)');
        console.log('  - window.onBackendMessage(message)');
    }

    /**
     * Disable debug logging (useful for production)
     */
    public setDebugMode(enabled: boolean) {
        this.debugMode = enabled;
    }
}

// Create singleton instance
export const bridge = new ParameterBridge();