import { bridge } from '../bridge/bridge';
import { ParameterId } from '../types/parameters';
import { setWaveformValue } from '../state/store';

/**
 * Svelte Action for dropdown menu logic.
 * Usage: <div use:dropMenuControl={parameterId}>
 */
export function dropMenuControl(node: HTMLElement, parameterId: ParameterId) {
    // Prevent the container from jumping during layout changes
    node.style.flexShrink = '0';

    const onClick = (e: MouseEvent) => {
        e.preventDefault();

        // Simple toggle/cycle logic for the demo
        // In a real app, this would show a proper menu
        const currentValue = (window as any).currentWaveformValue || 0;
        const newValue = (currentValue + 1) % 2; // Toggles between 0 (Saw) and 1 (Noise)
        (window as any).currentWaveformValue = newValue;

        console.log(`Menu clicked for: ${parameterId}, switching to ${newValue}`);

        // 1. Update Store
        setWaveformValue(newValue);

        // 2. Send to Bridge
        bridge.sendParameterChange(parameterId, newValue);
    };

    node.addEventListener('click', onClick);

    return {
        destroy() {
            node.removeEventListener('click', onClick);
        }
    };
}