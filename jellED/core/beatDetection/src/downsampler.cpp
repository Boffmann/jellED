#include "include/downsampler.h"
#include <cmath>
#include <algorithm>

namespace jellED {

Downsampler::Downsampler(float sample_rate_in, int downsample_factor, float fc_cut_frac)
    : downsample_factor(downsample_factor)
    , sampleCounter_(0)
{
    if (downsample_factor < 1) {
        this->downsample_factor = 1;
    }
    this->sample_rate_out = sample_rate_in / downsample_factor;
    // Note: alpha/z no longer used in simple decimation mode
    float nyq_new = sample_rate_out * 0.5f;
    float fc = std::max(1.0f, fc_cut_frac * nyq_new);
    alpha = std::exp(-2.0f * static_cast<float>(M_PI) * fc / sample_rate_in);
    z = 0.0f;
}

void Downsampler::downsample(const AudioBuffer& in, AudioBuffer& out) {
    // FAST: Simple decimation - just take every Nth sample
    // No anti-aliasing filter - for beat detection this is fine
    // since we care about transients, not perfect frequency response
    
    const int factor = this->downsample_factor;
    int out_index = 0;
    
    // Calculate starting offset to maintain phase alignment
    int start_offset = (factor - (this->sampleCounter_ % factor)) % factor;
    
    // Only iterate through samples we actually output
    for (size_t i = start_offset; i < in.num_samples; i += factor) {
        out.buffer[out_index++] = in.buffer[i];
    }
    
    this->sampleCounter_ += in.num_samples;
    out.num_samples = out_index;
    out.samplingRate = this->sample_rate_out;
}

} // end namespace jellED
