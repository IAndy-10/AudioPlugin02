#include "saw.h"
#include <algorithm>


namespace plugin01 {

static constexpr float A0 = 2.5f;   // HF compensation
static constexpr float A1 = -1.5f;
static constexpr float PI2 = 6.283185307179586f;

FMSaw::FMSaw(float sampleRate)
: sr_(sampleRate),
  w_(0.0f),
  phase_(0.0f),
  osc_(0.0f),
  z1_(0.0f),
  scaling_(0.0f),
  dc_(0.0f),
  norm_(1.0f)
{
}

void FMSaw::frequency(float hz)
{
    w_ = hz / sr_;

    float n = 0.5f - w_;
    scaling_ = filter_ * 13.0f * n * n * n * n;      // β = 13(0.5 − ω)^4
    dc_      = 0.376f - 0.752f * w_;       // DC compensation
    norm_    = 1.0f - 2.0f * w_;           // amplitude normalization
}

float FMSaw::process()

{
    // phase accumulator
    phase_ += 2.0f * w_;
    if (phase_ >= 1.0f)
        phase_ -= 2.0f;

    // self-FM sine
    float s = std::sinf(PI2 * (phase_ + osc_ * scaling_));

    // averaging (stability fix from the paper)
    osc_ = 0.5f * (osc_ + s);

    // HF compensation filter
    float out = A0 * osc_ + A1 * z1_;
    z1_ = osc_;

    // DC + normalization
    return (out + dc_) * norm_;
}

void FMSaw::filter(float amount)
{
    filter_ = std::clamp(amount, 0.0f, 1.0f);
}


}
