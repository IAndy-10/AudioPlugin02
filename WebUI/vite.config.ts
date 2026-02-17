import { defineConfig } from 'vite';
import { viteSingleFile } from 'vite-plugin-singlefile';
import { svelte, vitePreprocess } from '@sveltejs/vite-plugin-svelte';

export default defineConfig({
    plugins: [
        viteSingleFile(),
        svelte({
            preprocess: vitePreprocess(),
        }),
    ],
    build: {
        outDir: 'dist',
        emptyOutDir: true,
    },
});



