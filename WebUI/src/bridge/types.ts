/**
 * Bridge Communication Types
 * 
 * Defines all messages that can flow between the TypeScript frontend
 * and the C++/JUCE backend. This ensures type safety across the bridge.
 */

import { ParameterId } from '../types/parameters';

// ============================================
// Messages FROM C++ TO Frontend (C++ calls JS)
// ============================================

/**
 * C++ tells frontend to update a parameter value
 * Called when:
 * - User changes parameter in DAW/host
 * - Automation is applied
 * - Preset is loaded
 */
export interface SetParameterValueMessage {
    type: 'setParameterValue';
    parameterId: ParameterId;
    value: number; // 0-100 (normalized)
}

/**
 * C++ tells frontend about overall audio state
 * Called when:
 * - Plugin starts/stops processing
 * - Plugin is bypassed/enabled
 */
export interface AudioStateMessage {
    type: 'audioState';
    isPlaying: boolean;
    isBypassed: boolean;
}

/**
 * C++ sends initial state on plugin load
 * Called once when the plugin UI first loads
 */
export interface InitializationMessage {
    type: 'initialize';
    parameters: Record<ParameterId, ParameterData>;
}

/**
 * Structure of parameter data from C++
 */
export interface ParameterData {
    id: ParameterId;
    name: string;
    min: number;
    max: number;
    defaultValue: number;
    currentValue: number;
}

/**
 * C++ sends meter/analysis data to frontend
 * Called regularly for UI updates (audio level, spectrum, etc.)
 */
export interface MeterDataMessage {
    type: 'meterData';
    outputLevel: number; // -60 to 0 dB
    peakLevel: number;   // -60 to 0 dB
}

/**
 * C++ sends an error or warning to the frontend
 * Called when something goes wrong in the backend
 */
export interface ErrorMessage {
    type: 'error';
    severity: 'error' | 'warning' | 'info';
    message: string;
}

// ============================================
// Messages FROM Frontend TO C++ (JS calls C++)
// ============================================

/**
 * Frontend tells C++ that a parameter changed
 * Called when:
 * - User moves a knob/slider
 * - Preset is loaded
 */
export interface ParameterChangeRequest {
    type: 'parameterChange';
    parameterId: ParameterId;
    value: number; // 0-100 (normalized)
}

/**
 * Frontend requests to save current state as preset
 * Called when user saves a preset
 */
export interface PresetSaveRequest {
    type: 'presetSave';
    presetName: string;
}

/**
 * Frontend requests to load a preset
 * Called when user loads a saved preset
 */
export interface PresetLoadRequest {
    type: 'presetLoad';
    presetName: string;
}

/**
 * Frontend requests bypass state change
 * Called when user clicks bypass button
 */
export interface BypassRequest {
    type: 'bypass';
    bypassed: boolean;
}

// ============================================
// Message Unions
// ============================================

/**
 * All messages that C++ can send to Frontend
 */
export type BackendMessage =
    | SetParameterValueMessage
    | AudioStateMessage
    | InitializationMessage
    | MeterDataMessage
    | ErrorMessage;

/**
 * All messages that Frontend can send to C++
 */
export type FrontendRequest =
    | ParameterChangeRequest
    | PresetSaveRequest
    | PresetLoadRequest
    | BypassRequest;

/**
 * All possible messages on the bridge
 */
export type BridgeMessage = BackendMessage | FrontendRequest;

// ============================================
// Type Guards (runtime validation)
// ============================================

/**
 * Check if an object is a valid SetParameterValueMessage
 */
export function isSetParameterValueMessage(obj: any): obj is SetParameterValueMessage {
    return (
        obj &&
        obj.type === 'setParameterValue' &&
        typeof obj.parameterId === 'string' &&
        typeof obj.value === 'number' &&
        obj.value >= 0 &&
        obj.value <= 100
    );
}

/**
 * Check if an object is a valid AudioStateMessage
 */
export function isAudioStateMessage(obj: any): obj is AudioStateMessage {
    return (
        obj &&
        obj.type === 'audioState' &&
        typeof obj.isPlaying === 'boolean' &&
        typeof obj.isBypassed === 'boolean'
    );
}

/**
 * Check if an object is a valid ParameterChangeRequest
 */
export function isParameterChangeRequest(obj: any): obj is ParameterChangeRequest {
    return (
        obj &&
        obj.type === 'parameterChange' &&
        typeof obj.parameterId === 'string' &&
        typeof obj.value === 'number' &&
        obj.value >= 0 &&
        obj.value <= 100
    );
}

/**
 * Check if an object is a valid BackendMessage
 */
export function isBackendMessage(obj: any): obj is BackendMessage {
    if (!obj || typeof obj.type !== 'string') {
        return false;
    }

    switch (obj.type) {
        case 'setParameterValue':
            return isSetParameterValueMessage(obj);
        case 'audioState':
            return isAudioStateMessage(obj);
        case 'initialize':
            return obj.parameters && typeof obj.parameters === 'object';
        case 'meterData':
            return typeof obj.outputLevel === 'number' && typeof obj.peakLevel === 'number';
        case 'error':
            return typeof obj.message === 'string';
        default:
            return false;
    }
}

/**
 * Check if an object is a valid FrontendRequest
 */
export function isFrontendRequest(obj: any): obj is FrontendRequest {
    if (!obj || typeof obj.type !== 'string') {
        return false;
    }

    switch (obj.type) {
        case 'parameterChange':
            return isParameterChangeRequest(obj);
        case 'presetSave':
            return typeof obj.presetName === 'string';
        case 'presetLoad':
            return typeof obj.presetName === 'string';
        case 'bypass':
            return typeof obj.bypassed === 'boolean';
        default:
            return false;
    }
}

/**
 * Check if an object is a valid BridgeMessage
 */
export function isBridgeMessage(obj: any): obj is BridgeMessage {
    return isBackendMessage(obj) || isFrontendRequest(obj);
}

// ============================================
// Logging/Debugging Utilities
// ============================================

/**
 * Get a human-readable description of a message
 */
export function describeMessage(message: BridgeMessage): string {
    switch (message.type) {
        case 'setParameterValue':
            return `C++ → Parameter "${message.parameterId}" set to ${message.value}`;
        case 'audioState':
            return `C++ → Audio state: playing=${message.isPlaying}, bypassed=${message.isBypassed}`;
        case 'initialize':
            return `C++ → Initialize with ${Object.keys(message.parameters).length} parameters`;
        case 'meterData':
            return `C++ → Meter data: output=${message.outputLevel}dB, peak=${message.peakLevel}dB`;
        case 'error':
            return `C++ → Error [${message.severity}]: ${message.message}`;
        case 'parameterChange':
            return `JS → Parameter "${message.parameterId}" changed to ${message.value}`;
        case 'presetSave':
            return `JS → Save preset "${message.presetName}"`;
        case 'presetLoad':
            return `JS → Load preset "${message.presetName}"`;
        case 'bypass':
            return `JS → Bypass: ${message.bypassed}`;
        default:
            const exhaustiveCheck: never = message;
            return `Unknown message: ${exhaustiveCheck}`;
    }
}