// noise.cpp

#include "noise.h"
#include <algorithm>
#include <cmath>

namespace plugin01 {

static constexpr float PI = 3.14159265358979323846f;

NoiseGenerator::NoiseGenerator(float sampleRate)
: sr_(sampleRate),
  frequency_(440.0f),             // Add this initialization
  fadeTime_(200.0f),
  level_(0.1f),
  fadeCounter_(0.0f),
  colorType_(1),
  lpCoeff_(1.0f),
  hpCoeff_(0.0f),
  lpState_(0.0f),
  hpState_(0.0f),
  pink_b0_(0.0f),
  pink_b1_(0.0f),
  pink_b2_(0.0f),
  pink_b3_(0.0f),
  pink_b4_(0.0f),
  pink_b5_(0.0f),
  pink_b6_(0.0f),
  brown_last_(0.0f),
  seed_(1234567)
{
    lowpass(6000.0f);
    highpass(150.0f);
}

void NoiseGenerator::frequency(float hz)
{
    frequency_ = std::clamp(hz, 20.0f, sr_ * 0.5f);
}


void NoiseGenerator::color(int type)
{
    colorType_ = std::clamp(type, 0, 2);
}

void NoiseGenerator::lowpass(float freq)
{
    freq = std::clamp(freq, 20.0f, sr_ * 0.5f);
    float omega = 2.0f * PI * freq / sr_;
    lpCoeff_ = omega / (1.0f + omega);
}

void NoiseGenerator::highpass(float freq)
{
    freq = std::clamp(freq, 20.0f, sr_ * 0.5f);
    float omega = 2.0f * PI * freq / sr_;
    hpCoeff_ = 1.0f / (1.0f + omega);
}

void NoiseGenerator::fadeIn(float timeMs)
{
    fadeTime_ = (timeMs / 1000.0f) * sr_;
    fadeTime_ = std::max(fadeTime_, 1.0f);
}

void NoiseGenerator::reset()
{
    fadeCounter_ = 0.0f;
    lpState_ = 0.0f;
    hpState_ = 0.0f;
    brown_last_ = 0.0f;
}

float NoiseGenerator::random()
{
    seed_ = seed_ * 1664525 + 1013904223;
    return ((seed_ >> 9) & 0x7FFFFF) / 8388607.5f - 1.0f;
}

float NoiseGenerator::generateWhite()
{
    return random();
}

float NoiseGenerator::generatePink()
{
    float white = random();
    
    pink_b0_ = 0.99886f * pink_b0_ + white * 0.0555179f;
    pink_b1_ = 0.99332f * pink_b1_ + white * 0.0750759f;
    pink_b2_ = 0.96900f * pink_b2_ + white * 0.1538520f;
    pink_b3_ = 0.86650f * pink_b3_ + white * 0.3104856f;
    pink_b4_ = 0.55000f * pink_b4_ + white * 0.5329522f;
    pink_b5_ = -0.7616f * pink_b5_ - white * 0.0168980f;
    
    float pink = pink_b0_ + pink_b1_ + pink_b2_ + pink_b3_ + pink_b4_ + pink_b5_ + pink_b6_ + white * 0.5362f;
    pink_b6_ = white * 0.115926f;
    
    return pink * 0.11f;
}

float NoiseGenerator::generateBrown()
{
    float white = random();
    brown_last_ = (brown_last_ + (0.02f * white)) / 1.02f;
    brown_last_ = std::clamp(brown_last_, -1.0f, 1.0f);
    
    return brown_last_ * 3.5f;
}

float NoiseGenerator::process()
{
    float noise = 0.0f;
    
    switch (colorType_)
    {
        case 0: noise = generateWhite(); break;
        case 1: noise = generatePink(); break;
        case 2: noise = generateBrown(); break;
        default: noise = generatePink(); break;
    }
    
    lpState_ = lpState_ + lpCoeff_ * (noise - lpState_);
    float filtered = lpState_;
    
    float hpOut = filtered - hpState_;
    hpState_ = hpState_ + hpCoeff_ * hpOut;
    filtered = hpOut;
    
    float envelope = 1.0f;
    if (fadeCounter_ < fadeTime_)
    {
        envelope = fadeCounter_ / fadeTime_;
        fadeCounter_ += 1.0f;
    }
    
    float result = filtered * level_ * envelope;
    
    if (!std::isfinite(result))
        return 0.0f;
    
    return std::clamp(result, -1.0f, 1.0f);
}

}