<script>
  import { createEventDispatcher, onMount } from 'svelte';
  import NeuNumber from './NeuNumber.svelte';
  import NeuButton from './NeuButton.svelte';

  // Props — all in display units (Hz / Q), normalization done by parent
  export let loCutFreq = 200;    // Hz (50–18000)
  export let hiCutFreq = 8000;   // Hz (50–18000)
  export let q         = 1.0;    // Q factor (0.5–9.0)
  export let loCut     = false;  // high-pass enable
  export let hiCut     = false;  // low-pass enable

  const dispatch = createEventDispatcher();

  // ── Canvas constants ───────────────────────────────────────
  const CW = 300, CH = 110;
  const MX = 6,  MY = 8;
  const FMIN = 50, FMAX = 18000;
  const QMIN = 0.5, QMAX = 9.0;
  const DB_TOP = 6, DB_BOT = -48;

  let canvas;
  let dragging = false;
  let dragStartClientX, dragStartClientY;
  let dragStartActiveFreq, dragStartQ;

  // The active handle tracks whichever band is being edited
  $: activeFreq = (loCut && !hiCut) ? loCutFreq : hiCutFreq;

  // ── Frequency ↔ X (log scale) ─────────────────────────────
  function freqToX(f) {
    return MX + (CW - 2 * MX) * Math.log10(f / FMIN) / Math.log10(FMAX / FMIN);
  }
  function xToFreq(x) {
    const norm = (x - MX) / (CW - 2 * MX);
    return FMIN * Math.pow(FMAX / FMIN, norm);
  }

  // ── Magnitude → Y (dB scale) ──────────────────────────────
  function magToY(mag) {
    const db = 20 * Math.log10(Math.max(mag, 1e-6));
    const clamped = Math.max(DB_BOT, Math.min(DB_TOP, db));
    return CH - MY - ((clamped - DB_BOT) / (DB_TOP - DB_BOT)) * (CH - 2 * MY);
  }

  // ── Q ↔ Y (log scale) ─────────────────────────────────────
  function qToY(qv) {
    const norm = (Math.log(qv) - Math.log(QMIN)) / (Math.log(QMAX) - Math.log(QMIN));
    return MY + (1 - norm) * (CH - 2 * MY);
  }
  function yToQ(y) {
    const norm = Math.max(0, Math.min(1, 1 - (y - MY) / (CH - 2 * MY)));
    return Math.exp(norm * (Math.log(QMAX) - Math.log(QMIN)) + Math.log(QMIN));
  }

  // ── Filter transfer functions ──────────────────────────────
  function hpMag(f) {
    const r = f / loCutFreq;
    return (r * r) / Math.sqrt(1 + r * r * r * r);
  }
  function lpMag(f) {
    const r = f / hiCutFreq;
    return 1 / Math.sqrt((1 - r * r) * (1 - r * r) + (r / q) * (r / q));
  }

  // ── Draw ──────────────────────────────────────────────────
  function draw() {
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    // Background
    ctx.fillStyle = '#1a1a1a';
    ctx.fillRect(0, 0, CW, CH);

    // Grid
    ctx.lineWidth = 0.5;
    ctx.strokeStyle = 'rgba(255,255,255,0.06)';
    for (const gf of [50, 100, 200, 500, 1000, 2000, 5000, 10000]) {
      const gx = freqToX(gf);
      ctx.beginPath(); ctx.moveTo(gx, 0); ctx.lineTo(gx, CH); ctx.stroke();
    }
    ctx.strokeStyle = 'rgba(255,255,255,0.04)';
    ctx.beginPath(); ctx.moveTo(0, CH / 2); ctx.lineTo(CW, CH / 2); ctx.stroke();

    // Magnitude curve
    const pts = [];
    for (let px = 0; px <= CW; px++) {
      const f = xToFreq(px);
      if (f < FMIN || f > FMAX) continue;
      let mag;
      if      (!loCut && !hiCut) mag = 1.0;
      else if ( loCut && !hiCut) mag = hpMag(f);
      else if (!loCut &&  hiCut) mag = lpMag(f);
      else                       mag = hpMag(f) * lpMag(f);
      pts.push([px, magToY(mag)]);
    }

    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.lineTo(CW, CH); ctx.lineTo(0, CH); ctx.closePath();
    ctx.fillStyle = 'rgba(0,200,180,0.09)';
    ctx.fill();

    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.strokeStyle = '#00c8b4';
    ctx.lineWidth = 1.8; ctx.lineJoin = 'round'; ctx.stroke();

    // ── Active handle ─────────────────────────────────────
    const hx = freqToX(activeFreq);
    const hy = qToY(q);

    ctx.beginPath(); ctx.moveTo(hx, MY); ctx.lineTo(hx, hy - 6.5);
    ctx.strokeStyle = 'rgba(245,166,35,0.13)'; ctx.lineWidth = 0.8;
    ctx.setLineDash([2, 3]); ctx.stroke();
    ctx.beginPath(); ctx.moveTo(hx, hy + 6.5); ctx.lineTo(hx, CH - MY);
    ctx.stroke(); ctx.setLineDash([]);

    ctx.beginPath(); ctx.arc(hx, hy, 5.5, 0, Math.PI * 2);
    ctx.fillStyle = dragging ? '#ffb940' : '#f5a623'; ctx.fill();
    ctx.beginPath(); ctx.arc(hx, hy, 5.5, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(255,220,100,0.4)'; ctx.lineWidth = 1; ctx.stroke();

    // Freq label at bottom of handle
    const freqLabel = activeFreq >= 1000
      ? (activeFreq / 1000).toFixed(activeFreq % 1000 === 0 ? 0 : 1) + 'k'
      : Math.round(activeFreq).toString();
    ctx.font = '8px "DM Sans", sans-serif';
    ctx.fillStyle = 'rgba(245,166,35,0.5)';
    ctx.textAlign = 'center'; ctx.textBaseline = 'alphabetic';
    ctx.fillText(freqLabel, hx, CH - 1);
  }

  $: if (canvas) { loCutFreq; hiCutFreq; q; loCut; hiCut; draw(); }
  onMount(() => draw());

  // ── Drag ──────────────────────────────────────────────────
  function onCanvasMouseDown(e) {
    const rect = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;
    const mx = (e.clientX - rect.left) * scaleX;
    const my = (e.clientY - rect.top)  * scaleY;
    const hx = freqToX(activeFreq);
    const hy = qToY(q);

    if (Math.hypot(mx - hx, my - hy) < 16) {
      dragging = true;
      dragStartClientX   = e.clientX;
      dragStartClientY   = e.clientY;
      dragStartActiveFreq = activeFreq;
      dragStartQ = q;
      window.addEventListener('mousemove', onDragMove);
      window.addEventListener('mouseup',   onDragUp);
    }
  }

  function onDragMove(e) {
    if (!dragging) return;
    const rect = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;

    const dx = (e.clientX - dragStartClientX) * scaleX;
    const newFreq = Math.min(FMAX, Math.max(FMIN,
      xToFreq(freqToX(dragStartActiveFreq) + dx)));

    if (loCut && !hiCut) loCutFreq = newFreq;
    else                 hiCutFreq = newFreq;

    const dy = (e.clientY - dragStartClientY) * scaleY;
    q = yToQ(qToY(dragStartQ) + dy);

    draw();
    dispatch('change', { loCutFreq, hiCutFreq, q, loCut, hiCut });
  }

  function onDragUp() {
    dragging = false;
    draw();
    window.removeEventListener('mousemove', onDragMove);
    window.removeEventListener('mouseup',   onDragUp);
  }

  // ── NeuNumber handlers ─────────────────────────────────────
  function onFreqChange(e) {
    const v = Math.min(FMAX, Math.max(FMIN, e.detail.value));
    if (loCut && !hiCut) loCutFreq = v;
    else                 hiCutFreq = v;
    dispatch('change', { loCutFreq, hiCutFreq, q, loCut, hiCut });
  }
  function onQChange(e) {
    q = e.detail.value;
    dispatch('change', { loCutFreq, hiCutFreq, q, loCut, hiCut });
  }
</script>

<div class="fg-wrap">

  <!-- ── Toggles ────────────────────────────────────────── -->
  <div class="toggles">
    <NeuButton label="Lo Cut" active={loCut}
      on:change={e => { loCut = e.detail.active; dispatch('change', { loCutFreq, hiCutFreq, q, loCut, hiCut }); }}
    />
    <NeuButton label="Hi Cut" active={hiCut}
      on:change={e => { hiCut = e.detail.active; dispatch('change', { loCutFreq, hiCutFreq, q, loCut, hiCut }); }}
    />
  </div>

  <!-- ── Canvas ─────────────────────────────────────────── -->
  <div class="canvas-shell">
    <canvas
      bind:this={canvas}
      width={CW} height={CH}
      class="graph-canvas"
      class:dragging
      on:mousedown={onCanvasMouseDown}
    />
  </div>

  <!-- ── Readouts ───────────────────────────────────────── -->
  <div class="controls-row">
    <NeuNumber
      label="FREQ"
      value={Math.round(activeFreq)}
      min={50} max={18000} step={1} unit=" Hz" decimals={0}
      on:change={onFreqChange}
    />
    <NeuNumber
      label="WIDTH"
      value={+q.toFixed(2)}
      min={0.5} max={9} step={0.01} decimals={2}
      on:change={onQChange}
    />
  </div>

</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:wght@200;300;400&display=swap');

  .fg-wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
    width: 100%;
  }

  .toggles {
    display: flex;
    gap: 12px;
    justify-content: center;
  }

  .canvas-shell {
    width: 100%;
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

  .controls-row {
    display: flex;
    justify-content: center;
    gap: 20px;
  }
</style>
