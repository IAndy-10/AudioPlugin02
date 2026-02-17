import './index.css';
import App from './App.svelte';

/**
 * initializeApp now simply mounts the root Svelte component.
 * Svelte will handle the creation and lifecycle of all Knobs, 
 * Sliders, and Meters defined inside App.svelte.
 */
export function initializeApp() {
    const target = document.getElementById('app');
    
    if (!target) {
        throw new Error('App container (#app) not found in DOM');
    }

    // This creates the entire UI hierarchy
    new App({
        target: target,
    });

    console.log('✓ Svelte UI initialized');
}

// Auto-boot if needed, or call this from your index.html
document.addEventListener('DOMContentLoaded', initializeApp);