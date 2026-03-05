#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

// First-order shelving / one-pole lowpass used per-delay-line in the FDN
// to achieve frequency-dependent decay. Operates on a single channel (mono).
class CrossoverFilter {
public:
    void prepare(double sampleRate);
    void setCutoffFreq(float hz);
    void setDamping(float d); // 0 = no damping, 1 = heavy damping

    // Process a single sample; returns filtered value
    float processSample(float in);

    void reset();

private:
    void updateCoeff();

    double sr = 44100.0;
    float  cutoff  = 3000.0f;
    float  damping = 0.5f;

    double coeff = 0.5; // one-pole LP coefficient
    double z1    = 0.0;
};
