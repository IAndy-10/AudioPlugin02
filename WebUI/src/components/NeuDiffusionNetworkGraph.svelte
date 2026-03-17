<script>
  import { createEventDispatcher, onMount } from 'svelte';
  import NeuNumber from './NeuNumber.svelte';

  // All values normalized 0-1 (same convention as the store)
  export let crossoverFreq = 0.5;   // Node 1 X — low-band boundary (100–5000 Hz)
  export let diffusion     = 0.6;   // Node 1 Y — low-band diffusion amount
  export let damping       = 0.5;   // Node 2 X — high-band damping frequency
  export let feedback      = 0.75;  // Node 2 Y — feedback / reverb tail level

  const dispatch = createEventDispatcher();

  // ── Canvas constants ──────────────────────────────────────
  const CW = 320, CH = 130;
  const MX = 8,  MY = 10;
  const FMIN = 50, FMAX = 20000;

  // Node 1 (Low): crossoverFreq → 100–5000 Hz
  const N1_FMIN = 100,  N1_FMAX = 5000;
  // Node 2 (High): damping → 800–18000 Hz
  const N2_FMIN = 800,  N2_FMAX = 18000;

  let canvas;
  let dragging   = false;
  let activeNode = 0;
  let dragStartClientX, dragStartClientY;
  let dragStart = {};

  // ── Conversions ───────────────────────────────────────────
  function freqToX(f) {
    return MX + (CW - 2 * MX) * Math.log10(f / FMIN) / Math.log10(FMAX / FMIN);
  }
  function xToFreq(x) {
    return FMIN * Math.pow(FMAX / FMIN, (x - MX) / (CW - 2 * MX));
  }
  function n1ToFreq(p) { return N1_FMIN * Math.pow(N1_FMAX / N1_FMIN, p); }
  function freqToN1(f) { return Math.log(f / N1_FMIN) / Math.log(N1_FMAX / N1_FMIN); }
  function n2ToFreq(p) { return N2_FMIN * Math.pow(N2_FMAX / N2_FMIN, p); }
  function freqToN2(f) { return Math.log(f / N2_FMIN) / Math.log(N2_FMAX / N2_FMIN); }
  // param 0-1 → Y (1 = top, 0 = bottom)
  function pToY(p)  { return MY + (1 - p) * (CH - 2 * MY); }
  function yToP(y)  { return Math.max(0, Math.min(1, 1 - (y - MY) / (CH - 2 * MY))); }

  // ── Node positions ────────────────────────────────────────
  function getN1() {
    const f = n1ToFreq(crossoverFreq);
    return { x: freqToX(f), y: pToY(diffusion), f };
  }
  function getN2() {
    const f = n2ToFreq(damping);
    return { x: freqToX(f), y: pToY(feedback), f };
  }

  // ── Spectral envelope (smooth shelf from diffusion → feedback) ──
  function specMag(f) {
    const f1 = n1ToFreq(crossoverFreq);
    const f2 = n2ToFreq(damping);
    if (f1 >= f2) return (diffusion + feedback) * 0.5;
    const t = Math.max(0, Math.min(1, Math.log(f / f1) / Math.log(f2 / f1)));
    const s = t * t * (3 - 2 * t); // smoothstep
    return diffusion * (1 - s) + feedback * s;
  }

  // ── Draw ──────────────────────────────────────────────────
  function draw() {
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const n1  = getN1();
    const n2  = getN2();

    // Background
    ctx.fillStyle = '#1a1a1a';
    ctx.fillRect(0, 0, CW, CH);

    // Grid
    ctx.lineWidth = 0.5;
    ctx.strokeStyle = 'rgba(255,255,255,0.06)';
    for (const gf of [100, 200, 500, 1000, 2000, 5000, 10000]) {
      const gx = freqToX(gf);
      ctx.beginPath(); ctx.moveTo(gx, 0); ctx.lineTo(gx, CH); ctx.stroke();
    }
    ctx.strokeStyle = 'rgba(255,255,255,0.04)';
    ctx.beginPath(); ctx.moveTo(0, CH / 2); ctx.lineTo(CW, CH / 2); ctx.stroke();

    // Spectral envelope fill
    const pts = [];
    for (let px = MX; px <= CW - MX; px++) {
      pts.push([px, pToY(specMag(xToFreq(px)))]);
    }
    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.lineTo(CW - MX, CH); ctx.lineTo(MX, CH); ctx.closePath();
    ctx.fillStyle = 'rgba(0,200,180,0.07)';
    ctx.fill();

    // Spectral envelope stroke
    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.strokeStyle = '#00c8b4';
    ctx.lineWidth = 1.5; ctx.lineJoin = 'round'; ctx.stroke();

    // Vertical dashed guides
    ctx.setLineDash([2, 3]); ctx.lineWidth = 0.7;
    ctx.beginPath(); ctx.moveTo(n1.x, MY); ctx.lineTo(n1.x, CH - MY);
    ctx.strokeStyle = 'rgba(0,200,180,0.25)'; ctx.stroke();
    ctx.beginPath(); ctx.moveTo(n2.x, MY); ctx.lineTo(n2.x, CH - MY);
    ctx.strokeStyle = 'rgba(245,166,35,0.25)'; ctx.stroke();
    ctx.setLineDash([]);

    // ── Node 1: teal circle with "1" ─────────────────────
    ctx.beginPath(); ctx.arc(n1.x, n1.y, 9, 0, Math.PI * 2);
    ctx.fillStyle = (dragging && activeNode === 1) ? '#20e8d4' : '#00c8b4'; ctx.fill();
    ctx.beginPath(); ctx.arc(n1.x, n1.y, 9, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(150,240,220,0.45)'; ctx.lineWidth = 1.2; ctx.stroke();
    ctx.fillStyle = '#0e1f1e';
    ctx.font = 'bold 9px "DM Sans", sans-serif';
    ctx.textAlign = 'center'; ctx.textBaseline = 'middle';
    ctx.fillText('1', n1.x, n1.y);

    // ── Node 2: orange circle with "2" ───────────────────
    ctx.beginPath(); ctx.arc(n2.x, n2.y, 9, 0, Math.PI * 2);
    ctx.fillStyle = (dragging && activeNode === 2) ? '#ffb940' : '#f5a623'; ctx.fill();
    ctx.beginPath(); ctx.arc(n2.x, n2.y, 9, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(255,220,100,0.45)'; ctx.lineWidth = 1.2; ctx.stroke();
    ctx.fillStyle = '#231700';
    ctx.fillText('2', n2.x, n2.y);

    ctx.textBaseline = 'alphabetic';
  }

  $: if (canvas) { crossoverFreq; diffusion; damping; feedback; draw(); }
  onMount(() => draw());

  // ── Mouse down: detect which node ────────────────────────
  function onCanvasMouseDown(e) {
    const rect  = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;
    const mx = (e.clientX - rect.left) * scaleX;
    const my = (e.clientY - rect.top)  * scaleY;

    const n1 = getN1(), n2 = getN2();
    const d1 = Math.hypot(mx - n1.x, my - n1.y);
    const d2 = Math.hypot(mx - n2.x, my - n2.y);
    const HIT = 15;

    if (d1 < HIT || d2 < HIT) {
      activeNode       = (d1 <= d2 && d1 < HIT) ? 1 : 2;
      dragging         = true;
      dragStartClientX = e.clientX;
      dragStartClientY = e.clientY;
      dragStart        = { crossoverFreq, diffusion, damping, feedback };
      window.addEventListener('mousemove', onDragMove);
      window.addEventListener('mouseup',   onDragUp);
    }
  }

  function onDragMove(e) {
    if (!dragging) return;
    const rect  = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;
    const dx = (e.clientX - dragStartClientX) * scaleX;
    const dy = (e.clientY - dragStartClientY) * scaleY;

    if (activeNode === 1) {
      const startX  = freqToX(n1ToFreq(dragStart.crossoverFreq));
      const newF    = Math.min(N1_FMAX, Math.max(N1_FMIN, xToFreq(startX + dx)));
      crossoverFreq = freqToN1(newF);
      diffusion     = yToP(pToY(dragStart.diffusion) + dy);
    } else {
      const startX = freqToX(n2ToFreq(dragStart.damping));
      const newF   = Math.min(N2_FMAX, Math.max(N2_FMIN, xToFreq(startX + dx)));
      damping   = freqToN2(newF);
      feedback  = yToP(pToY(dragStart.feedback) + dy);
    }

    draw();
    dispatch('change', { crossoverFreq, diffusion, damping, feedback });
  }

  function onDragUp() {
    dragging = false;
    draw();
    window.removeEventListener('mousemove', onDragMove);
    window.removeEventListener('mouseup',   onDragUp);
  }

  // ── NeuNumber handlers ────────────────────────────────────
  function onCrossChange(e) {
    crossoverFreq = freqToN1(Math.max(N1_FMIN, Math.min(N1_FMAX, e.detail.value)));
    dispatch('change', { crossoverFreq, diffusion, damping, feedback });
  }
  function onDiffChange(e) {
    diffusion = e.detail.value / 100;
    dispatch('change', { crossoverFreq, diffusion, damping, feedback });
  }
  function onDampChange(e) {
    damping = freqToN2(Math.max(N2_FMIN, Math.min(N2_FMAX, e.detail.value)));
    dispatch('change', { crossoverFreq, diffusion, damping, feedback });
  }
  function onFeedChange(e) {
    feedback = e.detail.value / 100;
    dispatch('change', { crossoverFreq, diffusion, damping, feedback });
  }
</script>

<div class="dng-wrap">

  <!-- ── Canvas ──────────────────────────────────────────── -->
  <div class="canvas-shell">
    <canvas
      bind:this={canvas}
      width={CW}
      height={CH}
      class="graph-canvas"
      class:dragging
      on:mousedown={onCanvasMouseDown}
    />
  </div>

  <!-- ── Controls: one row per node ──────────────────────── -->
  <div class="controls">

    <div class="node-row">
      <span class="dot dot-1"></span>
      <NeuNumber label="CROSS"
        value={Math.round(n1ToFreq(crossoverFreq))}
        min={N1_FMIN} max={N1_FMAX} step={10} unit=" Hz" decimals={0}
        on:change={onCrossChange}
      />
      <NeuNumber label="DIFF"
        value={+(diffusion * 100).toFixed(1)}
        min={0} max={100} step={1} unit="%" decimals={1}
        on:change={onDiffChange}
      />
    </div>

    <div class="node-row">
      <span class="dot dot-2"></span>
      <NeuNumber label="DAMP"
        value={Math.round(n2ToFreq(damping))}
        min={N2_FMIN} max={N2_FMAX} step={50} unit=" Hz" decimals={0}
        on:change={onDampChange}
      />
      <NeuNumber label="FEED"
        value={+(feedback * 100).toFixed(1)}
        min={0} max={100} step={1} unit="%" decimals={1}
        on:change={onFeedChange}
      />
    </div>

  </div>

</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:wght@200;300;400&display=swap');

  .dng-wrap {
    display: flex;
    flex-direction: column;
    gap: 8px;
    user-select: none;
    width: 100%;
  }

  .canvas-shell {
    border-radius: 10px;
    overflow: hidden;
    box-shadow:
      inset 4px 4px 10px rgba(140, 120, 90, 0.35),
      inset -3px -3px 7px rgba(255, 250, 240, 0.6);
    background: #1a1a1a;
    line-height: 0;
  }

  .graph-canvas {
    display: block;
    width: 100%;
    height: auto;
    cursor: crosshair;
    border-radius: 10px;
  }

  .graph-canvas.dragging { cursor: grabbing; }

  .controls {
    display: flex;
    flex-direction: column;
    gap: 6px;
  }

  .node-row {
    display: flex;
    align-items: center;
    gap: 10px;
    justify-content: center;
  }

  .dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    flex-shrink: 0;
  }
  .dot-1 { background: #00c8b4; box-shadow: 0 0 4px rgba(0,200,180,0.5); }
  .dot-2 { background: #f5a623; box-shadow: 0 0 4px rgba(245,166,35,0.5); }
</style>
