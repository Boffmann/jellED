#include "include/peakdetection.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

namespace {
constexpr double BASELINE_ATTACK_TIME_SEC = 0.03;
constexpr double BASELINE_RELEASE_TIME_SEC = 0.8;
constexpr double THRESHOLD_RELAX_TIME_SEC = 1.0;
constexpr double MIN_RELATIVE_THRESHOLD_FACTOR = 0.25;
constexpr double RISING_THRESHOLD_SCALE = 1.15;
constexpr double FALLING_THRESHOLD_SCALE = 0.85;
}

namespace jellED {

PeakDetector::PeakDetector(double absolute_min_threshold, double threshold_rel, 
                           double min_peak_distance, double max_bpm, uint32_t sample_rate)
    : absolute_min_threshold(absolute_min_threshold),
      threshold_rel(threshold_rel),
      min_peak_distance(min_peak_distance),
      max_bpm(max_bpm),
      threshold_baseline(absolute_min_threshold),
      envelope(absolute_min_threshold),
      baseline_attack_coeff(1.0),
      baseline_release_coeff(1.0),
      dynamic_threshold_rel(threshold_rel),
      min_dynamic_threshold_rel(threshold_rel * MIN_RELATIVE_THRESHOLD_FACTOR),
      threshold_relax_coeff(0.0),
      prev_env(0.0),
      is_rising(false),
      last_peak_time(-min_peak_distance)
{
    const double sr = static_cast<double>(sample_rate);
    auto compute_coeff = [sr](double time_constant) {
        if (sr <= 0.0 || time_constant <= 0.0) {
            return 1.0;
        }
        return 1.0 - std::exp(-1.0 / (time_constant * sr));
    };

    baseline_attack_coeff = compute_coeff(BASELINE_ATTACK_TIME_SEC);
    baseline_release_coeff = compute_coeff(BASELINE_RELEASE_TIME_SEC);

    if (sr > 0.0 && THRESHOLD_RELAX_TIME_SEC > 0.0) {
        threshold_relax_coeff = std::exp(-1.0 / (THRESHOLD_RELAX_TIME_SEC * sr));
    } else {
        threshold_relax_coeff = 0.0;
    }
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

    // Threshold is baseline + some fraction above it
    double threshold_high = threshold * RISING_THRESHOLD_SCALE;  // Must exceed this to start rising
    double threshold_low = threshold * FALLING_THRESHOLD_SCALE;   // Must fall below this to start falling

    bool isPeak = false;

    // Peak detection: detect local maxima above threshold
    if (envelope_sample > threshold_high) {
        if (envelope_sample > prev_env) {
            is_rising = true;
        } else if (envelope_sample < prev_env && is_rising) {
            // Local maximum - check time-based constraints
            double min_beat_interval = 60.0 / max_bpm;  // Convert BPM to seconds
            double effective_min_interval = std::max(min_peak_distance, min_beat_interval);
            if (current_time - last_peak_time >= effective_min_interval) {
                isPeak = true;
                last_peak_time = current_time;
                dynamic_threshold_rel = threshold_rel;
            }
            is_rising = false;
        }
    } else if (envelope_sample < threshold_low) {
        is_rising = false;
    }

    prev_env = envelope_sample;

    return isPeak;
}

} // end namespace jellED
