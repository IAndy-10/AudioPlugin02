<script>
  import { createEventDispatcher } from 'svelte';

  export let label = 'GAIN';
  export let value = 0.5;
  export let min = 0;
  export let max = 100;
  export let unit = '';

  const dispatch = createEventDispatcher();

  let dragging = false;
  let startY = 0;
  let startVal = value;

  const MIN_ANGLE = -135;
  const MAX_ANGLE = 135;

  $: angle = MIN_ANGLE + value * (MAX_ANGLE - MIN_ANGLE);
  $: displayVal = (min + value * (max - min)).toFixed(1);

  function onMouseDown(e) {
    dragging = true;
    startY = e.clientY;
    startVal = value;
    window.addEventListener('mousemove', onMove);
    window.addEventListener('mouseup', onUp);
  }
  function onMove(e) {
    if (!dragging) return;
    value = Math.min(1, Math.max(0, startVal + (startY - e.clientY) / 120));
    dispatch('change', { value });
  }
  function onUp() {
    dragging = false;
    dispatch('change', { value });
    window.removeEventListener('mousemove', onMove);
    window.removeEventListener('mouseup', onUp);
  }
  function onWheel(e) {
    e.preventDefault();
    value = Math.min(1, Math.max(0, value - e.deltaY * 0.001));
    dispatch('change', { value });
  }
</script>

<div class="wrap">
  <div
    class="knob"
    class:pressed={dragging}
    on:mousedown={onMouseDown}
    on:wheel={onWheel}
    role="slider"
    tabindex="0"
    aria-valuenow={displayVal}
  >
    <div class="inner" style="transform: rotate({angle}deg)">
      <div class="marker"></div>
    </div>
    <svg class="arc" viewBox="0 0 64 64">
      <circle cx="32" cy="32" r="26"
        fill="none"
        stroke="rgba(180,165,145,0.18)"
        stroke-width="2"
        stroke-dasharray="130 41"
        stroke-dashoffset="10"
        stroke-linecap="round"
        transform="rotate(135 32 32)"
      />
      <circle cx="32" cy="32" r="26"
        fill="none"
        stroke="rgba(160,130,90,0.35)"
        stroke-width="2"
        stroke-dasharray="{value * 130} 171"
        stroke-dashoffset="10"
        stroke-linecap="round"
        transform="rotate(135 32 32)"
      />
    </svg>
  </div>
  <div class="label">{label}</div>
  <div class="val">{displayVal}{unit}</div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:wght@200;300;400&display=swap');

  .wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 6px;
    user-select: none;
  }

  .knob {
    position: relative;
    width: 64px;
    height: 64px;
    border-radius: 50%;
    background: #e8e0d4;
    box-shadow:
      6px 6px 14px rgba(160, 140, 110, 0.45),
      -5px -5px 12px rgba(255, 252, 245, 0.9);
    cursor: ns-resize;
    transition: box-shadow 0.15s;
    display: flex;
    align-items: center;
    justify-content: center;
  }

  .knob.pressed {
    box-shadow:
      2px 2px 6px rgba(160, 140, 110, 0.5),
      -2px -2px 5px rgba(255, 252, 245, 0.8),
      inset 3px 3px 8px rgba(150, 130, 100, 0.25),
      inset -2px -2px 6px rgba(255, 252, 240, 0.7);
  }

  .arc {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    pointer-events: none;
  }

  .inner {
    width: 40px;
    height: 40px;
    border-radius: 50%;
    background: radial-gradient(circle at 35% 30%, #f5efe6, #ddd4c4);
    box-shadow:
      3px 3px 8px rgba(150,130,100,0.4),
      -2px -2px 6px rgba(255,250,240,0.85);
    display: flex;
    align-items: flex-start;
    justify-content: center;
    padding-top: 5px;
  }

  .marker {
    width: 3px;
    height: 8px;
    border-radius: 2px;
    background: rgba(140, 110, 70, 0.6);
  }

  .label {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.7);
  }

  .val {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 200;
    color: rgba(150, 120, 80, 0.65);
    letter-spacing: 0.08em;
  }
</style>
