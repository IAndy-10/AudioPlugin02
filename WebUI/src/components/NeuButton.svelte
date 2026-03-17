<script>
  import { createEventDispatcher } from 'svelte';

  export let label = 'ON';
  export let active = false;
  export let icon = '';

  const dispatch = createEventDispatcher();

  function press() {
    active = !active;
    dispatch('change', { active });
  }
</script>

<div class="wrap">
  <button
    class="btn"
    class:active
    on:click={press}
    aria-pressed={active}
  >
    {#if icon}
      <span class="icon">{icon}</span>
    {/if}
  </button>
  <div class="label">{label}</div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:wght@200;300&display=swap');

  .wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 7px;
    user-select: none;
  }

  .btn {
    width: 52px;
    height: 52px;
    border-radius: 50%;
    border: none;
    background: #e8e0d4;
    box-shadow:
      6px 6px 14px rgba(155,135,105,0.45),
      -5px -5px 12px rgba(255,252,244,0.92);
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    transition: box-shadow 0.14s, background 0.14s;
  }

  /* Momentary press feedback (only when not already toggled on) */
  .btn:active:not(.active) {
    background: #e5ddd1;
  }

  .btn.active {
    background: #e2d9cc;
    box-shadow:
      inset 4px 4px 9px rgba(150,130,100,0.35),
      inset -3px -3px 7px rgba(255,250,240,0.75);
    transition: none;
  }

  .icon {
    font-size: 1rem;
    color: rgba(130,105,70,0.55);
    line-height: 1;
    transition: color 0.14s;
  }

  .btn.active .icon {
    color: rgba(130,90,40,0.75);
  }

  .label {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.7);
  }
</style>
