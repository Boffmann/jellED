#include "include/peakdetection.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

namespace jellED {

PeakDetector::PeakDetector(double absolute_min_threshold, double threshold_rel, 
                           double min_peak_distance, double max_bpm, uint32_t sample_rate)
    : absolute_min_threshold(absolute_min_threshold),
      threshold_rel(threshold_rel),
      min_peak_distance(min_peak_distance),
      sample_rate(sample_rate),
      max_bpm(max_bpm),
      threshold_baseline(0.0),
      envelope(0.0),
      prev_env(0.0),
      is_rising(false),
      last_peak_time(-min_peak_distance)
{}

bool PeakDetector::is_peak(double envelope_sample, double current_time) {
    double threshold = std::max(this->absolute_min_threshold, threshold_baseline * (1.0 + threshold_rel));

    // Threshold is baseline + some fraction above it
    double threshold_high = threshold * 1.2;  // Must exceed this to start rising
    double threshold_low = threshold * 0.8;   // Must fall below this to start falling

    bool isPeak = false;

    // Peak detection: detect local maxima above threshold
    if (envelope_sample > threshold_high) {
        if (envelope_sample > prev_env) {
            is_rising = true;
        } else if (envelope_sample < prev_env && is_rising) {
            // Local maximum - check time-based constraints
            double min_beat_interval = 60.0 / max_bpm;  // Convert BPM to seconds
            if (current_time - last_peak_time >= min_peak_distance && 
                current_time - last_peak_time >= min_beat_interval) {
                isPeak = true;
                last_peak_time = current_time;
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
