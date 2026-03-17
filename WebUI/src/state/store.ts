import { writable, derived } from 'svelte/store';
import type { ParameterId } from '../types/parameters';

// Per-parameter writable stores (normalized 0-1).
// Each store is independent — updating one param only notifies its own subscribers.
export const params = {
    // Input Filter
    loCutEnabled: writable(0),
    hiCutEnabled: writable(0),
    loCutFreq:    writable(0.35),
    hiCutFreq:    writable(0.42),

    // Early Reflections
    erEnabled: writable(0),
    erAmount:  writable(0.3),
    erRate:    writable(0.3),
    erShape:   writable(0.5),

    // FDN Core
    reverbMode:    writable(0),
    crossoverFreq: writable(0.5),
    diffusion:     writable(0.6),
    scale:         writable(0.5),
    decay:         writable(0.42),
    damping:       writable(0.5),
    feedback:      writable(0.75),

    // Chorus
    choEnable:    writable(0),
    chorusAmount: writable(0.2),
    chorusRate:   writable(0.27),

    // Output
    reflectGain: writable(0.8),
    diffuseGain: writable(0.8),
    dryWet:      writable(0.5),

    // Bottom Utility Row
    predelay: writable(0.08),
    smooth:   writable(0),
    size:     writable(0.5),
    freeze:   writable(0),
    flatCut:  writable(0),
    stereo:   writable(0.5),
    density:  writable(0.33),
};

export function setParameterValue(id: ParameterId, value: number) {
    params[id].set(value);
}

// Selector indices — only recompute when their own param changes
export const modeSelected    = derived(params.reverbMode, $v => Math.round($v));
export const smoothSelected  = derived(params.smooth,     $v => Math.round($v * 3));
export const densitySelected = derived(params.density,    $v => Math.round($v * 3));
export const flatCutSelected = derived(params.flatCut,    $v => Math.round($v));

// Display values — only recompute when their own param changes
export const loCutHz     = derived(params.loCutFreq,     $v => Math.round(20 + $v * 480));
export const hiCutHz     = derived(params.hiCutFreq,     $v => Math.round(1000 + $v * 19000));
export const crossoverHz = derived(params.crossoverFreq, $v => (200 + $v * 7800).toFixed(0));
export const diffusionPct = derived(params.diffusion,    $v => ($v * 100).toFixed(0));

// SVG node positions — only recompute when their own param changes
export const svgNode1X = derived(params.crossoverFreq, $v => 80  + $v * 40);
export const svgNode1Y = derived(params.damping,       $v => 15  + $v * 40);
export const svgNode2X = derived(params.feedback,      $v => 140 + $v * 30);
export const svgNode2Y = derived(params.diffusion,     $v => 40  + $v * 20);
