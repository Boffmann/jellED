#include "beatdetection.h"

#include "bandpassFilter.h"
#include "envelopeDetector.h"

#include <algorithm>
#include <iostream>

constexpr int NUM_FILTER_STAGES = 2;

BeatDetector::BeatDetector(IPlatformUtils& platformUtils, uint32_t sample_rate)
    : platformUtils(platformUtils),
      peakDetection{0.01, 0.1, 0.1, 0.4, 180.0, sample_rate},
      filterChain{nullptr},
      previous_filtered_sample{UNFILTERED},
      was_beat{false},
      sample_rate{sample_rate},
      sample_count{0}
{
    this->filterChain = (FilterStage**) malloc(sizeof(FilterStage) * NUM_FILTER_STAGES);
    this->filterChain[0] = new BandpassFilter();
    this->filterChain[1] = new EnvelopeDetector(sample_rate, 1);
}

BeatDetector::~BeatDetector() {
    if (this->filterChain != nullptr) {
        delete this->filterChain[0];
        delete this->filterChain[1];
        free(this->filterChain);
    }
}

bool BeatDetector::is_beat(const double sample) {
    double filteredSample = sample;
    for (int filterIndex = 0; filterIndex < NUM_FILTER_STAGES; filterIndex++) {
        filteredSample = this->filterChain[filterIndex]->apply(filteredSample);
    }
    
    // Calculate current time in seconds
    // This can be done better I guess
    double current_time = static_cast<double>(sample_count) / sample_rate;
    sample_count++;
    
    if (this->previous_filtered_sample == UNFILTERED || this->previous_filtered_sample != filteredSample) {
        this->previous_filtered_sample = filteredSample;
        
        bool is_beat_detected = peakDetection.is_peak(filteredSample, current_time);
        
        if (is_beat_detected && !this->was_beat) {
            std::cout << "BEAT DETECTED at " << current_time << "s" << std::endl;
            this->was_beat = true;
            return true;
        } else if (!is_beat_detected) {
            this->was_beat = false;
        }
    }
    return false;
}
