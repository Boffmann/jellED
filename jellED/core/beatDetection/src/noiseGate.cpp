#include "include/noiseGate.h"

#include <cmath>

namespace jellED {

NoiseGate::NoiseGate(uint32_t sample_rate, float threshold, float release_time_seconds)
    : threshold_(threshold),
      gate_gain_(0.0f),
      release_coeff_(std::exp(-1.0f / (release_time_seconds * sample_rate)))
{}

float NoiseGate::apply(float sample) {
    float abs_sample = std::abs(sample);

    if (abs_sample >= threshold_) {
        // Instant open (zero attack)
        gate_gain_ = 1.0f;
    } else {
        // Smooth release to avoid clicks
        gate_gain_ *= release_coeff_;
    }

    return sample * gate_gain_;
}

void NoiseGate::setThreshold(float threshold) {
    threshold_ = threshold;
}

float NoiseGate::getThreshold() const {
    return threshold_;
}

} // namespace jellED
