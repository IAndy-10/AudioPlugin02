<script lang="ts">
    import { store } from "../state/store";
    import type { ParameterId } from "../types/parameters";
    import { knobControl } from "../controls/knob";

    export let parameterId: ParameterId;

    $: state = $store[parameterId];
    $: value = state.value;
    $: name = state.name;

    const minAngle = -135;
    const maxAngle = 135;
    const angleRange = maxAngle - minAngle;

    $: currentAngle = minAngle + (value / 100) * angleRange;
    $: arcPath = describeArc(100, 100, 82, minAngle, currentAngle);

    function polarToCartesian(
        centerX: number,
        centerY: number,
        radius: number,
        angleInDegrees: number,
    ) {
        const angleInRadians = ((angleInDegrees - 90) * Math.PI) / 180.0;
        return {
            x: centerX + radius * Math.cos(angleInRadians),
            y: centerY + radius * Math.sin(angleInRadians),
        };
    }

    function describeArc(
        x: number,
        y: number,
        radius: number,
        startAngle: number,
        endAngle: number,
    ) {
        const start = polarToCartesian(x, y, radius, endAngle);
        const end = polarToCartesian(x, y, radius, startAngle);
        const largeArcFlag = endAngle - startAngle <= 180 ? "0" : "1";
        return [
            "M",
            start.x,
            start.y,
            "A",
            radius,
            radius,
            0,
            largeArcFlag,
            0,
            end.x,
            end.y,
        ].join(" ");
    }
</script>

<div class="flex flex-col items-center group">
    <div
        use:knobControl={parameterId}
        class="relative w-48 h-48 flex-shrink-0 cursor-none active:cursor-none flex items-center justify-center"
    >
        <svg
            viewBox="0 0 200 200"
            class="w-full h-full absolute inset-0"
            preserveAspectRatio="xMidYMid meet"
            style="filter: drop-shadow(0 20px 25px -5px rgba(0, 0, 0, 0.5))"
        >
            <defs>
                <linearGradient
                    id="knobGradient"
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

            <path
                d={describeArc(100, 100, 82, minAngle, maxAngle)}
                fill="none"
                stroke="#000"
                stroke-width="12"
                stroke-linecap="round"
                class="opacity-40"
            />

            <path
                d={arcPath}
                fill="none"
                stroke="url(#lineGradient)"
                stroke-width="8"
                stroke-linecap="round"
                filter="url(#glow)"
            />

            <linearGradient id="lineGradient" x1="0%" y1="0%" x2="100%" y2="0%">
                <stop offset="0%" stop-color="#06b6d4" />
                <stop offset="100%" stop-color="#3b82f6" />
            </linearGradient>

            <circle
                cx="100"
                cy="100"
                r="70"
                fill="url(#knobGradient)"
                stroke="#333"
                stroke-width="2"
            />

            <circle
                cx="100"
                cy="100"
                r="60"
                fill="transparent"
                stroke="white"
                stroke-width="0.5"
                stroke-opacity="0.1"
            />

            <g
                style="transform: rotate({currentAngle}deg); transform-origin: 100px 100px;"
                class="transition-transform duration-75"
            >
                <rect
                    x="98"
                    y="35"
                    width="4"
                    height="15"
                    rx="2"
                    fill="#3b82f6"
                    filter="url(#glow)"
                />
            </g>
        </svg>

        <div
            class="absolute inset-0 flex flex-col items-center justify-center pointer-events-none tracking-tighter"
        >
            <span
                class="text-3xl font-black text-white italic tabular-nums w-20 text-center"
                >{Math.round(value)}</span
            >
            <span
                class="text-[10px] font-bold uppercase text-zinc-600 tracking-[0.2em]"
                >{name}</span
            >
        </div>
    </div>
</div>

<style>
    .group {
        user-select: none;
    }
</style>