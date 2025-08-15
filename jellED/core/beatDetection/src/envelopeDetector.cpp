#include "envelopeDetector.h"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace jellED {

EnvelopeDetector::EnvelopeDetector(uint32_t sample_rate, uint8_t downsample_factor)
: sample_rate{sample_rate},
  downsample_factor{downsample_factor},
  current_envelope{0.0},
  sample_counter{0}
{
    this->attack_coeff = 1.0 - std::exp(-1.0 / (ATTACK_TIME * this->sample_rate));
    this->release_coeff = 1.0 - std::exp(-1.0 / (RELEASE_TIME * this->sample_rate));
}

EnvelopeDetector::~EnvelopeDetector() {
}

double EnvelopeDetector::apply(const double sample) {
    this->sample_counter++;
    
    // Only process every downsample_factor samples
    if (this->sample_counter % this->downsample_factor != 0) {
        return -1.0;
    }
    
    // Process the sample normally (equivalent to calling envelope_peak_decay_realtime)
    double sample_abs = std::abs(sample);

    if (sample_abs > this->current_envelope) {
        // Attack phase - fast response to sudden increases
        this->current_envelope += this->attack_coeff * (sample_abs - this->current_envelope);
    } else {
        // Release phase - gradual decay
        this->current_envelope += this->release_coeff * (sample_abs - this->current_envelope);
    }
    
    return this->current_envelope;
}

} // end namespace jellED
