#pragma once

namespace plugin01 {

class FMPulse
{
public:
    explicit FMPulse(float sampleRate);

    void frequency(float hz);
    void filter(float amount); 
    void pulseWidth(float amount);
    float process();

private:
    float sr_;
    float w_;
    float phase_;
    float osc1_;
    float osc2_;
    float z1_;
    float scaling_;
    float dc_;
    float norm_;
    float filter_ = 1.0f; 
    float pw_ = 0.5f;
};

}