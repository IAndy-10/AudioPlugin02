import './index.css';
import { initializeApp } from './app';
import './bridge/bridge';

document.addEventListener('DOMContentLoaded', async () => {
    try {
        await initializeApp();
    } catch (error) {
        console.error('Fatal error during initialization:', error);
        const app = document.getElementById('app');
        if (app) {
            app.innerHTML = '<div class="text-red-500 text-center">Failed to load plugin UI</div>';
        }
    }
});