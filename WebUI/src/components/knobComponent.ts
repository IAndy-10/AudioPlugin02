import { store } from '../state/store';
import { ParameterId } from '../types/parameters';
import { KnobControl } from '../controls/knob';
import knobHTML from './knob.html?raw';


export async function createKnob(container: HTMLElement, parameterId: ParameterId) {
    const state = store.getState()[parameterId];

    // 1. Load the HTML template
    container.innerHTML = knobHTML;


    // 2. Query elements by data attributes
    const knobWrapper = container.querySelector('[data-knob]') as HTMLElement;
    const progressPath = container.querySelector('[data-progress]') as SVGPathElement;
    const indicator = container.querySelector('[data-indicator]') as SVGLineElement;
    const valueDisplay = container.querySelector('[data-value]') as HTMLElement;
    const nameDisplay = container.querySelector('[data-name]') as HTMLElement;

    // 3. Initialize with current values
    valueDisplay.textContent = `${state.value}%`;
    nameDisplay.textContent = state.name;

    // 4. Attach drag interaction
    new KnobControl(knobWrapper, parameterId);

    // 5. Define reactive updates
    const updateUI = () => {
        const value = store.getState()[parameterId].value;
        const minAngle = -135;
        const maxAngle = 135;
        const angleRange = maxAngle - minAngle;
        const currentAngle = minAngle + (value / 100) * angleRange;

        // Update SVG Path
        progressPath.setAttribute('d', describeArc(100, 100, 70, minAngle, currentAngle));

        // Update Indicator
        indicator.style.transform = `rotate(${currentAngle}deg)`;
        indicator.style.transformOrigin = '100px 100px';

        // Update Value
        valueDisplay.textContent = `${value}%`;
    };

    // 6. Subscribe to store changes
    store.subscribe(updateUI);
    
    // 7. Initial render
    updateUI();
}

function polarToCartesian(centerX: number, centerY: number, radius: number, angleInDegrees: number) {
    const angleInRadians = (angleInDegrees - 90) * Math.PI / 180.0;
    return {
        x: centerX + (radius * Math.cos(angleInRadians)),
        y: centerY + (radius * Math.sin(angleInRadians))
    };
}

function describeArc(x: number, y: number, radius: number, startAngle: number, endAngle: number) {
    const start = polarToCartesian(x, y, radius, endAngle);
    const end = polarToCartesian(x, y, radius, startAngle);
    const largeArcFlag = endAngle - startAngle <= 180 ? "0" : "1";

    return [
        "M", start.x, start.y,
        "A", radius, radius, 0, largeArcFlag, 0, end.x, end.y
    ].join(" ");
}