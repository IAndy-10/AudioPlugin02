<script lang="ts">
    import Knob from "./components/knob.svelte";
    import WaveSelector from "./components/waveSelector.svelte";
</script>

<main
    class="min-h-screen bg-[#121212] flex items-center justify-center p-8 text-white font-sans overflow-hidden selection:bg-cyan-500/30"
>
    <div
        class="relative bg-[#1a1a1a] p-12 rounded-[2rem] border border-white/5 shadow-[0_20px_50px_rgba(0,0,0,0.5),inset_0_1px_1px_rgba(255,255,255,0.1)] overflow-hidden"
    >
        <div
            class="absolute inset-0 pointer-events-none opacity-[0.03] bg-[url('https://www.transparenttextures.com/patterns/brushed-alum.png')]"
        ></div>

        <div class="relative flex flex-col items-center gap-12">
            <header class="text-center space-y-1">
                <h1
                    class="text-4xl font-black tracking-tighter uppercase italic bg-gradient-to-b from-white to-zinc-500 bg-clip-text text-transparent drop-shadow-2xl"
                >
                    Audio Plugin 02
                </h1>
                <div class="flex items-center justify-center gap-2">
                    <span class="h-[1px] w-8 bg-zinc-700"></span>
                    <p
                        class="text-zinc-500 text-[10px] font-bold tracking-[0.4em] uppercase"
                    >
                        Svelte Control Surface
                    </p>
                    <span class="h-[1px] w-8 bg-zinc-700"></span>
                </div>
            </header>

            <div
                class="flex items-center gap-16 p-10 bg-black/40 rounded-3xl border border-white/5 shadow-inner min-w-[500px] justify-center"
            >
                <Knob parameterId="gain" />
                <WaveSelector parameterId="waveform" />
            </div>

            <footer class="flex flex-col items-center gap-4">
                <div class="flex gap-2">
                    <div
                        class="w-1.5 h-1.5 rounded-full bg-cyan-500 shadow-[0_0_8px_#06b6d4] animate-pulse"
                    ></div>
                    <div class="w-1.5 h-1.5 rounded-full bg-zinc-800"></div>
                    <div class="w-1.5 h-1.5 rounded-full bg-zinc-800"></div>
                </div>
            </footer>
        </div>
    </div>
</main>

<style>
    :global(body) {
        margin: 0;
        padding: 0;
        background: #000;
        cursor: default;
    }
</style>
