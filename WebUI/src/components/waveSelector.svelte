
<script lang="ts">
    import { store, setWaveformValue } from "../state/store";
    import type { ParameterId } from "../types/parameters";
    import { bridge } from "../bridge/bridge";

    export let parameterId: ParameterId;

    $: state = $store[parameterId];
    $: value = state?.value ?? 0;
    $: name = state?.name ?? "Select";

    const choices = ["Saw", "Noise"];
    $: currentChoice = choices[Math.round(value)] || "Unknown";

    let isOpen = false;

    function toggleMenu() {
        isOpen = !isOpen;
    }

    function selectOption(index: number) {
        // Optimistic update
        setWaveformValue(index);
        // Send to backend
        bridge.sendParameterChange(parameterId, index);
        isOpen = false;
    }

    function handleKeydown(e: KeyboardEvent) {
        if (e.key === "Escape") isOpen = false;
    }

    // Direct directive for clicking outside
    function clickOutside(node: HTMLElement) {
        const handleClick = (event: MouseEvent) => {
            if (
                node &&
                !node.contains(event.target as Node) &&
                !event.defaultPrevented
            ) {
                isOpen = false;
            }
        };

        document.addEventListener("click", handleClick, true);

        return {
            destroy() {
                document.removeEventListener("click", handleClick, true);
            },
        };
    }
</script>

<svelte:window on:keydown={handleKeydown} />

<div class="flex flex-col items-center group relative" use:clickOutside>
    <button
        type="button"
        on:click={toggleMenu}
        aria-haspopup="listbox"
        aria-expanded={isOpen}
        class="relative w-48 h-12 flex-shrink-0 bg-zinc-800 rounded-lg border border-white/10 flex items-center px-4 cursor-pointer hover:bg-zinc-700 transition-all active:scale-95 shadow-lg {isOpen
            ? 'ring-2 ring-cyan-500/50 border-cyan-500/50'
        : ''}"
    >
        <div class="flex flex-col items-start overflow-hidden">
            <span
                class="text-[10px] font-bold uppercase text-zinc-500 tracking-widest"
                >{name}</span
            >
            <span class="text-sm font-black text-white italic tabular-nums"
                >{currentChoice}</span
            >
        </div>

        <div
            class="ml-auto transition-transform duration-200 {isOpen
                ? 'rotate-180'
                : ''}"
        >
            <svg
                xmlns="http://www.w3.org/2000/svg"
                class="w-4 h-4 text-zinc-500"
                viewBox="0 0 200 200"
                fill="currentColor"
            >
                <path d="M50 80l50 50 50-50H50z" />
            </svg>
        </div>
    </button>

    {#if isOpen}
        <div
            class="absolute top-14 left-0 w-48 bg-zinc-900 border border-white/10 rounded-lg shadow-2xl z-50 overflow-hidden py-1 animate-in fade-in zoom-in-95 duration-100"
        >
            {#each choices as choice, i}
                <button
                    class="w-full px-4 py-2 text-left text-sm font-bold transition-colors hover:bg-cyan-500/20 hover:text-cyan-400 {Math.round(
                        value,
                    ) === i
                        ? 'text-cyan-400 bg-white/5'
                        : 'text-zinc-400'}"
                    on:click={() => selectOption(i)}
                >
                    {choice}
                </button>
            {/each}
        </div>
    {/if}

    <div
        class="mt-2 px-3 py-1 bg-black/20 rounded-full border border-white/5 opacity-0 group-hover:opacity-100 transition-opacity"
    >
        <span class="text-[9px] text-zinc-400 font-mono">ID: {parameterId}</span
        >
    </div>
</div>

<style>
    .group {
        user-select: none;
    }

    button {
        appearance: none;
        background: none;
        border: none;
        outline: none;
        cursor: pointer;
    }
</style>
