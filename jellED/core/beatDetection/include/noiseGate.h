#ifndef __NOISE_GATE_JELLED_H__
#define __NOISE_GATE_JELLED_H__

#include <stdint.h>

namespace jellED {

class NoiseGate {
private:
    float threshold_;
    float gate_gain_;
    float release_coeff_;

public:
    NoiseGate(uint32_t sample_rate, float threshold, float release_time_seconds = 0.005f);

    float apply(float sample);

    void setThreshold(float threshold);
    float getThreshold() const;
};

} // namespace jellED

#endif
