#include "include/envelopeDetector.h"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace jellED {

EnvelopeDetector::EnvelopeDetector(uint32_t sample_rate, uint8_t downsample_factor, double novelty_gain)
: sample_rate{sample_rate},
  downsample_factor{downsample_factor},
  current_envelope{0.0},
  previous_envelope{0.0},
  sample_counter{0},
  novelty_gain{novelty_gain}
{
    this->attack_coeff = 1.0 - std::exp(-1.0 / (ATTACK_TIME * this->sample_rate));
    this->release_coeff = 1.0 - std::exp(-1.0 / (RELEASE_TIME * this->sample_rate));
}

EnvelopeDetector::~EnvelopeDetector() {
}

double EnvelopeDetector::apply(const double sample) {
    // 1. Rectify the sample
    double sample_abs = std::abs(sample);

    // 2. Smooth with single pole lowpass filter
    // double alpha = std::exp(-2.0 * M_PI * ENVELOPE_CUTOFF / (sample_rate));
    // this->current_envelope = alpha * current_envelope + (1.0 - alpha) * sample_abs;

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

    // return this->current_envelope;

    // 4. Novelty (positive derivative)
    double novelty = std::max(0.0, this->current_envelope - this->previous_envelope);
    this->previous_envelope = this->current_envelope;

    return novelty * novelty_gain;
}

} // end namespace jellED
