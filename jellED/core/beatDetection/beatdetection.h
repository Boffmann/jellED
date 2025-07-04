#ifndef _BEAT_DETECTOR_JELLED_H_
#define _BEAT_DETECTOR_JELLED_H_

#include <stdint.h>
#include "include/filterStage.h"
#include "include/peakdetection.h"
#include "../IPlatformUtils.h"

class BeatDetector {
private:
    static constexpr double UNFILTERED = 2.0;
    IPlatformUtils& platformUtils;
    FilterStage** filterChain;
    PeakDetector peakDetection;
    double previous_filtered_sample;
    bool was_beat;
    uint32_t sample_rate;
    uint64_t sample_count;

public:
    BeatDetector(IPlatformUtils& platformUtils, uint32_t sample_rate);
    ~BeatDetector();
    bool is_beat(const double sample);
};

#endif
