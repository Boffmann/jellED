#ifndef __ENVELOPE_DETECTOR_JELLED_H__
#define __ENVELOPE_DETECTOR_JELLED_H__

#include "filterStage.h"

namespace jellED {

class EnvelopeDetector : public FilterStage {
private:
    // Attack/release times for envelope follower
    static constexpr double ATTACK_TIME = 0.005;   // 5ms attack - fast enough to catch transients
    static constexpr double RELEASE_TIME = 0.050;  // 50ms release - smooth decay between beats

    double attack_coeff;
    double release_coeff;
    
    uint32_t sample_rate;
    uint8_t downsample_factor;
    double current_envelope;
    uint8_t sample_counter;
    double envelope_gain;

public:
    // envelope_gain scales the output envelope (useful for normalizing across frequency bands)
    EnvelopeDetector(uint32_t sample_rate, uint8_t downsample_factor = 1, double envelope_gain = 1.0);
    ~EnvelopeDetector();
    double apply(const double sample);
};

} // namespace jellED

#endif
