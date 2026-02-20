#include "AudioApp.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"
#include "revmodel.hpp" // Freeverb
using namespace gam;

namespace ky {

template <typename T> T lerp(T a, T b, float t) {
  return a * (1.0f - t) + b * t;
}

template <typename T> class Array : public std::vector<T> {
public:
  T lookup(float samples) {
    size_t to_the_left = static_cast<size_t>(std::floor(samples));
    size_t to_the_right =
        (to_the_left == (this->size() - 1u)) ? 0u : to_the_left + 1u;
    float t = samples - (float)to_the_left;
    return lerp(this->operator[](to_the_left), this->operator[](to_the_right),
                t);
  }
  T phasor(float t) { return lookup(this->size() * t); }
};

} // namespace ky

class MyApp : public gam::AudioApp {
public:
  // Short-time Fourier transform
  STFT stft{
      2048,     // Window size
      2048 / 4, // Hop size; number of samples between transforms
      0,        // Pad size; number of zero-valued samples appended to window
      HANN,     // Window type: BARTLETT, BLACKMAN, BLACKMAN_HARRIS,
                //		HAMMING, HANN, WELCH, NYQUIST, or RECTANGLE
      COMPLEX   // Format of frequency samples:
                //		COMPLEX, MAG_PHASE, or MAG_FREQ
  };

  SamplePlayer<> player;
  STFT::bin_type data[2048];

  // Freeverb instance
  revmodel reverb;

  MyApp() {
    player.load("this-is-a-test1.wav");

    // Reverb settings (all values between 0.0 and 1.0)
    reverb.setroomsize(1.0f); // 0 = small room, 1 = large hall
    reverb.setdamp(0.5f);     // 0 = bright reverb, 1 = dark/muffled
    reverb.setwet(0.8f);      // how much reverb signal
    reverb.setdry(0.2f);      // how much dry (original) signal
    reverb.setwidth(1.0f);    // stereo width
  }

  void onAudio(AudioIOData &io) {
    while (io()) {
      float s = player();

      if (stft(s)) {
        // make a copy of the bins and zero the output bins
        for (int k = 0; k < stft.numBins(); ++k) {
          data[k] = stft.bin(k);
          stft.bin(k) *= 0.0;
        }
        // stretch the spectrum; 2x up-shift
        for (int k = 0; k < stft.numBins() / 2; ++k) {
          stft.bin(k * 1.8) = data[k];
        }
      }

      s = stft();

      // Apply Freeverb — separate left and right output
      float outL = 0.0f, outR = 0.0f;
      reverb.processreplace(&s, &s, &outL, &outR, 1, 1);

      io.out(0) = outL;
      io.out(1) = outR;
    }
  }
};

int main() { MyApp().start(); }