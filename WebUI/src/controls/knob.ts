import { bridge } from '../bridge/bridge';
import { ParameterId } from '../types/parameters';
import { setParameterValue } from '../state/store';

/**
 * Svelte Action for knob dragging logic.
 * Usage: <div use:knobControl={parameterId}>
 */
export function knobControl(node: HTMLElement, parameterId: ParameterId) {
    let isDragging = false;
    const minAngle = -135;
    const maxAngle = 135;
    const angleRange = maxAngle - minAngle;

    const updateFromEvent = (clientX: number, clientY: number) => {
        const rect = node.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;

        let angle = Math.atan2(clientY - centerY, clientX - centerX) * (180 / Math.PI);
        angle += 90;

        if (angle > 180) angle -= 360;

        const clampedAngle = Math.max(minAngle, Math.min(maxAngle, angle));
        const value = Math.round(((clampedAngle - minAngle) / angleRange) * 100);

        // 1. Update our Svelte Store (which updates the UI automatically)
        setParameterValue(parameterId, value);
        
        // 2. Send the value to the C++ Engine via the Bridge
        bridge.sendParameterChange(parameterId, value);
    };

    const onMouseDown = (e: MouseEvent) => {
        // Prevent selecting text or dragging images while moving the knob
        e.preventDefault();
        isDragging = true;
        updateFromEvent(e.clientX, e.clientY);
    };

    const onMouseMove = (e: MouseEvent) => {
        if (!isDragging) return;
        updateFromEvent(e.clientX, e.clientY);
    };

    const onMouseUp = () => {
        isDragging = false;
    };

    // Attach listeners
    node.addEventListener('mousedown', onMouseDown);
    // Move and Up are on window so the user can drag outside the knob area
    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup', onMouseUp);

    return {
        // Svelte calls this automatically when the component is destroyed
        destroy() {
            node.removeEventListener('mousedown', onMouseDown);
            window.removeEventListener('mousemove', onMouseMove);
            window.removeEventListener('mouseup', onMouseUp);
        }
    };
}