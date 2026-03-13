#include "include/envelopeDetector.h"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace jellED {

EnvelopeDetector::EnvelopeDetector(uint32_t sample_rate, uint8_t downsample_factor, double envelope_gain,
                                   double attack_time, double release_time)
: sample_rate{sample_rate},
  downsample_factor{downsample_factor},
  current_envelope{0.0},
  sample_counter{0},
  envelope_gain{envelope_gain}
{
    this->attack_coeff = 1.0 - std::exp(-1.0 / (attack_time * this->sample_rate));
    this->release_coeff = 1.0 - std::exp(-1.0 / (release_time * this->sample_rate));
}

EnvelopeDetector::~EnvelopeDetector() {
}

double EnvelopeDetector::apply(const double sample) {
    // 1. Rectify the sample
    double sample_abs = std::abs(sample);

    // 2. Smooth with single pole lowpass filter (attack/release envelope follower)
    if (sample_abs > current_envelope) {
        current_envelope += attack_coeff * (sample_abs - current_envelope);
    } else {
        current_envelope += release_coeff * (sample_abs - current_envelope);
    }

    // 3. Downsample
    sample_counter++;
    if (sample_counter % downsample_factor != 0) {
        return -1.0;
    }

    // 4. Return the envelope value directly (scaled)
    // Peak detection will find actual peaks in the envelope,
    // which correspond to peaks in the filtered audio signal
    return current_envelope * envelope_gain;
}

void EnvelopeDetector::setTimings(double attackTime, double releaseTime) {
    attack_coeff = 1.0 - std::exp(-1.0 / (attackTime * sample_rate));
    release_coeff = 1.0 - std::exp(-1.0 / (releaseTime * sample_rate));
}

} // end namespace jellED
