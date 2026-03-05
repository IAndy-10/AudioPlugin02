#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include "DelayLine.h"
#include "FeedbackMatrix.h"
#include "CrossoverFilter.h"
#include "DiffusionNetwork.h"
#include "LFO.h"

// 8-line Feedback Delay Network reverb with Hadamard feedback matrix,
// frequency-dependent decay, diffusion, and per-line modulation.
// Real-time safe: no allocations in process().
class FDNReverb {
public:
    static constexpr int N = 8; // Must match FeedbackMatrix::N

    void prepare(double sampleRate);

    void setDecayMs(float ms);
    void setDamping(float d);       // 0-1
    void setDiffusion(float d);     // 0-1
    void setSize(float s);          // 0-1: scales delay lengths
    void setCrossoverFreq(float hz);
    void setFeedback(float fb);     // 0-1 (additional feedback scaling)
    void setFrozen(bool frozen);
    void setReverbMode(int mode);   // 0=High, 1=Low

    void process(juce::AudioBuffer<float>& buffer);
    void reset();

private:
    // Mutually prime base delay lengths (samples at 44100 Hz, 12-23 ms)
    static constexpr int BASE_DELAYS[N] = { 557, 617, 683, 743, 809, 877, 947, 1019 };

    void updateDecayGains();
    void updateModulation();

    double sr = 44100.0;

    std::array<DelayLine, N>       delayLines;
    std::array<CrossoverFilter, N> dampFilters;
    DiffusionNetwork               diffusion;
    std::array<LFO, N>             modLFOs;

    std::array<float, N> decayGain  {};
    std::array<int, N>   delayLens  {};
    std::array<float, N> modDepth   {};

    float decayMs    = 1500.0f;
    float dampAmount = 0.5f;
    float diffAmount = 0.6f;
    float sizeScale  = 0.5f;
    float crossFreq  = 3000.0f;
    float feedback   = 0.8f;
    bool  frozen     = false;
    int   mode       = 0; // 0=High, 1=Low

    static constexpr float MOD_DEPTH_BASE = 2.0f; // samples of modulation
    static constexpr float MOD_FREQ_BASE  = 0.31f; // Hz (different per line)
};
