#pragma once
#include <random>


namespace plugin01 {

class DelayLine {
public:
    DelayLine(int maxSize = 48000);
    
    void setSize(int newSize);
    void write(float sample);
    float read() const;
    void clear();
    int getSize() const;  // NEW: Added getter for size
    
private:
    std::vector<float> buffer;
    int size = 0;
    int writeIndex = 0;
};

class MeanFilter {
public:
    MeanFilter();
    
    float process(float input);
    void reset();
    
private:
    float previousSample;
};



class KarplusStrong {
public:
    KarplusStrong(float sampleRate = 44100.0f);
    
    void setSampleRate(float newSampleRate);
    void frequency(float hertz);
    void pluck();
    float operator()();
    float getFrequency() const;
    
private:
    float sampleRate;
    float currentFrequency = 440.0f;
    
    plugin01::DelayLine delay;
    plugin01::MeanFilter filter;
    
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
};

}