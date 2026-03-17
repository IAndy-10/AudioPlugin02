<script lang="ts">
    import {
        params, setParameterValue,
        modeSelected, smoothSelected, densitySelected,
        flatEnabledSelected, cutEnabledSelected,
        crossoverHz, diffusionPct,
        toNormalized, sizeToNormalized
    } from './state/store';
    import { bridge } from './bridge/bridge';
    import type { ParameterId } from './types/parameters';

    import NeuKnob     from './components/NeuKnob.svelte';
    import NeuButton   from './components/NeuButton.svelte';
    import NeuSelector from './components/NeuSelector.svelte';
    import NeuNumber   from './components/NeuNumber.svelte';
    import FilterGraph from './components/FilterGraph.svelte';
    import NeuDiffusionNetworkGraph from './components/NeuDiffusionNetworkGraph.svelte';
    import NeuKnobDiscrete from './components/NeuKnobDiscrete.svelte';

    // Destructure individual stores so Svelte's $X auto-subscribe syntax works
    const {
        loCutEnabled, hiCutEnabled, loCutFreq, hiCutFreq, hiCutQ,
        erEnabled, erAmount, erRate, erShape,
        crossoverFreq, diffusion, scale, decay, damping, feedback,
        highFilterType,
        chorusEnabled, chorusAmount, chorusRate,
        reflectGain, diffuseGain, dryWet,
        predelay, smooth, size, freeze,
        flatEnabled, cutEnabled, stereo, density,
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

    // Reactive display values for FilterGraph (denormalize from store)
    $: freqDisp = Math.max(50, Math.min(18000, 50 + 17950 * Math.pow($hiCutFreq, 1 / 0.3)));
    $: qDisp    = Math.max(0.5, Math.min(9.0,   0.5 + 8.5   * Math.pow($hiCutQ,    1 / 0.5)));

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

        <!-- ===== PANEL 1: INPUT ===== -->
        <section class="panel panel-input">
            <div class="panel-title">Input</div>
            <div class="panel-body">

                <FilterGraph
                    freq={freqDisp}
                    q={qDisp}
                    loCut={$loCutEnabled > 0.5}
                    hiCut={$hiCutEnabled > 0.5}
                    highFilterType={$highFilterType > 0.5}
                    on:change={e => {
                        send('loCutEnabled', e.detail.loCut ? 1 : 0);
                        send('hiCutEnabled', e.detail.hiCut ? 1 : 0);
                        send('hiCutFreq', toNormalized(e.detail.freq, 50, 18000, 0.3));
                        send('hiCutQ',    toNormalized(e.detail.q,    0.5, 9.0,  0.5));
                    }}
                />

                <NeuKnob label="Predelay" value={$predelay} min={0} max={500} unit=" ms"
                    on:change={e => send('predelay', e.detail.value)} />

            </div>
        </section>

        <!-- ===== PANEL 2: EARLY REFLECTIONS ===== -->
        <section class="panel panel-early-ref">
            <div class="panel-title">Early Reflections</div>
            <div class="panel-body">

                <NeuButton
                    label="Spin"
                    active={$erEnabled > 0.5}
                    icon="↺"
                    on:change={e => sendBool('erEnabled', e.detail.active)}
                />

                <div class="row">
                    <NeuNumber label="Amount"
                        value={+(2 + 53 * Math.pow($erAmount, 2)).toFixed(1)}
                        min={2} max={55} step={0.5} decimals={1}
                        on:change={e => send('erAmount', Math.pow((e.detail.value - 2) / 53, 0.5))}
                    />
                    <NeuNumber label="Rate"
                        value={+(0.07 + 1.23 * Math.pow($erRate, 2)).toFixed(2)}
                        min={0.07} max={1.3} step={0.01} unit=" Hz" decimals={2}
                        on:change={e => send('erRate', Math.pow((e.detail.value - 0.07) / 1.23, 0.5))}
                    />
                </div>

                <div class="subsection-label">Shape</div>
                <div class="row">
                    <NeuKnob label="Shape" value={$erShape} min={0} max={1} unit=""
                        on:change={e => send('erShape', e.detail.value)} />
                    <NeuKnob label="Reflect" value={$reflectGain} min={-30} max={6} unit=" dB"
                        on:change={e => send('reflectGain', e.detail.value)} />
                </div>

            </div>
        </section>

        <!-- ===== PANEL 3: DIFFUSION NETWORK ===== -->
        <section class="panel panel-diffusion">
            <div class="panel-title">Diffusion Network</div>
            <div class="panel-body">

                <NeuDiffusionNetworkGraph
                    crossoverFreq={$crossoverFreq}
                    diffusion={$diffusion}
                    damping={$damping}
                    feedback={$feedback}
                    highFilterType={$highFilterType > 0.5}
                    on:change={e => {
                        send('crossoverFreq', e.detail.crossoverFreq);
                        send('diffusion',     e.detail.diffusion);
                        send('damping',       e.detail.damping);
                        send('feedback',      e.detail.feedback);
                    }}
                />

                <div class="subsection-label">Difussion</div>
                <div class="row" style="gap:10px; margin-top:4px;">
                    <NeuNumber label="Diffusion Density"
                        value={+($diffusion * 100).toFixed(1)}
                        min={0} max={100} step={1} unit="%" decimals={1}
                        on:change={e => send('diffusion', e.detail.value / 100)}
                    />
                    <NeuNumber label="Scale"
                        value={+($scale * 100).toFixed(1)}
                        min={0} max={100} step={1} unit="%" decimals={1}
                        on:change={e => send('scale', e.detail.value / 100)}
                    />
                <!-- ⚠ Scale is registered in APVTS but unconnected to DSP. Wiring in a future step. -->

                <NeuNumber label="Diffuse" value={+(-30 + 36 * $diffuseGain).toFixed(1)} min={-30} max={6} step={0.5} unit=" dB" decimals={1}
                    on:change={e => send('diffuseGain', (e.detail.value + 30) / 36)} />
                </div>

                <div class="row" style="gap:8px;">
                    <div>
                        <div class="subsection-label">Chorus</div>
                        <div class="row" style="gap:8px;">
                            <NeuButton label="Chorus"
                                active={$chorusEnabled > 0.5}
                                icon=":)"
                                on:change={e => sendBool('chorusEnabled', e.detail.active)}
                            />
                            <div class="col">
                                <NeuNumber label="Amount"
                                    value={+(0.01 + 3.99 * $chorusAmount).toFixed(2)}
                                    min={0.01} max={4.0} step={0.01} decimals={2}
                                    on:change={e => send('chorusAmount', (e.detail.value - 0.01) / 3.99)}
                                />
                                <NeuNumber label="Rate"
                                    value={+(0.01 + 7.99 * Math.pow($chorusRate, 2)).toFixed(2)}
                                    min={0.01} max={8.0} step={0.01} unit=" Hz" decimals={2}
                                    on:change={e => send('chorusRate', Math.pow((e.detail.value - 0.01) / 7.99, 0.5))}
                                />
                            </div>
                        </div>
                    </div>  
                    <div>
                        <div class="subsection-label">Size</div>
                        <div class="row" style="gap:10px;">
                            <NeuKnobDiscrete
                                label="Smooth"
                                options={['Off', 'Low', 'Med', 'High']}
                                selected={$smoothSelected}
                                on:change={e => sendSelector('smooth', e.detail.selected, 4)}
                            />
                            <NeuKnob label="Size" value={$size} min={0.22} max={500} unit=""
                                on:change={e => send('size', e.detail.value)} />
                    </div>
                </div>    
            </div>

            </div>
        </section>

        <!-- ===== PANEL 4: DECAY ===== -->
        <section class="panel panel-decay">
            <div class="panel-title">Decay</div>
            <div class="panel-body">

                <NeuKnob label="Decay" value={$decay} min={200} max={60000} unit=" ms"
                    on:change={e => send('decay', e.detail.value)} />

                <div class="subsection-label">Mode</div>
                <div class="triangle-group">
                        <NeuButton
                        label="Freeze"
                        active={$freeze > 0.5}
                        icon="❄"
                        on:change={e => sendBool('freeze', e.detail.active)}
                    />
                    <div class="triangle-top">
                        <NeuButton label="Flat"
                            active={$flatEnabled > 0.5}
                            on:change={e => sendBool('flatEnabled', e.detail.active)}
                        />
                        <NeuButton label="Cut"
                            active={$cutEnabled > 0.5}
                            on:change={e => sendBool('cutEnabled', e.detail.active)}
                        />
                    </div>
                </div>
                <!-- ⚠ Flat / Cut: registered in APVTS but DSP wiring pending. -->

            </div>
        </section>

        <!-- ===== PANEL 5: OUTPUT ===== -->
        <section class="panel panel-output">
            <div class="panel-title">Output</div>
            <div class="panel-body">

                <NeuKnobDiscrete
                label="Density"
                options={['Sparse', 'Low', 'Mid', 'High']}
                selected={$densitySelected}
                on:change={e => sendSelector('density', e.detail.selected, 4)}
                />

                <NeuKnob label="Stereo" value={$stereo} min={0} max={120} unit="°"
                    on:change={e => send('stereo', e.detail.value)} />

                <NeuKnob label="Dry/Wet" value={$dryWet} min={0} max={100} unit="%"
                    on:change={e => send('dryWet', e.detail.value)} />

            </div>
        </section>

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

    .plugin-root {
        display: flex;
        flex-direction: column;
        background: #ede6da;
        min-height: 100vh;
        padding: 0;
    }

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

    .panels-row {
        display: flex;
        flex: 1;
        gap: 0;
        padding: 12px 12px 8px;
        align-items: stretch;
        overflow: hidden;
    }

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

    .panel-input      { flex: 1.4; }
    .panel-early-ref  { flex: 1.1; }
    .panel-diffusion  { flex: 2.8; }
    .panel-decay      { flex: 0.9; min-width: 120px; }
    .panel-output     { flex: 0.7; min-width: 90px; }

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

    .panel-body {
        display: flex;
        flex-direction: column;
        align-items: center;
        flex: 1;
        gap: 10px;
    }

    .subsection-label {
        font-size: 0.5rem;
        font-weight: 300;
        letter-spacing: 0.2em;
        text-transform: uppercase;
        color: rgba(140, 115, 82, 0.4);
        width: 100%;
        text-align: center;
        padding-top: 4px;
        border-top: 1px solid rgba(150,130,100,0.08);
        margin-top: 2px;
    }

    .row {
        display: flex;
        align-items: center;
        gap: 12px;
    }

    .col {
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 6px;
    }

    .triangle-group {
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 6px;
    }

    .triangle-top {
        display: flex;
        gap: 10px;
    }
</style>
