#pragma once
#include <cmath>

namespace plugin01 {

class FMSaw {
public:
    FMSaw(float sampleRate);

    void frequency(float hz);
    float process(); 
    void filter(float amount);

private:
    float sr_;
    float w_;        // normalized frequency
    float phase_;    // 0..1
    float osc_;      // feedback state
    float z1_;       // HF filter delay

    // precalculated per frequency
    float scaling_;
    float dc_;
    float norm_;
    float filter_ = 1.0f;

};

} 
