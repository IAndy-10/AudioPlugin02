import './index.css';
import App from './App.svelte';
import './bridge/bridge';

const target = document.getElementById('app');

if (!target) {
    console.error('Fatal error: App container (#app) not found in DOM');
} else {
    try {
        new App({
            target: target,
        });
        console.log('✓ Svelte Plugin UI initialized');
    } catch (error) {
        console.error('Fatal error during Svelte initialization:', error);
        target.innerHTML = `
            <div class="flex items-center justify-center h-full text-red-500 font-mono text-sm">
                [FATAL ERROR]: UI_MOUNT_FAILED
            </div>
        `;
    }
}