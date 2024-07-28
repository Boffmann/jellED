#ifndef __BANDPASS_FILTER_JELLED_H__
#define __BANDPASS_FILTER_JELLED_H__

#include "filterStage.h"
#include "ringbuffer.h"

class BandpassFilter : public FilterStage {
private:
    // TODO define num sections
    static constexpr uint8_t NUM_SECTIONS = 4;
    // The Butterworth numerator (b)
    double numerator[NUM_SECTIONS][3];
    // The Butterworth denominator (a)
    double denominator[NUM_SECTIONS][3];

    double applyBandpass(double sample, uint8_t section);
    Ringbuffer** prev_samples_per_section;
    Ringbuffer** prev_filtered_per_section;

public:
    BandpassFilter();
    ~BandpassFilter();
    double apply(const double sample);

};

#endif
