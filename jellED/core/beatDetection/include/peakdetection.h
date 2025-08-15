#ifndef __PEAKDETECTION_JELLED_H__
#define __PEAKDETECTION_JELLED_H__

#include "ringbuffer.h"
#include <vector>
#include <memory>

namespace jellED {

class PeakDetector {
private:
    // Configuration parameters
    double attack;
    double release;
    double threshold_rel;
    double min_peak_distance;
    uint32_t sample_rate;
    double max_bpm;

    // Envelope follower state
    double envelope;

    Ringbuffer* local_min_buffer;
    Ringbuffer* sample_times_buffer;
    Ringbuffer* envelope_buffer;
    
    // Peak detection state
    double prev_env;
    bool is_rising;
    double last_peak_time;

    double update_envelope(double sample);
    double dynamic_threshold();

public:
    PeakDetector(double attack, double release, double threshold_rel, 
                 double min_peak_distance, double max_bpm, uint32_t sample_rate);
    ~PeakDetector();
    
    bool is_peak(double sample, double current_time);
};

} // end namespace jellED

#endif
