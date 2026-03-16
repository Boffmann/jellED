#ifndef __ENVELOPE_DETECTOR_JELLED_H__
#define __ENVELOPE_DETECTOR_JELLED_H__

#include "filterStage.h"

namespace jellED {

class EnvelopeDetector : public FilterStage {
public:
    static constexpr float DEFAULT_ATTACK_TIME = 0.005f;   // 5ms attack - fast enough to catch transients
    static constexpr float DEFAULT_RELEASE_TIME = 0.050f;  // 50ms release - smooth decay between beats

    EnvelopeDetector(uint32_t sample_rate, uint8_t downsample_factor = 1, float envelope_gain = 1.0f,
                     float attack_time = DEFAULT_ATTACK_TIME, float release_time = DEFAULT_RELEASE_TIME);
    ~EnvelopeDetector();
    float apply(const float sample);

    void setTimings(float attackTime, float releaseTime);

private:
    float attack_coeff;
    float release_coeff;

    uint32_t sample_rate;
    uint8_t downsample_factor;
    float current_envelope;
    uint8_t sample_counter;
    float envelope_gain;
};

} // namespace jellED

#endif
