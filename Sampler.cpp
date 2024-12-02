#include "Sampler.h"

Sampler::Sampler(const std::vector<std::string>& filenames, float sampleRate) 
    : readPointer_(-1), sampleSelector_(0), amplitude_(0.0), sampleRate_(sampleRate) {
    for (const auto& filename : filenames) {
        sampleBuffers_.emplace_back(AudioFileUtilities::loadMono(filename));
    }
    amplitudeAR_.setSampleRate(sampleRate);
}

void Sampler::trigger() {
    sampleSelector_ = std::rand() % sampleBuffers_.size();
    readPointer_ = 0;
    amplitudeAR_.reset();
    amplitudeAR_.trigger();
}

void Sampler::stop() {
    // amplitudeAR_.release();
}

void Sampler::setAttackTime(float attackTime) {
    amplitudeAR_.setAttackTime(attackTime);
}

void Sampler::setReleaseTime(float releaseTime) {
    amplitudeAR_.setReleaseTime(releaseTime);
}

float Sampler::process() {
    float out = 0.0;
    if (readPointer_ != -1) {
        out = 0.5 * sampleBuffers_[sampleSelector_][readPointer_] * amplitudeAR_.process();
        readPointer_++;
        if (readPointer_ >= (int)sampleBuffers_[sampleSelector_].size()) {
            readPointer_ = -1;
        }
    }
    return out;
}