<script lang="ts">
    import {
        params, setParameterValue,
        modeSelected, smoothSelected, densitySelected, flatCutSelected,
        crossoverHz, diffusionPct
    } from './state/store';
    import { bridge } from './bridge/bridge';
    import type { ParameterId } from './types/parameters';

    import NeuKnob     from './components/NeuKnob.svelte';
    import NeuButton   from './components/NeuButton.svelte';
    import NeuSelector from './components/NeuSelector.svelte';
    import NeuNumber   from './components/NeuNumber.svelte';
    import FilterGraph from './components/FilterGraph.svelte';
    import NeuDiffusionNetworkGraph from './components/NeuDiffusionNetworkGraph.svelte';

    let filterFreq = 1000;
    let filterQ    = 1.0;
    let filterLoCut = false;
    let filterHiCut = false;

    // Destructure individual stores so Svelte's $X auto-subscribe syntax works
    const {
        erEnabled, erAmount, erRate, erShape,
        crossoverFreq, diffusion, scale, decay, damping, feedback,
        choEnable, chorusAmount, chorusRate,
        reflectGain, diffuseGain, dryWet,
        predelay, size, freeze, stereo,
    } = params;

    function send(id: ParameterId, value: number) {
        setParameterValue(id, value);
        bridge.sendParameterChange(id, value);
    }

    function sendBool(id: ParameterId, active: boolean) {
        send(id, active ? 1 : 0);
    }

    function sendSelector(id: ParameterId, selected: number, numOptions: number) {
        send(id, selected / (numOptions - 1));
    }

</script>

<main class="plugin-root">

    <!-- ========== TOP BAR ========== -->
    <div class="top-bar">
        <div class="plugin-title">
            <span class="title-main">Audio Plugin 02</span>
        </div>
    </div>

    <!-- ========== MAIN PANELS ========== -->
    <div class="panels-row">

        <!-- ===== PANEL 1: INPUT FILTER ===== -->
        <section class="panel panel-input-filter">
            <div class="panel-title">Input Filter</div>

            <div class="panel-body" style="gap:14px; align-items:center;">
                <!-- Filter graph -->
                <FilterGraph
                    freq={filterFreq}
                    q={filterQ}
                    loCut={filterLoCut}
                    hiCut={filterHiCut}
                    on:change={e => {
                        filterFreq   = e.detail.freq;
                        filterQ      = e.detail.q;
                        filterLoCut  = e.detail.loCut;
                        filterHiCut  = e.detail.hiCut;
                    }}
                />
            </div>
        </section>

        <!-- ===== PANEL 2: EARLY REFLECTIONS ===== -->
        <section class="panel panel-early-ref">
            <div class="panel-title">Early Reflections</div>

            <div class="panel-body" style="gap:14px; align-items:center;">
                <NeuButton
                    label="Spin"
                    active={$erEnabled > 0.5}
                    icon="↺"
                    on:change={e => sendBool('erEnabled', e.detail.active)}
                />

                <div class="row">
                    <NeuNumber label="Amount" value={+($erAmount * 53 + 2).toFixed(1)} min={2} max={55} step={0.5} unit="%" decimals={1}
                        on:change={e => send('erAmount', (e.detail.value - 2) / 53)} />
                    <NeuNumber label="Rate" value={+($erRate * 1.23 + 0.07).toFixed(2)} min={0.07} max={1.3} step={0.01} unit=" Hz" decimals={2}
                        on:change={e => send('erRate', (e.detail.value - 0.07) / 1.23)} />
                </div>

                <NeuKnob label="Shape" value={$erShape} min={0} max={100} unit="%"
                    on:change={e => send('erShape', e.detail.value)} />
            </div>
        </section>

        <!-- ===== PANEL 3: DIFFUSION NETWORK (largest) ===== -->
        <section class="panel panel-diffusion">
            <div class="panel-title">Diffusion Network</div>
            <div class="panel-body" style="gap:10px; align-items:center;">
                <NeuDiffusionNetworkGraph
                    crossoverFreq={$crossoverFreq}
                    diffusion={$diffusion}
                    damping={$damping}
                    feedback={$feedback}
                    on:change={e => {
                        send('crossoverFreq', e.detail.crossoverFreq);
                        send('diffusion',     e.detail.diffusion);
                        send('damping',       e.detail.damping);
                        send('feedback',      e.detail.feedback);
                    }}
                />
            </div>
        </section>

        <!-- ===== PANEL 4: CHORUS ===== -->
        <section class="panel panel-chorus">
            <div class="panel-title">Chorus</div>

            <div class="panel-body" style="gap:14px; align-items:center;">

                <NeuButton
                label="Chorus"
                active={$choEnable > 0.5}
                icon=":)"
                on:change={e => sendBool('choEnable', e.detail.active)}
            />
                <NeuNumber label="Amount" value={+($chorusAmount * 3.99 + 0.01).toFixed(2)} min={0.01} max={4.00} step={0.01} decimals={2}
                    on:change={e => send('chorusAmount', (e.detail.value - 0.01) / 3.99)} />
                <NeuNumber label="Rate" value={+($chorusRate * 7.99 + 0.01).toFixed(2)} min={0.01} max={8.00} step={0.01} unit=" Hz" decimals={2}
                    on:change={e => send('chorusRate', (e.detail.value - 0.01) / 7.99)} />
            </div>
        </section>

        <!-- ===== PANEL 5: OUTPUT / REFLECT STRIP ===== -->
        <section class="panel panel-output">
            <div class="panel-title-vertical">Reflect</div>

            <div class="panel-body" style="gap:14px; align-items:center;">
                <NeuKnob label="Reflect" value={$reflectGain} min={-24} max={6} unit=" dB"
                    on:change={e => send('reflectGain', e.detail.value)} />
                <NeuKnob label="Diffuse" value={$diffuseGain} min={-24} max={6} unit=" dB"
                    on:change={e => send('diffuseGain', e.detail.value)} />
                <NeuKnob label="Dry/Wet" value={$dryWet} min={0} max={100} unit="%"
                    on:change={e => send('dryWet', e.detail.value)} />
            </div>
        </section>

    </div>

    <!-- ========== BOTTOM UTILITY ROW ========== -->
    <div class="bottom-row">

        <NeuNumber label="Predelay" value={$predelay * 500} min={0} max={500} step={1} unit=" ms" decimals={0}
            on:change={e => send('predelay', e.detail.value / 500)} />

        <NeuSelector
            label="Smooth"
            options={['Off', 'Low', 'Med', 'High']}
            selected={$smoothSelected}
            on:change={e => sendSelector('smooth', e.detail.selected, 4)}
        />

        <NeuKnob label="Size" value={$size} min={0} max={100} unit="%"
            on:change={e => send('size', e.detail.value)} />

        <NeuKnob label="Decay" value={$decay} min={100} max={10000} unit=" ms"
            on:change={e => send('decay', e.detail.value)} />

        <NeuButton
            label="Freeze"
            active={$freeze > 0.5}
            icon="❄"
            on:change={e => sendBool('freeze', e.detail.active)}
        />

        <NeuSelector
            options={['Flat', 'Cut']}
            selected={$flatCutSelected}
            on:change={e => sendSelector('flatCut', e.detail.selected, 2)}
        />

        <NeuKnob label="Stereo" value={$stereo} min={0} max={200} unit="%"
            on:change={e => send('stereo', e.detail.value)} />

        <NeuSelector
            label="Density"
            options={['Low', 'Med', 'High', 'Ultra']}
            selected={$densitySelected}
            on:change={e => sendSelector('density', e.detail.selected, 4)}
        />

    </div>

</main>

<style>
    @import url('https://fonts.googleapis.com/css2?family=DM+Sans:ital,wght@0,200;0,300;0,400&family=DM+Serif+Display&display=swap');

    :global(body) {
        margin: 0;
        padding: 0;
        background: #ede6da;
        font-family: 'DM Sans', sans-serif;
        overflow: hidden;
        user-select: none;
        cursor: default;
    }

    /* ── Root ── */
    .plugin-root {
        display: flex;
        flex-direction: column;
        background: #ede6da;
        min-height: 100vh;
        padding: 0;
    }

    /* ── Top bar ── */
    .top-bar {
        display: flex;
        align-items: center;
        justify-content: space-between;
        padding: 10px 20px 8px;
        border-bottom: 1px solid rgba(150,130,100,0.15);
    }

    .plugin-title {
        display: flex;
        align-items: baseline;
        gap: 10px;
    }

    .title-main {
        font-family: 'DM Serif Display', serif;
        font-size: 1.1rem;
        letter-spacing: 0.12em;
        color: rgba(110, 88, 60, 0.85);
    }

    .title-sub {
        font-size: 0.6rem;
        font-weight: 200;
        letter-spacing: 0.2em;
        text-transform: uppercase;
        color: rgba(140, 115, 82, 0.5);
    }

    .window-controls {
        display: flex;
        gap: 7px;
    }

    .win-btn {
        width: 13px;
        height: 13px;
        border-radius: 50%;
        background: #e8e0d4;
        box-shadow: 3px 3px 6px rgba(150,130,100,0.4), -2px -2px 5px rgba(255,252,244,0.9);
    }
    .win-btn.red    { background: rgba(220, 100, 80, 0.55); }
    .win-btn.yellow { background: rgba(220, 180, 60, 0.55); }
    .win-btn.green  { background: rgba(80, 180, 100, 0.55); }

    /* ── Main panels row ── */
    .panels-row {
        display: flex;
        flex: 1;
        gap: 0;
        padding: 12px 12px 8px;
        align-items: stretch;
        overflow: hidden;
    }

    /* ── Panel base ── */
    .panel {
        background: #ede6da;
        border-radius: 20px;
        box-shadow:
            10px 10px 24px rgba(155,135,105,0.32),
            -7px -7px 20px rgba(255,252,244,0.92);
        padding: 14px 12px;
        display: flex;
        flex-direction: column;
        gap: 10px;
        margin: 0 4px;
    }

    .panel-input-filter  { flex: 1.2; }
    .panel-early-ref     { flex: 1.2; }
    .panel-diffusion     { flex: 2.5; }
    .panel-chorus        { flex: 0.9; min-width: 110px; }
    .panel-output        { flex: 0.7; min-width: 90px; }

    .panel-title {
        font-size: 0.58rem;
        font-weight: 300;
        letter-spacing: 0.22em;
        text-transform: uppercase;
        color: rgba(130, 110, 85, 0.65);
        text-align: center;
        padding-bottom: 6px;
        border-bottom: 1px solid rgba(150,130,100,0.12);
    }

    .panel-title-vertical {
        font-size: 0.55rem;
        font-weight: 300;
        letter-spacing: 0.22em;
        text-transform: uppercase;
        color: rgba(130, 110, 85, 0.65);
        text-align: center;
    }

    .panel-body {
        display: flex;
        flex-direction: column;
        align-items: center;
        flex: 1;
    }

    /* ── Shared layout helpers ── */
    .row {
        display: flex;
        align-items: center;
        gap: 12px;
    }

    .col {
        display: flex;
        flex-direction: column;
        align-items: center;
    }

    /* ── Frequency graph ── */
    .freq-graph {
        width: 100%;
        background: #e8e0d4;
        border-radius: 10px;
        box-shadow:
            inset 3px 3px 7px rgba(150,130,100,0.28),
            inset -2px -2px 5px rgba(255,250,240,0.75);
        padding: 6px;
        box-sizing: border-box;
    }

    .freq-graph.wide { height: 80px; }

    .freq-svg {
        width: 100%;
        height: 50px;
        display: block;
    }

    .freq-readouts {
        display: flex;
        gap: 12px;
        justify-content: center;
    }

    .freq-val {
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 2px;
    }

    .freq-label {
        font-size: 0.5rem;
        letter-spacing: 0.2em;
        text-transform: uppercase;
        color: rgba(140, 115, 82, 0.45);
    }

    .freq-num {
        font-size: 0.62rem;
        font-weight: 300;
        color: rgba(120, 95, 65, 0.75);
        letter-spacing: 0.06em;
    }

    /* ── Diffusion panel center ── */
    .diffusion-center {
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 10px;
        width: 100%;
    }

    .xy-and-params {
        display: flex;
        align-items: center;
        justify-content: center;
        gap: 8px;
    }

    /* ── Display blocks ── */
    .display-block {
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 2px;
        background: #e8e0d4;
        border-radius: 8px;
        box-shadow:
            inset 2px 2px 5px rgba(150,130,100,0.25),
            inset -1px -1px 3px rgba(255,250,240,0.7);
        padding: 5px 8px;
        min-width: 60px;
    }

    .disp-label {
        font-size: 0.5rem;
        letter-spacing: 0.2em;
        text-transform: uppercase;
        color: rgba(140, 115, 82, 0.45);
    }

    .disp-val {
        font-size: 0.65rem;
        font-weight: 300;
        color: rgba(120, 95, 65, 0.8);
        letter-spacing: 0.04em;
    }

    /* ── Bottom row ── */
    .bottom-row {
        display: flex;
        align-items: center;
        justify-content: space-around;
        padding: 8px 20px 14px;
        background: #ede6da;
        box-shadow: 0 -1px 0 rgba(150,130,100,0.12);
        gap: 8px;
        flex-wrap: nowrap;
    }
</style>
