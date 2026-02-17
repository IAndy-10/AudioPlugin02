<script lang="ts">
    import { store } from "../state/store";
    import type { ParameterId } from "../types/parameters";
    // This is now correctly imported [cite: 9]
    import { dropMenuControl } from "../controls/dropMenu";

    export let parameterId: ParameterId;

    $: state = $store[parameterId];
    $: value = state.value;
    $: name = state.name;

    const choices = ["Saw", "Noise"];
    $: currentChoice = choices[Math.round(value)] || "Unknown";
</script>

<div class="flex flex-col items-center group">
    <div
        use:dropMenuControl={parameterId}
        class="relative w-48 h-12 flex-shrink-0 bg-zinc-800 rounded-lg border border-white/10 flex items-center px-4 cursor-pointer hover:bg-zinc-700 transition-all active:scale-95 shadow-lg"
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

        <div class="ml-auto">
            <svg
                xmlns="http://www.w3.org/2000/svg"
                class="w-4 h-4 text-zinc-500"
                viewBox="0 0 200 200"
                fill="currentColor"
            >
                <path d="M50 80l50 50 50-50H50z" />
            </svg>
        </div>
    </div>

    <div
        class="mt-2 px-3 py-1 bg-black/20 rounded-full border border-white/5 opacity-0 group-hover:opacity-100 transition-opacity"
    >
        <span class="text-[9px] text-zinc-400 font-mono">ID: {parameterId}</span
        >
    </div>
</div>

<style>
    .group {
        user-select: none; /* [cite: 34] */
    }
</style>
