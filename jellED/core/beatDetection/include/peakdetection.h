#ifndef __PEAKDETECTION_JELLED_H__
#define __PEAKDETECTION_JELLED_H__

#include "include/ringbuffer.h"
#include <vector>
#include <memory>

namespace jellED {

class PeakDetector {
private:
    // Configuration parameters
    double absolute_min_threshold;
    double threshold_rel;
    double min_peak_distance;
    uint32_t sample_rate;
    double max_bpm;
    double threshold_baseline; 

    // Envelope follower state
    double envelope;
    double baseline_attack_coeff;
    double baseline_release_coeff;
    double dynamic_threshold_rel;
    double min_dynamic_threshold_rel;
    double threshold_relax_coeff;
    
    // Peak detection state
    double prev_env;
    bool is_rising;
    double last_peak_time;

    double update_envelope(double sample);

public:
    PeakDetector(double absolute_min_threshold, double threshold_rel, 
                 double min_peak_distance, double max_bpm, uint32_t sample_rate);
    
    bool is_peak(double sample, double current_time);
};

} // end namespace jellED

#endif
