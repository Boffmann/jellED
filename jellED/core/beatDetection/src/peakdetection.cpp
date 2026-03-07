#include "include/peakdetection.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

namespace {

double compute_coeff(double sample_rate, double time_constant) {
    if (sample_rate <= 0.0 || time_constant <= 0.0) {
        return 1.0;
    }
    return 1.0 - std::exp(-1.0 / (time_constant * sample_rate));
}

double compute_decay_coeff(double sample_rate, double time_constant) {
    if (sample_rate <= 0.0 || time_constant <= 0.0) {
        return 0.0;
    }
    return std::exp(-1.0 / (time_constant * sample_rate));
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
      baseline_attack_coeff(1.0),
      baseline_release_coeff(1.0),
      dynamic_threshold_rel(config.thresholdRel),
      min_dynamic_threshold_rel(config.thresholdRel * config.minRelativeThresholdFactor),
      threshold_relax_coeff(0.0),
      prev_env(0.0),
      is_rising(false),
      last_peak_time(0.0),
      recent_min(0.0),
      recent_min_decay_coeff(0.0),
      last_threshold_(config.absoluteMinThreshold),
      sample_rate_(sample_rate)
{
    const double sr = static_cast<double>(sample_rate);

    baseline_attack_coeff = compute_coeff(sr, config.baselineAttackTime);
    baseline_release_coeff = compute_coeff(sr, config.baselineReleaseTime);
    threshold_relax_coeff = compute_decay_coeff(sr, config.thresholdRelaxTime);

    // Recent minimum tracker: decay using the same time constant as baseline release
    recent_min_decay_coeff = compute_decay_coeff(sr, config.baselineReleaseTime);
}

double PeakDetector::update_envelope(double sample) {
    double clamped_sample = std::max(0.0, sample);
    double coeff = (clamped_sample > envelope) ? baseline_attack_coeff : baseline_release_coeff;
    envelope += coeff * (clamped_sample - envelope);
    threshold_baseline = envelope;
    return threshold_baseline;
}

bool PeakDetector::is_peak(double envelope_sample, double current_time) {
    double baseline = update_envelope(envelope_sample);

    if (threshold_relax_coeff > 0.0) {
        dynamic_threshold_rel = min_dynamic_threshold_rel +
            (dynamic_threshold_rel - min_dynamic_threshold_rel) * threshold_relax_coeff;
    }

    double threshold = std::max(this->absolute_min_threshold, baseline * (1.0 + dynamic_threshold_rel));
    last_threshold_ = threshold;

    double threshold_high = threshold * rising_threshold_scale;
    double threshold_low = threshold * falling_threshold_scale;

    if (envelope_sample < recent_min || recent_min == 0.0) {
        recent_min = envelope_sample;
    } else {
        recent_min = recent_min * recent_min_decay_coeff + envelope_sample * (1.0 - recent_min_decay_coeff);
    }

    bool isPeak = false;

    if (envelope_sample > threshold_high) {
        if (envelope_sample > prev_env) {
            is_rising = true;
        } else if (envelope_sample < prev_env && is_rising) {
            double min_beat_interval = 60.0 / max_bpm;
            if (current_time - last_peak_time >= min_beat_interval) {
            double effective_min = std::max(recent_min, absolute_min_threshold * 0.5);
            double onset_threshold = effective_min * onset_ratio;
            
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

void PeakDetector::setAbsoluteMinThreshold(double threshold) {
    absolute_min_threshold = threshold;
}

void PeakDetector::setThresholdRel(double newThresholdRel) {
    threshold_rel = newThresholdRel;
    min_dynamic_threshold_rel = threshold_rel * min_relative_threshold_factor;
    if (dynamic_threshold_rel < min_dynamic_threshold_rel) {
        dynamic_threshold_rel = min_dynamic_threshold_rel;
    }
}

void PeakDetector::setMaxBpm(double bpm) {
    max_bpm = bpm;
}

void PeakDetector::setOnsetRatio(double ratio) {
    onset_ratio = ratio;
}

void PeakDetector::setTimingParams(double baselineAttackTime, double baselineReleaseTime,
                                    double thresholdRelaxTime) {
    const double sr = static_cast<double>(sample_rate_);
    baseline_attack_coeff = compute_coeff(sr, baselineAttackTime);
    baseline_release_coeff = compute_coeff(sr, baselineReleaseTime);
    threshold_relax_coeff = compute_decay_coeff(sr, thresholdRelaxTime);
    recent_min_decay_coeff = compute_decay_coeff(sr, baselineReleaseTime);
}

void PeakDetector::setHysteresisScales(double risingScale, double fallingScale) {
    rising_threshold_scale = risingScale;
    falling_threshold_scale = fallingScale;
}

void PeakDetector::setMinRelativeThresholdFactor(double factor) {
    min_relative_threshold_factor = factor;
    min_dynamic_threshold_rel = threshold_rel * factor;
    if (dynamic_threshold_rel < min_dynamic_threshold_rel) {
        dynamic_threshold_rel = min_dynamic_threshold_rel;
    }
}

} // end namespace jellED
