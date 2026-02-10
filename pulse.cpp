// pulse.cpp

#include "pulse.h"
#include <algorithm>
#include <cmath>

namespace plugin01 {

static constexpr float A0  = 2.5f;
static constexpr float A1  = -1.5f;
static constexpr float PI2 = 6.283185307179586f;

FMPulse::FMPulse(float sampleRate)
: sr_(sampleRate),
  w_(0.0f),
  phase_(0.0f),
  osc1_(0.0f),
  osc2_(0.0f),
  z1_(0.0f),
  scaling_(0.0f),
  dc_(0.0f),
  norm_(1.0f),
  filter_(1.0f),
  pw_(0.5f)
{
}

void FMPulse::frequency(float hz)
{
    w_ = hz / sr_;
    w_ = std::clamp(w_, 0.001f, 0.499f);

    float n = 0.5f - w_;
    scaling_ = filter_ * 13.0f * n * n * n * n;
    scaling_ = std::clamp(scaling_, -10.0f, 10.0f);  // Clamp scaling to prevent infinity
    
    dc_      = 0.376f - 0.752f * w_;
    norm_    = 1.0f - 2.0f * w_;
    norm_    = std::max(norm_, 0.001f);
}

float FMPulse::process()
{
    // phase accumulator
    phase_ += 2.0f * w_;
    if (phase_ >= 1.0f)
        phase_ -= 2.0f;

    // saw 1
    float fm_mod1 = osc1_ * scaling_;
    fm_mod1 = std::clamp(fm_mod1, -1.0f, 1.0f);  // Clamp FM modulation
    float s1 = std::sinf(PI2 * (phase_ + fm_mod1));
    osc1_ = 0.5f * (osc1_ + s1);

    // saw 2 (phase-shifted by pulse width = 0.5)
    float fm_mod2 = osc2_ * scaling_;
    fm_mod2 = std::clamp(fm_mod2, -1.0f, 1.0f);
    float s2 = std::sinf(PI2 * (phase_ + pw_ + fm_mod2));
    osc2_ = 0.5f * (osc2_ + s2);

    // subtract → pulse
    float out = osc1_ - osc2_;

    // HF compensation
    out = A0 * out + A1 * z1_;
    z1_ = out;
    z1_ = std::clamp(z1_, -10.0f, 10.0f);  // Clamp filter state

    float result = (out + dc_) * norm_;
    
    // Only clamp if we detect NaN or inf
    if (!std::isfinite(result))
        return 0.0f;
    
    return result;
}

void FMPulse::filter(float amount)
{
    filter_ = std::clamp(amount, 0.0f, 1.0f);
}

void FMPulse::pulseWidth(float amount)
{
    pw_ = std::clamp(amount, 0.0f, 1.0f);
}


}

