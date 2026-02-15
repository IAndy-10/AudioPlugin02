import { createKnob } from './components/knobComponent';

type ComponentType = 'knob'; // | 'slider' | 'meter';

interface ComponentFactory {
    create: (container: HTMLElement, parameterId: string) => Promise<void>;
}

const factories: Record<ComponentType, ComponentFactory> = {
    knob: { create: createKnob },
    // slider: { create: createSlider },
    // meter: { create: createMeter },
};

export async function initializeApp() {
    const app = document.getElementById('app');
    
    if (!app) {
        throw new Error('App container (#app) not found in DOM');
    }

    const components = app.querySelectorAll('[data-component]');

    for (const el of components) {
        const type = el.getAttribute('data-component') as ComponentType;
        const parameterId = el.getAttribute('data-param');

        if (!type || !parameterId) {
            console.warn('Invalid component config:', el);
            continue;
        }

        try {
            const factory = factories[type];
            if (!factory) {
                throw new Error(`Unknown component type: ${type}`);
            }

            await factory.create(el as HTMLElement, parameterId);
            console.log(`✓ Mounted ${type}: ${parameterId}`);
        } catch (error) {
            console.error(`✗ Failed to mount ${type}/${parameterId}:`, error);
        }
    }

    console.log('✓ Plugin UI initialized');
}