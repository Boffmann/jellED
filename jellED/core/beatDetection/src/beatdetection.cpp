#include "beatdetection.h"

#include "bandpassFilter.h"
#include "envelopeDetector.h"

#include <algorithm>
#include <iostream>

constexpr int NUM_FILTER_STAGES = 2;
constexpr int ENVELOPE_DOWNSAMPLE_RATIO = 8;

BeatDetector::BeatDetector(IPlatformUtils& platformUtils, uint32_t sample_rate)
    : platformUtils(platformUtils),
      peakDetection(0.01, 0.1, 0.1, 0.4, 180.0, sample_rate / ENVELOPE_DOWNSAMPLE_RATIO),
      filterChain{nullptr},
      previous_filtered_sample{UNFILTERED},
      was_beat{false},
      sample_rate{sample_rate},
      sample_count{0}
{
    this->filterChain = (FilterStage**) malloc(sizeof(FilterStage*) * NUM_FILTER_STAGES);
    this->filterChain[0] = new BandpassFilter();
    this->filterChain[1] = new EnvelopeDetector(sample_rate, ENVELOPE_DOWNSAMPLE_RATIO);
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
    
    sample_count++;
    double current_time = static_cast<double>(sample_count) / (sample_rate / ENVELOPE_DOWNSAMPLE_RATIO);
    
    /*if (!std::isfinite(filteredSample)) {*/
    /*    std::cerr << "Warning: filteredSample is not finite: " << filteredSample << std::endl;*/
    /*}*/

    // Only process time and peak detection if the envelope detector actually processed the sample
    if (filteredSample == -1.0) {
        return false;
    }

    return peakDetection.is_peak(filteredSample, current_time);
}
