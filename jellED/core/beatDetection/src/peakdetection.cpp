#include "include/peakdetection.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

namespace jellED {

PeakDetector::PeakDetector(double attack, double release, double threshold_rel, 
                           double min_peak_distance, double max_bpm, uint32_t sample_rate)
    : attack(attack),
      release(release),
      threshold_rel(threshold_rel),
      min_peak_distance(min_peak_distance),
      sample_rate(sample_rate),
      max_bpm(max_bpm),
      envelope(0.0),
      prev_env(0.0),
      is_rising(false),
      last_peak_time(-min_peak_distance)
{
    this->local_min_buffer = new Ringbuffer((uint16_t) (0.5 * sample_rate));  // 0.5s window
    this->sample_times_buffer = new Ringbuffer((uint16_t) (1.0 * sample_rate));   // 1s window
    this->envelope_buffer = new Ringbuffer((uint16_t) (0.1 * sample_rate));  // 0.1s window
}

PeakDetector::~PeakDetector() {
    delete this->local_min_buffer;
    delete this->sample_times_buffer;
    delete this->envelope_buffer;
}

double PeakDetector::update_envelope(double sample) {
    double sample_abs = std::abs(sample);
    double coeff;
    
    if (sample_abs > envelope) {
        coeff = 1.0 - std::exp(-1.0 / (attack * sample_rate));
    } else {
        coeff = 1.0 - std::exp(-1.0 / (release * sample_rate));
    }
    
    envelope += coeff * (sample_abs - envelope);
    return envelope;
}

double PeakDetector::dynamic_threshold() {
    // Use a moving minimum as the baseline
    if (local_min_buffer->size() == 0) {
        return 0.0;
    }
    double local_min = local_min_buffer->min();
    return local_min + threshold_rel * (local_min_buffer->max() - local_min);
}

bool PeakDetector::is_peak(double sample, double current_time) {
    // Update envelope
    double env = update_envelope(sample);
    
    // Update ring buffers
    local_min_buffer->append(env);
    sample_times_buffer->append(current_time);
    envelope_buffer->append(env);
    
    // Calculate dynamic threshold
    double threshold = dynamic_threshold();
    bool isPeak = false;

    // Peak detection: detect local maxima above threshold
    if (env > threshold) {
        if (env > prev_env) {
            is_rising = true;
        } else if (env < prev_env && is_rising) {
            // Local maximum - check time-based constraints
            if (current_time - last_peak_time >= min_peak_distance) {
                isPeak = true;
                last_peak_time = current_time;
            }
            is_rising = false;
        }
    } else {
        is_rising = false;
    }

    prev_env = env;

    return isPeak;
}

} // end namespace jellED
