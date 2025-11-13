#ifndef __ENVELOPE_DETECTOR_JELLED_H__
#define __ENVELOPE_DETECTOR_JELLED_H__

#include "filterStage.h"
#include "ringbuffer.h"

namespace jellED {

class EnvelopeDetector : public FilterStage {
private:
    static constexpr double ATTACK_TIME = 0.005;
    static constexpr double RELEASE_TIME = 0.05;
    static constexpr double ENVELOPE_CUTOFF = 6.0;

    double attack_coeff;
    double release_coeff;
    
    uint32_t sample_rate;
    uint8_t downsample_factor;
    double current_envelope;
    double previous_envelope;
    uint8_t sample_counter;

public:
    EnvelopeDetector(uint32_t sample_rate, uint8_t downsample_factor = 8);
    ~EnvelopeDetector();
    double apply(const double sample);
};

} // namespace jellED

#endif
