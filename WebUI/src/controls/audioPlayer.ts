import { bridge } from '../bridge/bridge';
import type { ParameterId } from '../types/parameters';
import { store, setParameterValue } from '../state/store';

/**
 * Svelte Action for play/stop toggle logic.
 * Usage: <button use:audioPlayer={parameterId}>
 */
export function audioPlayer(node: HTMLElement, parameterId: ParameterId) {
    const toggle = () => {
        // Read current value directly from the store (synchronous snapshot)
        let currentValue = 0;
        const unsubscribe = store.subscribe(s => {
            currentValue = s[parameterId]?.value ?? 0;
        });
        unsubscribe();

        const newValue = currentValue > 0.5 ? 0 : 1;

        // 1. Optimistic store update (UI reacts immediately)
        setParameterValue(parameterId, newValue);
        // 2. Notify backend
        bridge.sendParameterChange(parameterId, newValue);
    };

    node.addEventListener('click', toggle);

    return {
        destroy() {
            node.removeEventListener('click', toggle);
        }
    };
}