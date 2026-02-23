<script lang="ts">
    import { store, setPlayingValue } from "../state/store";
    import type { ParameterId } from "../types/parameters";
    import { audioPlayer } from "../controls/audioPlayer";


    export let parameterId: ParameterId;

    $: state = $store[parameterId];
    $: isPlaying = state?.value > 0.5;
    $: name = state?.name ?? "Player";
    

</script>

<div class="flex flex-col items-center group">
    <button
        use:audioPlayer={parameterId}
        class="relative w-48 h-48 flex-shrink-0 cursor-pointer active:cursor-pointer flex items-center justify-center bg-none border-none p-0"
    >
        <svg
            viewBox="0 0 200 200"
            class="w-full h-full absolute inset-0"
            preserveAspectRatio="xMidYMid meet"
            style="filter: drop-shadow(0 20px 25px -5px rgba(0, 0, 0, 0.5))"
        >
            <defs>
                <linearGradient
                    id="buttonGradient"
                    x1="0%"
                    y1="0%"
                    x2="100%"
                    y2="100%"
                >
                    <stop offset="0%" stop-color="#333" />
                    <stop offset="50%" stop-color="#222" />
                    <stop offset="100%" stop-color="#111" />
                </linearGradient>

                <filter id="glow">
                    <feGaussianBlur stdDeviation="3" result="coloredBlur" />
                    <feMerge>
                        <feMergeNode in="coloredBlur" />
                        <feMergeNode in="SourceGraphic" />
                    </feMerge>
                </filter>
            </defs>

            <!-- Background circle -->
            <circle
                cx="100"
                cy="100"
                r="70"
                fill="url(#buttonGradient)"
                stroke="#333"
                stroke-width="2"
            />

            <!-- Inner highlight circle -->
            <circle
                cx="100"
                cy="100"
                r="60"
                fill="transparent"
                stroke="white"
                stroke-width="0.5"
                stroke-opacity="0.1"
            />

            <!-- Play/Pause icon group -->
            <g class="transition-opacity duration-200">
                {#if isPlaying}
                    <!-- Pause icon -->
                    <rect
                        x="75"
                        y="65"
                        width="8"
                        height="70"
                        rx="2"
                        fill="#3b82f6"
                        filter="url(#glow)"
                        class="transition-all duration-200"
                    />
                    <rect
                        x="117"
                        y="65"
                        width="8"
                        height="70"
                        rx="2"
                        fill="#3b82f6"
                        filter="url(#glow)"
                        class="transition-all duration-200"
                    />
                {:else}
                    <!-- Play icon (triangle) -->
                    <polygon
                        points="75,65 75,135 130,100"
                        fill="#3b82f6"
                        filter="url(#glow)"
                        class="transition-all duration-200"
                    />
                {/if}
            </g>
        </svg>

        <div
            class="absolute inset-0 flex flex-col items-center justify-center pointer-events-none tracking-tighter"
        >
            <span
                class="text-[10px] font-bold uppercase text-zinc-600 tracking-[0.2em]"
                >{name}</span
            >
        </div>
    </button>
</div>

<style>
    .group {
        user-select: none;
    }

    button {
        outline: none;
    }

    button:active {
        filter: brightness(0.95);
    }
</style>
