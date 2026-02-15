import { bridge } from '../bridge/bridge';
import { ParameterId } from '../types/parameters';
import { store } from '../state/store';

export class KnobControl {
    private isDragging = false;
    private minAngle = -135;
    private maxAngle = 135;

    constructor(
        private element: HTMLElement,
        private parameterId: ParameterId
    ) {
        this.attachEventListeners();
    }

    private attachEventListeners() {
        this.element.addEventListener('mousedown', this.onMouseDown.bind(this));
        document.addEventListener('mousemove', this.onMouseMove.bind(this));
        document.addEventListener('mouseup', this.onMouseUp.bind(this));

    }

    private onMouseDown(e: MouseEvent) {
        e.preventDefault();
        this.isDragging = true;
        this.updateFromEvent(e.clientX, e.clientY);
    }


    private onMouseMove(e: MouseEvent) {
        if (!this.isDragging) return;
        this.updateFromEvent(e.clientX, e.clientY);
    }

    
    private onMouseUp() {
        this.isDragging = false;
    }

    private updateFromEvent(clientX: number, clientY: number) {
        const rect = this.element.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;

        let angle = Math.atan2(clientY - centerY, clientX - centerX) * (180 / Math.PI);
        angle += 90;

        if (angle > 180) angle -= 360;

        const angleRange = this.maxAngle - this.minAngle;
        const clampedAngle = Math.max(this.minAngle, Math.min(this.maxAngle, angle));
        const value = Math.round(((clampedAngle - this.minAngle) / angleRange) * 100);

        store.setParameterValue(this.parameterId, value);
        bridge.sendParameterChange(this.parameterId, value);
    }
}
