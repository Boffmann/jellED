#include "include/peakdetection.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

namespace {

float compute_coeff(float sample_rate, float time_constant) {
    if (sample_rate <= 0.0f || time_constant <= 0.0f) {
        return 1.0f;
    }
    return 1.0f - std::exp(-1.0f / (time_constant * sample_rate));
}

float compute_decay_coeff(float sample_rate, float time_constant) {
    if (sample_rate <= 0.0f || time_constant <= 0.0f) {
        return 0.0f;
    }
    return std::exp(-1.0f / (time_constant * sample_rate));
}

} // anonymous namespace

namespace jellED {

PeakDetector::PeakDetector(const PeakDetectorConfig& config, uint32_t sample_rate)
    : absolute_min_threshold(config.absoluteMinThreshold),
      threshold_rel(config.thresholdRel),
      max_bpm(config.maxBpm),
      onset_ratio(config.onsetRatio),
      min_relative_threshold_factor(config.minRelativeThresholdFactor),
      rising_threshold_scale(config.risingThresholdScale),
      falling_threshold_scale(config.fallingThresholdScale),
      threshold_baseline(config.absoluteMinThreshold),
      envelope(config.absoluteMinThreshold),
      baseline_attack_coeff(1.0f),
      baseline_release_coeff(1.0f),
      dynamic_threshold_rel(config.thresholdRel),
      min_dynamic_threshold_rel(config.thresholdRel * config.minRelativeThresholdFactor),
      threshold_relax_coeff(0.0f),
      prev_env(0.0f),
      is_rising(false),
      last_peak_time(0.0f),
      recent_min(0.0f),
      recent_min_decay_coeff(0.0f),
      last_threshold_(config.absoluteMinThreshold),
      sample_rate_(sample_rate)
{
    const float sr = static_cast<float>(sample_rate);

    baseline_attack_coeff = compute_coeff(sr, config.baselineAttackTime);
    baseline_release_coeff = compute_coeff(sr, config.baselineReleaseTime);
    threshold_relax_coeff = compute_decay_coeff(sr, config.thresholdRelaxTime);

    // Recent minimum tracker: decay using the same time constant as baseline release
    recent_min_decay_coeff = compute_decay_coeff(sr, config.baselineReleaseTime);
}

float PeakDetector::update_envelope(float sample) {
    float clamped_sample = std::max(0.0f, sample);
    float coeff = (clamped_sample > envelope) ? baseline_attack_coeff : baseline_release_coeff;
    envelope += coeff * (clamped_sample - envelope);
    threshold_baseline = envelope;
    return threshold_baseline;
}

bool PeakDetector::is_peak(float envelope_sample, float current_time) {
    float baseline = update_envelope(envelope_sample);

    if (threshold_relax_coeff > 0.0f) {
        dynamic_threshold_rel = min_dynamic_threshold_rel +
            (dynamic_threshold_rel - min_dynamic_threshold_rel) * threshold_relax_coeff;
    }

    float threshold = std::max(this->absolute_min_threshold, baseline * (1.0f + dynamic_threshold_rel));
    last_threshold_ = threshold;

    float threshold_high = threshold * rising_threshold_scale;
    float threshold_low = threshold * falling_threshold_scale;

    if (envelope_sample < recent_min || recent_min == 0.0f) {
        recent_min = envelope_sample;
    } else {
        recent_min = recent_min * recent_min_decay_coeff + envelope_sample * (1.0f - recent_min_decay_coeff);
    }

    bool isPeak = false;

    if (envelope_sample > threshold_high) {
        if (envelope_sample > prev_env) {
            is_rising = true;
        } else if (envelope_sample < prev_env && is_rising) {
            float min_beat_interval = 60.0f / max_bpm;
            if (current_time - last_peak_time >= min_beat_interval) {
            float effective_min = std::max(recent_min, absolute_min_threshold * 0.5f);
            float onset_threshold = effective_min * onset_ratio;

            if (prev_env > onset_threshold && prev_env > absolute_min_threshold) {
                isPeak = true;
                last_peak_time = current_time;
                dynamic_threshold_rel = threshold_rel;
            }
            }
            is_rising = false;
            recent_min = envelope_sample;
        }
    } else if (envelope_sample < threshold_low) {
        is_rising = false;
    }

    prev_env = envelope_sample;

    return isPeak;
}

void PeakDetector::setAbsoluteMinThreshold(float threshold) {
    absolute_min_threshold = threshold;
}

void PeakDetector::setThresholdRel(float newThresholdRel) {
    threshold_rel = newThresholdRel;
    min_dynamic_threshold_rel = threshold_rel * min_relative_threshold_factor;
    if (dynamic_threshold_rel < min_dynamic_threshold_rel) {
        dynamic_threshold_rel = min_dynamic_threshold_rel;
    }
}

void PeakDetector::setMaxBpm(float bpm) {
    max_bpm = bpm;
}

void PeakDetector::setOnsetRatio(float ratio) {
    onset_ratio = ratio;
}

void PeakDetector::setTimingParams(float baselineAttackTime, float baselineReleaseTime,
                                    float thresholdRelaxTime) {
    const float sr = static_cast<float>(sample_rate_);
    baseline_attack_coeff = compute_coeff(sr, baselineAttackTime);
    baseline_release_coeff = compute_coeff(sr, baselineReleaseTime);
    threshold_relax_coeff = compute_decay_coeff(sr, thresholdRelaxTime);
    recent_min_decay_coeff = compute_decay_coeff(sr, baselineReleaseTime);
}

void PeakDetector::setHysteresisScales(float risingScale, float fallingScale) {
    rising_threshold_scale = risingScale;
    falling_threshold_scale = fallingScale;
}

void PeakDetector::setMinRelativeThresholdFactor(float factor) {
    min_relative_threshold_factor = factor;
    min_dynamic_threshold_rel = threshold_rel * factor;
    if (dynamic_threshold_rel < min_dynamic_threshold_rel) {
        dynamic_threshold_rel = min_dynamic_threshold_rel;
    }
}

} // end namespace jellED
