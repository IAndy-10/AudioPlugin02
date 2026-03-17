<script>
  import { createEventDispatcher, onMount } from 'svelte';
  import NeuNumber from './NeuNumber.svelte';
  import NeuButton from './NeuButton.svelte';

  export let freq   = 1000;   // center frequency, Hz (20–20000)
  export let q      = 1.0;    // Q factor (0.1–20)
  export let loCut  = false;  // high-pass toggle
  export let hiCut  = false;  // low-pass toggle

  const dispatch = createEventDispatcher();

  // ── Canvas constants ───────────────────────────────────────
  const CW = 300, CH = 110;
  const MX = 6, MY = 8;             // x / y margins
  const FMIN = 50, FMAX = 18000;
  const QMIN = 0.1, QMAX = 9.0;
  const LOCUT_FC = 100;             // fixed Lo Cut corner (Hz)
  const HICUT_FC = 8000;            // fixed Hi Cut corner (Hz)
  const DB_TOP   = 6;               // +6 dB headroom at canvas top
  const DB_BOT   = -48;             // noise floor at canvas bottom

  let canvas;

  // ── Frequency ↔ canvas-X (log scale) ──────────────────────
  function freqToX(f) {
    return MX + (CW - 2 * MX) * Math.log10(f / FMIN) / Math.log10(FMAX / FMIN);
  }
  function xToFreq(x) {
    const norm = (x - MX) / (CW - 2 * MX);
    return FMIN * Math.pow(FMAX / FMIN, norm);
  }

  // ── Magnitude → canvas-Y (dB scale) ──────────────────────
  // 0 dB (mag=1) sits near the top; +6 dB at the very top to
  // leave headroom for the LP resonance bump.
  function magToY(mag) {
    const db = 20 * Math.log10(Math.max(mag, 1e-6));
    const clamped = Math.max(DB_BOT, Math.min(DB_TOP, db));
    return CH - MY - ((clamped - DB_BOT) / (DB_TOP - DB_BOT)) * (CH - 2 * MY);
  }

  // ── Q ↔ canvas-Y (log scale) ──────────────────────────────
  // High Q (narrow peak) → small Y (near top)
  // Low  Q (wide peak)   → large Y (near bottom)
  function qToY(qv) {
    const norm = (Math.log(qv) - Math.log(QMIN)) / (Math.log(QMAX) - Math.log(QMIN));
    return MY + (1 - norm) * (CH - 2 * MY);
  }
  function yToQ(y) {
    const norm = Math.max(0, Math.min(1, 1 - (y - MY) / (CH - 2 * MY)));
    return Math.exp(norm * (Math.log(QMAX) - Math.log(QMIN)) + Math.log(QMIN));
  }

  // ── Filter transfer functions (linear magnitude) ──────────
  function bpMag(f) {
    const r = f / freq;
    return 1 / Math.sqrt(1 + q * q * (r - 1 / r) * (r - 1 / r));
  }
  function hpMag(f) {
    // 2nd-order Butterworth high-pass
    const r = f / LOCUT_FC;
    return (r * r) / Math.sqrt(1 + r * r * r * r);
  }
  function lpMag(f) {
    // 2nd-order low-pass with Q-dependent resonance
    const r = f / HICUT_FC;
    return 1 / Math.sqrt((1 - r*r)*(1 - r*r) + (r/q)*(r/q));
  }

  // ── Draw ──────────────────────────────────────────────────
  function draw() {
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    // Background
    ctx.fillStyle = '#1a1a1a';
    ctx.fillRect(0, 0, CW, CH);

    // Subtle grid — vertical lines at decade/half-decade freqs
    ctx.lineWidth = 0.5;
    ctx.strokeStyle = 'rgba(255,255,255,0.06)';
    for (const gf of [50, 100, 200, 500, 1000, 2000, 5000, 10000]) {
      const gx = freqToX(gf);
      ctx.beginPath();
      ctx.moveTo(gx, 0);
      ctx.lineTo(gx, CH);
      ctx.stroke();
    }
    // Horizontal centre line (≈ −21 dB reference)
    ctx.strokeStyle = 'rgba(255,255,255,0.04)';
    ctx.beginPath();
    ctx.moveTo(0, CH / 2);
    ctx.lineTo(CW, CH / 2);
    ctx.stroke();

    // ── Frequency response curve ─────────────────────────────
    // Build points
    const pts = [];
    for (let px = 0; px <= CW; px++) {
      const f = xToFreq(px);
      if (f < FMIN || f > FMAX) continue;
      let mag;
      if      (!loCut && !hiCut)  mag = bpMag(f);
      else if ( loCut && !hiCut)  mag = hpMag(f);
      else if (!loCut &&  hiCut)  mag = lpMag(f);
      else                        mag = bpMag(f) * hpMag(f) * lpMag(f);
      pts.push([px, magToY(mag)]);
    }

    // Fill
    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.lineTo(CW, CH);
    ctx.lineTo(0, CH);
    ctx.closePath();
    ctx.fillStyle = 'rgba(0,200,180,0.09)';
    ctx.fill();

    // Stroke
    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.strokeStyle = '#00c8b4';
    ctx.lineWidth = 1.8;
    ctx.lineJoin = 'round';
    ctx.stroke();

    // ── Draggable handle ─────────────────────────────────────
    const hx = freqToX(freq);
    const hy = qToY(q);             // moves with Q: high Q = near top

    // Dashed vertical from top to handle
    ctx.beginPath();
    ctx.moveTo(hx, MY);
    ctx.lineTo(hx, hy - 6.5);
    ctx.strokeStyle = 'rgba(245,166,35,0.13)';
    ctx.lineWidth = 0.8;
    ctx.setLineDash([2, 3]);
    ctx.stroke();
    // Dashed vertical from handle to bottom
    ctx.beginPath();
    ctx.moveTo(hx, hy + 6.5);
    ctx.lineTo(hx, CH - MY);
    ctx.strokeStyle = 'rgba(245,166,35,0.13)';
    ctx.stroke();
    ctx.setLineDash([]);

    // Handle circle
    ctx.beginPath();
    ctx.arc(hx, hy, 5.5, 0, Math.PI * 2);
    ctx.fillStyle = dragging ? '#ffb940' : '#f5a623';
    ctx.fill();
    // Subtle rim
    ctx.beginPath();
    ctx.arc(hx, hy, 5.5, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(255,220,100,0.4)';
    ctx.lineWidth = 1;
    ctx.stroke();

    // Freq label near handle (bottom of canvas)
    const freqLabel = freq >= 1000
      ? (freq / 1000).toFixed(freq % 1000 === 0 ? 0 : 1) + 'k'
      : Math.round(freq).toString();
    ctx.font = '8px "DM Sans", sans-serif';
    ctx.fillStyle = 'rgba(245,166,35,0.5)';
    ctx.textAlign = 'center';
    ctx.fillText(freqLabel, hx, CH - 1);
  }

  // Reactive redraw
  $: if (canvas) { freq; q; loCut; hiCut; draw(); }
  onMount(() => draw());

  // ── Drag logic ────────────────────────────────────────────
  let dragging = false;
  let dragStartClientX, dragStartClientY;
  let dragStartFreq, dragStartQ;

  function onCanvasMouseDown(e) {
    const rect = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;
    const mx = (e.clientX - rect.left) * scaleX;
    const my = (e.clientY - rect.top) * scaleY;
    const hx = freqToX(freq);
    const hy = qToY(q);             // hit-test against actual handle position

    if (Math.hypot(mx - hx, my - hy) < 16) {
      dragging = true;
      dragStartClientX = e.clientX;
      dragStartClientY = e.clientY;
      dragStartFreq = freq;
      dragStartQ = q;
      window.addEventListener('mousemove', onDragMove);
      window.addEventListener('mouseup', onDragUp);
    }
  }

  function onDragMove(e) {
    if (!dragging) return;
    const rect = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;

    // Horizontal → frequency (log scale via canvas coords)
    const startCanvasX = freqToX(dragStartFreq);
    const dx = (e.clientX - dragStartClientX) * scaleX;
    freq = Math.min(FMAX, Math.max(FMIN, xToFreq(startCanvasX + dx)));

    // Vertical → Q (direct canvas-Y → Q mapping)
    const startCanvasY = qToY(dragStartQ);
    const dy = (e.clientY - dragStartClientY) * scaleY;
    q = yToQ(startCanvasY + dy);

    draw();
    dispatch('change', { freq, q, loCut, hiCut });
  }

  function onDragUp() {
    dragging = false;
    draw(); // redraw to update handle color
    window.removeEventListener('mousemove', onDragMove);
    window.removeEventListener('mouseup', onDragUp);
  }

  // ── Toggle handlers ───────────────────────────────────────
  function toggleLoCut() {
    loCut = !loCut;
    dispatch('change', { freq, q, loCut, hiCut });
  }
  function toggleHiCut() {
    hiCut = !hiCut;
    dispatch('change', { freq, q, loCut, hiCut });
  }

  // ── NeuNumber handlers ────────────────────────────────────
  function onFreqChange(e) {
    freq = e.detail.value;
    dispatch('change', { freq, q, loCut, hiCut });
  }
  function onQChange(e) {
    q = e.detail.value;
    dispatch('change', { freq, q, loCut, hiCut });
  }
</script>

<div>

  <!-- ── Header ─────────────────────────────────────────── -->
  <div class="fg-header">
    <div class="toggles">
      <NeuButton
        label="Lo Cut"
        active={loCut}
        on:change={e => { loCut = e.detail.active; dispatch('change', { freq, q, loCut, hiCut }); }}
      />
      <NeuButton
        label="Hi Cut"
        active={hiCut}
        on:change={e => { hiCut = e.detail.active; dispatch('change', { freq, q, loCut, hiCut }); }}
      />
    </div>
  </div>

  <!-- ── Canvas ─────────────────────────────────────────── -->
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

  <!-- ── NeuNumber controls ─────────────────────────────── -->
  <div class="controls-row" style="align-items:center;">
    <NeuNumber
      label="FREQ"
      value={freq}
      min={20}
      max={20000}
      step={1}
      unit=" Hz"
      decimals={0}
      on:change={onFreqChange}
    />
    <NeuNumber
      label="Q"
      value={q}
      min={0.1}
      max={9}
      step={0.01}
      decimals={2}
      on:change={onQChange}
    />
  </div>

</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:wght@200;300;400&display=swap');

  /* ── Outer wrapper ────────────────────────────────────── */
  .fg-wrap {
    display: flex;
    flex-direction: column;
    gap: 8px;
    background: #ede6da;
    border-radius: 14px;
    box-shadow:
      8px 8px 20px rgba(150, 130, 100, 0.3),
      -6px -6px 16px rgba(255, 252, 244, 0.9);
    padding: 10px 10px 12px;
    user-select: none;
  }

  /* ── Header row ───────────────────────────────────────── */
  .fg-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0 2px;
  }

  .fg-title {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.55rem;
    font-weight: 300;
    letter-spacing: 0.22em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.65);
  }

  /* ── Toggle buttons (NeuButton row) ──────────────────── */
  .toggles {
    display: flex;
    gap: 12px;
  }

  /* ── Canvas shell (inset neumorphic recess) ───────────── */
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

  .graph-canvas.dragging {
    cursor: grabbing;
  }

  /* ── NeuNumber row ────────────────────────────────────── */
  .controls-row {
    display: flex;
    justify-content: center;
    gap: 20px;
    padding-top: 2px;
  }
</style>
