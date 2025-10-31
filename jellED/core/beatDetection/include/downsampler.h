#ifndef __DOWNSAMPLER_H__
#define __DOWNSAMPLER_H__

#include "sound/audiobuffer.h"

namespace jellED {

class Downsampler {
private:
    int downsample_factor;      // decimation factor (integer)
    double sample_rate_in;    // input sample rate
    double sample_rate_out;   // output sample rate = fs_in / factor
    double alpha;    // smoothing parameter for 1-pole anti-alias
    double z = 0.0;
public:
    Downsampler(int downsample_factor, double sample_rate, double fc_cut_frac);
    void downsample(const AudioBuffer& in, AudioBuffer& out);
};

} // end namespace jellED

#endif