#include "include/downsampler.h"
#include <cmath>
#include <algorithm>

namespace jellED {

Downsampler::Downsampler(int downsample_factor, double sample_rate_in, double fc_cut_frac)
    : downsample_factor(downsample_factor),
      sample_rate_in(sample_rate_in)
{
    if (downsample_factor < 1) {
        this->downsample_factor = 1;
    }
    this->sample_rate_out = sample_rate_in / downsample_factor;
    double nyq_new = sample_rate_out * 0.5;
    double fc = std::max(1.0, fc_cut_frac * nyq_new);
    alpha = std::exp(-2.0 * M_PI * fc / sample_rate_in);
    z = 0.0;
}

void Downsampler::downsample(const AudioBuffer& in, AudioBuffer& out) {
    int out_index = 0;
    for (int i = 0; i < in.num_samples; i++) {
        out.buffer[i / this->downsample_factor] = in.buffer[i];
        // 1-pole lowpass smoothing (simple IIR)
        this->z = this->alpha * this->z + (1.0 - this->alpha) * in.buffer[i];
        if (i % this->downsample_factor == 0) {
            out.buffer[out_index] = this->z;
            out_index++;
        }
    }
    out.num_samples = out_index;
    // out.bytes_read = samples_to_read * bytes_per_sample;
    out.samplingRate = this->sample_rate_out;
  
}

} // end namespace jellED