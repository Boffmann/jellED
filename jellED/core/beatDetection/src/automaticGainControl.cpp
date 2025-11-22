#include "include/automaticGainControl.h"

#include <iostream>
#include <cmath>
#include <algorithm>

namespace jellED {


AutomaticGainControl::AutomaticGainControl(uint32_t sample_rate, double target_level)
    : peak_limiter_gain(1.0),
        peak_envelope(0.0),
        section_gain(20.0),  // Start with reasonable default
        section_rms(0.0),
        section_counter(0),
        local_peak(0.01),
        sample_rate(sample_rate),
        target_level(target_level)
{
    // Stage 1: Very fast attack (10ms), medium release (1s)
    peak_attack_coeff = 1.0 - std::exp(-1.0 / (0.01 * sample_rate));
    peak_release_coeff = 1.0 - std::exp(-1.0 / (1.0 * sample_rate));
    
    // Stage 2: Update every 2 seconds (track song sections)
    section_alpha = 1.0 - std::exp(-1.0 / (2.0 * sample_rate));
    
    // Stage 3: Fast local tracking (100ms)
    local_alpha = 1.0 - std::exp(-1.0 / (0.1 * sample_rate));
}

double AutomaticGainControl::apply(double sample) {
    // STAGE 1: Peak Limiting (prevent hard clipping)
    double sample_abs = std::abs(sample);
    
    if (sample_abs > peak_envelope) {
        peak_envelope += peak_attack_coeff * (sample_abs - peak_envelope);
    } else {
        peak_envelope += peak_release_coeff * (sample_abs - peak_envelope);
    }
    
    // Reduce gain if approaching clipping after all processing
    double predicted_output = sample_abs * section_gain * peak_limiter_gain;
    if (predicted_output > 0.95) {
        // Fast reduction to prevent clipping
        peak_limiter_gain *= 0.95;
    } else if (predicted_output < 0.5) {
        // Slow recovery
        peak_limiter_gain = std::min(1.0, peak_limiter_gain * 1.001);
    }
    
    // STAGE 2: Section-level adaptation (every 2 seconds)
    section_rms = section_alpha * sample_abs + (1.0 - section_alpha) * section_rms;
    section_counter++;
    
    if (section_counter >= sample_rate * 2) {  // Every 2 seconds
        if (section_rms > 0.001) {  // Only if signal present
            double desired_gain = target_level / section_rms;
            
            // Limit gain range for safety
            desired_gain = std::max(1.0, std::min(100.0, desired_gain));
            
            // Smooth gain changes (avoid sudden jumps)
            section_gain = 0.7 * section_gain + 0.3 * desired_gain;
            
            std::cout << "AGC Update: RMS=" << section_rms 
                        << ", Section Gain=" << section_gain 
                        << ", Limiter=" << peak_limiter_gain << std::endl;
        }
        section_counter = 0;
    }
    
    
    // STAGE 3: Local peak tracking (beat preservation)
    // Don't let loud beats get crushed relative to their section
    local_peak = local_alpha * sample_abs + (1.0 - local_alpha) * local_peak;
    
    double beat_preservation = 1.0;
    if (sample_abs > local_peak * 2.0) {
        // This is a beat - boost it slightly to maintain punch
        beat_preservation = 1.2;
    }
    
    // APPLY ALL STAGES
    return sample * section_gain * peak_limiter_gain * beat_preservation;
}

// Diagnostics
double AutomaticGainControl::getSectionGain() const { return section_gain; }
double AutomaticGainControl::getPeakLimiterGain() const { return peak_limiter_gain; }
double AutomaticGainControl::getCurrentRMS() const { return section_rms; }

} // end namespace jellED
