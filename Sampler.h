#ifndef SAMPLER_H
#define SAMPLER_H

#include <Bela.h>
#include <libraries/AudioFile/AudioFile.h>
#include <vector>
#include "AR.h"

class Sampler {
public:
    Sampler(const std::vector<std::string>& filenames, float sampleRate);
    void trigger();
    void stop();
    void setAttackTime(float attackTime);
    void setReleaseTime(float releaseTime);
    float process();

private:
    std::vector<std::vector<float>> sampleBuffers_;
    int readPointer_;
    int sampleSelector_;
    AR amplitudeAR_;
    float amplitude_;
    float sampleRate_;
};

#endif // SAMPLER_H