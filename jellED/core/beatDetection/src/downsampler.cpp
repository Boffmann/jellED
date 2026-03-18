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
    float nyq_new = sample_rate_out * 0.5f;
    float fc = std::max(1.0f, fc_cut_frac * nyq_new);
    alpha = std::exp(-2.0f * static_cast<float>(M_PI) * fc / sample_rate_in);
    z1 = 0.0f;
    z2 = 0.0f;
}

void Downsampler::downsample(const AudioBuffer& in, AudioBuffer& out) {
    const int factor = this->downsample_factor;
    const float a = this->alpha;
    const float one_minus_a = 1.0f - a;
    int out_index = 0;

    // Calculate starting offset to maintain phase alignment
    int start_offset = (factor - (this->sampleCounter_ % factor)) % factor;

    // Apply cascaded 2-pole low-pass to every input sample, then decimate
    for (size_t i = 0; i < in.num_samples; i++) {
        z1 = a * z1 + one_minus_a * in.buffer[i];
        z2 = a * z2 + one_minus_a * z1;

        if (i >= (size_t)start_offset && ((i - start_offset) % factor) == 0) {
            out.buffer[out_index++] = z2;
        }
    }

    this->sampleCounter_ += in.num_samples;
    out.num_samples = out_index;
    out.samplingRate = this->sample_rate_out;
}

} // end namespace jellED
