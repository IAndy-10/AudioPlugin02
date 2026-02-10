#include "KarplusStrong.h"


namespace plugin01 {

DelayLine::DelayLine(int maxSize) 
    : buffer(maxSize, 0.0f), writeIndex(0), size(maxSize) {}  // FIX: Initialize size to maxSize

void DelayLine::setSize(int newSize) {
    if (newSize <= buffer.size() && newSize > 0) {
        size = newSize;
        writeIndex = 0;  // Reset write position when changing size
    }
}

void DelayLine::write(float sample) {
    if (size > 0) {  // Safety check
        buffer[writeIndex] = sample;
        writeIndex = (writeIndex + 1) % size;
    }
}

float DelayLine::read() const {
    if (size > 0) {
        return buffer[writeIndex];
    }
    return 0.0f;
}

void DelayLine::clear() {
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    writeIndex = 0;
}

int DelayLine::getSize() const {  
    return size;
}

MeanFilter::MeanFilter() : previousSample(0.0f) {}

float MeanFilter::process(float input) {
    float output = 0.5f * (input + previousSample);
    previousSample = input;
    return output;
}

void MeanFilter::reset() {
    previousSample = 0.0f;
}

KarplusStrong::KarplusStrong(float sampleRate) 
    : sampleRate(sampleRate),
      delay(static_cast<int>(sampleRate)),
      rng(std::random_device{}()),
      dist(-1.0f, 1.0f)
{
    frequency(440.0f);
}

void KarplusStrong::setSampleRate(float newSampleRate) {
    sampleRate = newSampleRate;
}

void KarplusStrong::frequency(float hertz) {
    if (hertz > 0.0f && hertz < sampleRate / 2.0f) {
        int delayLength = static_cast<int>(sampleRate / hertz);
        if (delayLength < 2) delayLength = 2;
        
        delay.setSize(delayLength);
        currentFrequency = hertz;
    }
}

void KarplusStrong::pluck() {
    delay.clear();
    
    int delaySize = delay.getSize();
    for (int i = 0; i < delaySize; ++i) {
        float noiseSample = dist(rng);
        delay.write(noiseSample);
    }
    filter.reset();
}

float KarplusStrong::operator()() {
    float delaySample = delay.read();
    float filtered = filter.process(delaySample);
    filtered *= 0.996f;  // Damping factor
    delay.write(filtered);
    return filtered;
}

float KarplusStrong::getFrequency() const {
    return currentFrequency;
}

}