<script lang="ts">
    import { store, setParameterValue } from './state/store';
    import { bridge } from './bridge/bridge';
    import type { ParameterId } from './types/parameters';

    import NeuKnob     from './components/NeuKnob.svelte';
    import NeuButton   from './components/NeuButton.svelte';
    import NeuSelector from './components/NeuSelector.svelte';
    import NeuXYPad    from './components/NeuXYPad.svelte';

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

    // XY Pad → decay (X) and diffusion (Y) macro
    let xyX = 0.42;
    let xyY = 0.6;
    function onXYChange(e: CustomEvent) {
        xyX = e.detail.x;
        xyY = e.detail.y;
        send('decay', xyX);
        send('diffusion', xyY);
    }

    // Selector indices derived from normalized values
    $: modeSelected   = Math.round($store.reverbMode.value);
    $: smoothSelected = Math.round($store.smooth.value * 3);
    $: densitySelected = Math.round($store.density.value * 3);
    $: flatCutSelected = Math.round($store.flatCut.value);
</script>

<main class="plugin-root">

    <!-- ========== TOP BAR ========== -->
    <div class="top-bar">
        <div class="plugin-title">
            <span class="title-main">Audio Plugin 02</span>
            <span class="title-sub">FDN Reverb · Professional</span>
        </div>
        <div class="window-controls">
            <div class="win-btn red"></div>
            <div class="win-btn yellow"></div>
            <div class="win-btn green"></div>
        </div>
    </div>

    <!-- ========== MAIN PANELS ========== -->
    <div class="panels-row">

        <!-- ===== PANEL 1: INPUT FILTER ===== -->
        <section class="panel panel-input-filter">
            <div class="panel-title">Input Filter</div>

            <div class="panel-body" style="gap:14px; align-items:center;">
                <div class="row">
                    <NeuButton
                        label="Lo Cut"
                        active={$store.loCutEnabled.value > 0.5}
                        on:change={e => sendBool('loCutEnabled', e.detail.active)}
                    />
                    <NeuButton
                        label="Hi Cut"
                        active={$store.hiCutEnabled.value > 0.5}
                        on:change={e => sendBool('hiCutEnabled', e.detail.active)}
                    />
                </div>

                <!-- Frequency response placeholder -->
                <div class="freq-graph">
                    <svg viewBox="0 0 120 60" class="freq-svg">
                        <path
                            d="M 0 50 Q 20 50 30 30 Q 40 10 60 10 Q 80 10 90 30 Q 100 50 120 50"
                            fill="none" stroke="rgba(160,130,90,0.4)" stroke-width="1.5"
                        />
                    </svg>
                </div>

                <div class="freq-readouts">
                    <div class="freq-val">
                        <span class="freq-label">LO</span>
                        <span class="freq-num">
                            {Math.round(20 + $store.loCutFreq.value * 480)} Hz
                        </span>
                    </div>
                    <div class="freq-val">
                        <span class="freq-label">HI</span>
                        <span class="freq-num">
                            {Math.round(1000 + $store.hiCutFreq.value * 19000)} Hz
                        </span>
                    </div>
                </div>

                <NeuKnob label="LO FREQ" value={$store.loCutFreq.value} min={20} max={500} unit=" Hz"
                    on:change={e => send('loCutFreq', e.detail.value)} />
                <NeuKnob label="HI FREQ" value={$store.hiCutFreq.value} min={1000} max={20000} unit=" Hz"
                    on:change={e => send('hiCutFreq', e.detail.value)} />
            </div>
        </section>

        <!-- ===== PANEL 2: EARLY REFLECTIONS ===== -->
        <section class="panel panel-early-ref">
            <div class="panel-title">Early Reflections</div>

            <div class="panel-body" style="gap:14px; align-items:center;">
                <NeuButton
                    label="Spin"
                    active={$store.erEnabled.value > 0.5}
                    icon="↺"
                    on:change={e => sendBool('erEnabled', e.detail.active)}
                />

                <div class="row">
                    <NeuKnob label="Amount" value={$store.erAmount.value} min={0} max={100} unit="%"
                        on:change={e => send('erAmount', e.detail.value)} />
                    <NeuKnob label="Rate" value={$store.erRate.value} min={0.1} max={10} unit=" Hz"
                        on:change={e => send('erRate', e.detail.value)} />
                </div>

                <NeuKnob label="Shape" value={$store.erShape.value} min={0} max={100} unit="%"
                    on:change={e => send('erShape', e.detail.value)} />
            </div>
        </section>

        <!-- ===== PANEL 3: DIFFUSION NETWORK (largest) ===== -->
        <section class="panel panel-diffusion">
            <div class="panel-title">Diffusion Network</div>

            <div class="panel-body" style="gap:12px;">

                <!-- Top row: mode selector + crossover freq display -->
                <div class="row" style="justify-content:space-between; width:100%;">
                    <NeuSelector
                        label="Mode"
                        options={['High', 'Low']}
                        selected={modeSelected}
                        on:change={e => sendSelector('reverbMode', e.detail.selected, 2)}
                    />
                    <div class="display-block">
                        <span class="disp-label">Freq</span>
                        <span class="disp-val">
                            {(200 + $store.crossoverFreq.value * 7800).toFixed(0)} Hz
                        </span>
                    </div>
                    <div class="display-block">
                        <span class="disp-label">Diffuse</span>
                        <span class="disp-val">{($store.diffusion.value * 100).toFixed(0)}%</span>
                    </div>
                </div>

                <!-- Center: XY Pad + freq graph -->
                <div class="diffusion-center">
                    <div class="freq-graph wide">
                        <svg viewBox="0 0 200 80" class="freq-svg">
                            <!-- Frequency response shape -->
                            <path
                                d="M 0 70 C 30 70 40 20 80 15 C 120 10 140 40 160 60 C 180 70 200 70 200 70"
                                fill="rgba(160,130,90,0.08)" stroke="rgba(160,130,90,0.4)" stroke-width="1.5"
                            />
                            <!-- Node 1 -->
                            <circle cx={80 + $store.crossoverFreq.value * 40} cy={15 + $store.damping.value * 40}
                                r="6" fill="#e8e0d4"
                                stroke="rgba(150,120,80,0.4)" stroke-width="1.5"
                            />
                            <!-- Node 2 -->
                            <circle cx={140 + $store.feedback.value * 30} cy={40 + $store.diffusion.value * 20}
                                r="6" fill="#e8e0d4"
                                stroke="rgba(150,120,80,0.4)" stroke-width="1.5"
                            />
                        </svg>
                    </div>

                    <div class="xy-and-params">
                        <NeuXYPad
                            x={xyX}
                            y={xyY}
                            labelX="DECAY"
                            labelY="DIFFUSE"
                            on:change={onXYChange}
                        />
                        <div class="col" style="gap:10px; margin-left:16px;">
                            <NeuKnob label="Diffusion" value={$store.diffusion.value} min={0} max={100} unit="%"
                                on:change={e => send('diffusion', e.detail.value)} />
                            <NeuKnob label="Scale" value={$store.scale.value} min={0} max={100} unit="%"
                                on:change={e => send('scale', e.detail.value)} />
                        </div>
                    </div>
                </div>

                <!-- Bottom: crossover + damping + feedback knobs -->
                <div class="row" style="gap:14px; justify-content:center;">
                    <NeuKnob label="Crossover" value={$store.crossoverFreq.value} min={200} max={8000} unit=" Hz"
                        on:change={e => send('crossoverFreq', e.detail.value)} />
                    <NeuKnob label="Damping" value={$store.damping.value} min={0} max={100} unit="%"
                        on:change={e => send('damping', e.detail.value)} />
                    <NeuKnob label="Feedback" value={$store.feedback.value} min={0} max={100} unit="%"
                        on:change={e => send('feedback', e.detail.value)} />
                </div>
            </div>
        </section>

        <!-- ===== PANEL 4: CHORUS ===== -->
        <section class="panel panel-chorus">
            <div class="panel-title">Chorus</div>

            <div class="panel-body" style="gap:14px; align-items:center;">
                <NeuKnob label="Amount" value={$store.chorusAmount.value} min={0} max={100} unit="%"
                    on:change={e => send('chorusAmount', e.detail.value)} />
                <NeuKnob label="Rate" value={$store.chorusRate.value} min={0.1} max={10} unit=" Hz"
                    on:change={e => send('chorusRate', e.detail.value)} />
            </div>
        </section>

        <!-- ===== PANEL 5: OUTPUT / REFLECT STRIP ===== -->
        <section class="panel panel-output">
            <div class="panel-title-vertical">Reflect</div>

            <div class="panel-body" style="gap:14px; align-items:center;">
                <NeuKnob label="Reflect" value={$store.reflectGain.value} min={-24} max={6} unit=" dB"
                    on:change={e => send('reflectGain', e.detail.value)} />
                <NeuKnob label="Diffuse" value={$store.diffuseGain.value} min={-24} max={6} unit=" dB"
                    on:change={e => send('diffuseGain', e.detail.value)} />
                <NeuKnob label="Dry/Wet" value={$store.dryWet.value} min={0} max={100} unit="%"
                    on:change={e => send('dryWet', e.detail.value)} />
            </div>
        </section>

    </div>

    <!-- ========== BOTTOM UTILITY ROW ========== -->
    <div class="bottom-row">

        <NeuKnob label="Predelay" value={$store.predelay.value} min={0} max={500} unit=" ms"
            on:change={e => send('predelay', e.detail.value)} />

        <NeuSelector
            label="Smooth"
            options={['Off', 'Low', 'Med', 'High']}
            selected={smoothSelected}
            on:change={e => sendSelector('smooth', e.detail.selected, 4)}
        />

        <NeuKnob label="Size" value={$store.size.value} min={0} max={100} unit="%"
            on:change={e => send('size', e.detail.value)} />

        <NeuKnob label="Decay" value={$store.decay.value} min={100} max={10000} unit=" ms"
            on:change={e => send('decay', e.detail.value)} />

        <NeuButton
            label="Freeze"
            active={$store.freeze.value > 0.5}
            icon="❄"
            on:change={e => sendBool('freeze', e.detail.active)}
        />

        <NeuSelector
            label="Flat/Cut"
            options={['Flat', 'Cut']}
            selected={flatCutSelected}
            on:change={e => sendSelector('flatCut', e.detail.selected, 2)}
        />

        <NeuKnob label="Stereo" value={$store.stereo.value} min={0} max={200} unit="%"
            on:change={e => send('stereo', e.detail.value)} />

        <NeuSelector
            label="Density"
            options={['Low', 'Med', 'High', 'Ultra']}
            selected={densitySelected}
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
