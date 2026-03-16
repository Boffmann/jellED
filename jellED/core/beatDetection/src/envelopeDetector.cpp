#include "include/envelopeDetector.h"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace jellED {

EnvelopeDetector::EnvelopeDetector(uint32_t sample_rate, uint8_t downsample_factor, float envelope_gain,
                                   float attack_time, float release_time)
: sample_rate{sample_rate},
  downsample_factor{downsample_factor},
  current_envelope{0.0f},
  sample_counter{0},
  envelope_gain{envelope_gain}
{
    this->attack_coeff = 1.0f - std::exp(-1.0f / (attack_time * this->sample_rate));
    this->release_coeff = 1.0f - std::exp(-1.0f / (release_time * this->sample_rate));
}

EnvelopeDetector::~EnvelopeDetector() {
}

float EnvelopeDetector::apply(const float sample) {
    // 1. Rectify the sample
    float sample_abs = std::abs(sample);

    // 2. Smooth with single pole lowpass filter (attack/release envelope follower)
    if (sample_abs > current_envelope) {
        current_envelope += attack_coeff * (sample_abs - current_envelope);
    } else {
        current_envelope += release_coeff * (sample_abs - current_envelope);
    }

    // 3. Downsample
    sample_counter++;
    if (sample_counter % downsample_factor != 0) {
        return -1.0f;
    }

    // 4. Return the envelope value directly (scaled)
    // Peak detection will find actual peaks in the envelope,
    // which correspond to peaks in the filtered audio signal
    return current_envelope * envelope_gain;
}

void EnvelopeDetector::setTimings(float attackTime, float releaseTime) {
    attack_coeff = 1.0f - std::exp(-1.0f / (attackTime * sample_rate));
    release_coeff = 1.0f - std::exp(-1.0f / (releaseTime * sample_rate));
}

} // end namespace jellED
