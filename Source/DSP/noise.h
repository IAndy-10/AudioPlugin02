#pragma once

namespace plugin01 {

class NoiseGenerator
{
public:
    explicit NoiseGenerator(float sampleRate);

    void frequency(float hz);       // Add this - for consistency with other oscillators
    void color(int type);           // 0=White, 1=Pink, 2=Brown
    void lowpass(float freq);       // Low-pass filter cutoff
    void highpass(float freq);      // High-pass filter cutoff
    void fadeIn(float timeMs);      // Fade-in time in milliseconds
    void reset();                   // Reset fade envelope
    float process();

private:
    float sr_;
    float frequency_;               // Add this member variable
    float level_;                   // Linear amplitude
    float fadeTime_;                // Fade-in time in samples
    float fadeCounter_;             // Current fade position
    int colorType_;                 // 0=White, 1=Pink, 2=Brown
    
    // Filter coefficients
    float lpCoeff_;
    float hpCoeff_;
    
    // Filter states
    float lpState_;
    float hpState_;
    
    // Pink noise state (using Paul Kellet's method)
    float pink_b0_, pink_b1_, pink_b2_, pink_b3_, pink_b4_, pink_b5_, pink_b6_;
    
    // Brown noise state
    float brown_last_;
    
    // Random number generation
    unsigned int seed_;
    
    float generateWhite();
    float generatePink();
    float generateBrown();
    float random();                 // Returns value between -1 and 1
};

}