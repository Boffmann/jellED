#include "include/automaticGainControl.h"

#include <iostream>
#include <cmath>
#include <algorithm>

namespace jellED {


AutomaticGainControl::AutomaticGainControl(uint32_t sample_rate, float target_level)
    : peak_limiter_gain(1.0f),
        peak_envelope(0.0f),
        section_gain(1.0f),  // Start at unity gain - let it adapt
        section_rms(0.0f),
        section_counter(0),
        local_peak(0.01f),
        sample_rate(sample_rate),
        target_level(target_level)
{
    // Stage 1: Very fast attack (5ms), slower release (500ms)
    peak_attack_coeff = 1.0f - std::exp(-1.0f / (0.005f * sample_rate));
    peak_release_coeff = 1.0f - std::exp(-1.0f / (0.5f * sample_rate));

    // Stage 2: Update continuously with 1 second time constant
    section_alpha = 1.0f - std::exp(-1.0f / (1.0f * sample_rate));

    // Stage 3: Fast local tracking (100ms)
    local_alpha = 1.0f - std::exp(-1.0f / (0.1f * sample_rate));
}

float AutomaticGainControl::apply(float sample) {
    float sample_abs = std::abs(sample);

    // STAGE 1: Track signal envelope for peak limiting
    if (sample_abs > peak_envelope) {
        peak_envelope += peak_attack_coeff * (sample_abs - peak_envelope);
    } else {
        peak_envelope += peak_release_coeff * (sample_abs - peak_envelope);
    }

    // STAGE 2: Continuous RMS tracking
    section_rms = section_alpha * (sample * sample) + (1.0f - section_alpha) * section_rms;
    float current_rms = std::sqrt(section_rms);

    // Calculate desired gain to reach target level
    if (current_rms > 0.001f) {  // Only if signal present
        float desired_gain = target_level / current_rms;

        // IMPORTANT: Allow gain < 1.0 for attenuation, limit max amplification
        // Range: 0.1x to 20x (attenuate loud signals, amplify quiet ones)
        // 20x = 26dB, enough for quiet USB mics on Raspberry Pi
        desired_gain = std::max(0.1f, std::min(20.0f, desired_gain));

        // Smooth gain changes (avoid sudden jumps)
        // Slower adaptation for more stability
        section_gain = 0.995f * section_gain + 0.005f * desired_gain;
    }

    // STAGE 3: Peak limiter - fast reduction if output would clip
    float predicted_output = sample_abs * section_gain * peak_limiter_gain;
    if (predicted_output > 0.9f) {
        // Fast reduction to prevent clipping
        peak_limiter_gain = std::max(0.1f, peak_limiter_gain * 0.9f);
    } else if (predicted_output < 0.5f && peak_limiter_gain < 1.0f) {
        // Slow recovery back to unity
        peak_limiter_gain = std::min(1.0f, peak_limiter_gain * 1.0005f);
    }

    // STAGE 4: Local peak tracking (optional beat preservation)
    local_peak = local_alpha * sample_abs + (1.0f - local_alpha) * local_peak;

    // Subtle beat preservation - only 5% boost, not 20%
    float beat_preservation = 1.0f;
    if (sample_abs > local_peak * 2.0f) {
        beat_preservation = 1.05f;
    }

    // APPLY ALL STAGES
    float output = sample * section_gain * peak_limiter_gain * beat_preservation;

    // Hard limiter as final safety (soft clip)
    if (std::abs(output) > 1.0f) {
        output = std::tanh(output);  // Soft saturation instead of hard clip
    }

    return output;
}

// Diagnostics
float AutomaticGainControl::getSectionGain() const { return section_gain; }
float AutomaticGainControl::getPeakLimiterGain() const { return peak_limiter_gain; }
float AutomaticGainControl::getCurrentRMS() const { return section_rms; }

} // end namespace jellED
