<script lang="ts">
    import Header from "./components/header.svelte";
    import Knob from "./components/knob.svelte";
    // import WaveSelector from "./components/waveSelector.svelte";
    import AudioPlayer from "./components/audioPlayer.svelte";
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
            <Header />
            <div
                class="flex items-center gap-16 p-10 bg-black/40 rounded-3xl border border-white/5 shadow-inner min-w-[500px] justify-center"
            >
                <AudioPlayer parameterId="playing" />
                <Knob parameterId="gain" />
            </div>

            <footer class="flex flex-col items-center gap-4">
                <div class="flex gap-2">
                    <div
                        class="w-1.5 h-1.5 rounded-full bg-cyan-500 shadow-[0_0_8px_#06b6d4] animate-pulse"
                    ></div>
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
