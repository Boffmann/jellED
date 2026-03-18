#ifndef __DOWNSAMPLER_H__
#define __DOWNSAMPLER_H__

#include "sound/audiobuffer.h"

namespace jellED {

class Downsampler {
private:
    int downsample_factor;
    float sample_rate_out;
    float alpha;
    float z1 = 0.0f;  // first pole state
    float z2 = 0.0f;  // second pole state (cascaded for -12 dB/octave)
    uint32_t sampleCounter_;
public:
    Downsampler(float sample_rate, int downsample_factor, float fc_cut_frac);
    void downsample(const AudioBuffer& in, AudioBuffer& out);
};

} // end namespace jellED

#endif
