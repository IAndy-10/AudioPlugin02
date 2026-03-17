<script>
  import { createEventDispatcher } from 'svelte';

  export let label = 'VALUE';
  export let value = 0;
  export let min = 0;
  export let max = 100;
  export let step = 1;
  export let unit = '';
  export let decimals = 1;

  const dispatch = createEventDispatcher();

  let editing = false;
  let inputEl;
  let rawInput = '';

  $: displayVal = value.toFixed(decimals);

  function clamp(v) {
    return Math.min(max, Math.max(min, v));
  }

  function emit() {
    dispatch('change', { value });
  }

  // ── Edit mode ──────────────────────────────────────────────
  function startEdit() {
    editing = true;
    rawInput = displayVal;
    // Focus the hidden input on next tick
    setTimeout(() => inputEl && inputEl.focus(), 0);
  }

  function commitEdit() {
    const parsed = parseFloat(rawInput);
    if (!isNaN(parsed)) {
      value = clamp(parsed);
      emit();
    }
    editing = false;
  }

  function cancelEdit() {
    editing = false;
  }

  function onInputKey(e) {
    if (e.key === 'Enter') { e.preventDefault(); commitEdit(); }
    else if (e.key === 'Escape') { e.preventDefault(); cancelEdit(); }
    else if (e.key === 'ArrowUp')   { e.preventDefault(); rawInput = clamp(parseFloat(rawInput || value) + step).toFixed(decimals); }
    else if (e.key === 'ArrowDown') { e.preventDefault(); rawInput = clamp(parseFloat(rawInput || value) - step).toFixed(decimals); }
  }

  // ── Display mode keyboard ──────────────────────────────────
  function onDisplayKey(e) {
    if (e.key === 'ArrowUp') {
      e.preventDefault();
      value = clamp(value + step);
      emit();
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      value = clamp(value - step);
      emit();
    } else if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault();
      startEdit();
    } else if ((e.key >= '0' && e.key <= '9') || e.key === '-' || e.key === '.') {
      // Start typing a new value directly
      rawInput = e.key;
      editing = true;
      setTimeout(() => {
        if (inputEl) {
          inputEl.focus();
          inputEl.setSelectionRange(rawInput.length, rawInput.length);
        }
      }, 0);
    }
  }

  // ── Scroll ─────────────────────────────────────────────────
  function onWheel(e) {
    e.preventDefault();
    const dir = e.deltaY < 0 ? 1 : -1;
    value = clamp(value + dir * step);
    emit();
  }
</script>

<!-- svelte-ignore a11y-no-noninteractive-tabindex -->
<div class="wrap">
  <div class="label">{label}</div>

  <div
    class="display"
    class:editing
    tabindex="0"
    role="spinbutton"
    aria-valuenow={value}
    aria-valuemin={min}
    aria-valuemax={max}
    on:click={startEdit}
    on:keydown={onDisplayKey}
    on:wheel={onWheel}
  >
    {#if editing}
      <input
        bind:this={inputEl}
        bind:value={rawInput}
        class="raw-input"
        type="text"
        inputmode="decimal"
        on:keydown={onInputKey}
        on:blur={commitEdit}
      />
    {:else}
      <span class="val-text">{displayVal}{unit}</span>
    {/if}
  </div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:wght@200;300;400&display=swap');

  .wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 5px;
    user-select: none;
  }

  .label {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.55rem;
    font-weight: 300;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.65);
  }

  .display {
    min-width: 72px;
    height: 28px;
    padding: 0 8px;
    border-radius: 8px;
    background: #e8e0d4;
    box-shadow:
      inset 3px 3px 7px rgba(150, 130, 100, 0.28),
      inset -2px -2px 5px rgba(255, 250, 240, 0.75);
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: text;
    outline: none;
    transition: box-shadow 0.12s;
  }

  .display:focus {
    box-shadow:
      inset 3px 3px 7px rgba(150, 130, 100, 0.38),
      inset -2px -2px 5px rgba(255, 250, 240, 0.85),
      0 0 0 1.5px rgba(160, 130, 90, 0.3);
  }

  .display.editing {
    box-shadow:
      inset 4px 4px 8px rgba(150, 130, 100, 0.35),
      inset -2px -2px 5px rgba(255, 250, 240, 0.8);
  }

  .val-text {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.68rem;
    font-weight: 300;
    color: rgba(120, 95, 65, 0.85);
    letter-spacing: 0.05em;
    pointer-events: none;
  }

  .raw-input {
    width: 100%;
    background: transparent;
    border: none;
    outline: none;
    font-family: 'DM Sans', sans-serif;
    font-size: 0.68rem;
    font-weight: 300;
    color: rgba(100, 75, 45, 0.95);
    letter-spacing: 0.05em;
    text-align: center;
    padding: 0;
  }
</style>
