#ifndef __BANDPASS_FILTER_JELLED_H__
#define __BANDPASS_FILTER_JELLED_H__

#include "filterStage.h"
#include "ringbuffer.h"

namespace jellED {

static constexpr uint8_t NUM_SECTIONS = 4;

struct BandpassFilterCoefficients {
    float numerator[NUM_SECTIONS][3];
    float denominator[NUM_SECTIONS][3];
};

static const BandpassFilterCoefficients BANDPASS_FILTER_COEFFICIENTS_LOW = {
    // Butterworth coefficients: 50 Hz - 150 Hz, order: 4, framerate: 12000
    .numerator = {
        {4.39093236e-07f, 8.78186472e-07f, 4.39093236e-07f},
        {1.0f, 2.0f, 1.0f},
        {1.0f,-2.0f, 1.0f},
        {1.0f,-2.0f, 1.0f}
    },
    .denominator = {
        {1.0f,-1.93798878f, 0.94131409f},
        {1.0f,-1.96313762f, 0.96434969f},
        {1.0f,-1.96511018f, 0.97084012f},
        {1.0f,-1.98886357f, 0.98958652f}
    }
};

static const BandpassFilterCoefficients BANDPASS_FILTER_COEFFICIENTS_MID = {
    // Butterworth coefficients: 150 Hz - 500 Hz, order: 4, framerate: 12000
    .numerator = {
        {5.60866407e-05f, 1.12173281e-04f, 5.60866407e-05f},
        {1.0f, 2.0f, 1.0f},
        {1.0f,-2.0f, 1.0f},
        {1.0f,-2.0f, 1.0f}
    },
    .denominator = {
        {1.0f,-1.77053868f, 0.80385243f},
        {1.0f,-1.87468597f, 0.88548066f},
        {1.0f,-1.83863865f, 0.89966225f},
        {1.0f,-1.96016618f, 0.96660249f}
    }
};

static const BandpassFilterCoefficients BANDPASS_FILTER_COEFFICIENTS_HIGH = {
    // Butterworth coefficients: 2000 Hz - 5000 Hz, order: 4, framerate: 12000
    .numerator = {
        { 0.09398085f,-0.18796170f, 0.09398085f},
        {1.0f, 2.0f, 1.0f},
        {1.0f,-2.0f, 1.0f},
        {1.0f, 2.0f, 1.0f}
    },
    .denominator = {
        {1.0f,-0.29808828f, 0.12197872f},
        {1.0f, 1.03013908f, 0.32436912f},
        {1.0f,-0.76731899f, 0.59925657f},
        {1.0f, 1.49936980f, 0.74502761f}
    }
};

class BandpassFilter : public FilterStage {
private:
    const BandpassFilterCoefficients& coefficients;

    float applyBandpass(float sample, uint8_t section);
    Ringbuffer** prev_samples_per_section;
    Ringbuffer** prev_filtered_per_section;

public:
    BandpassFilter(const BandpassFilterCoefficients& coefficients);
    ~BandpassFilter();
    float apply(const float sample);

};

} // namespace jellED

#endif
