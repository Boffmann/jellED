#ifndef __ENVELOPE_DETECTOR_JELLED_H__
#define __ENVELOPE_DETECTOR_JELLED_H__

#include "filterStage.h"

namespace jellED {

class EnvelopeDetector : public FilterStage {
public:
    static constexpr double DEFAULT_ATTACK_TIME = 0.005;   // 5ms attack - fast enough to catch transients
    static constexpr double DEFAULT_RELEASE_TIME = 0.050;  // 50ms release - smooth decay between beats

    EnvelopeDetector(uint32_t sample_rate, uint8_t downsample_factor = 1, double envelope_gain = 1.0,
                     double attack_time = DEFAULT_ATTACK_TIME, double release_time = DEFAULT_RELEASE_TIME);
    ~EnvelopeDetector();
    double apply(const double sample);

    void setTimings(double attackTime, double releaseTime);

private:
    double attack_coeff;
    double release_coeff;
    
    uint32_t sample_rate;
    uint8_t downsample_factor;
    double current_envelope;
    uint8_t sample_counter;
    double envelope_gain;
};

} // namespace jellED

#endif
