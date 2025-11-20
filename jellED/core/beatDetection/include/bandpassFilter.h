#ifndef __BANDPASS_FILTER_JELLED_H__
#define __BANDPASS_FILTER_JELLED_H__

#include "filterStage.h"
#include "ringbuffer.h"

namespace jellED {

static constexpr uint8_t NUM_SECTIONS = 4;

struct BandpassFilterCoefficients {
    double numerator[NUM_SECTIONS][3];
    double denominator[NUM_SECTIONS][3];
};

static const BandpassFilterCoefficients BANDPASS_FILTER_COEFFICIENTS_LOW = {
    // Butterworth coefficients: 50 Hz - 150 Hz, order: 4, framerate: 12000
    .numerator = {
        {4.3909323578773363e-07,8.781864715754673e-07,4.3909323578773363e-07},
        {1.0,2.0,1.0},
        {1.0,-2.0,1.0},
        {1.0,-2.0,1.0}
    },
    .denominator = {
        {1.0,-1.9379887838746337,0.9413140928118505},
        {1.0,-1.9631376181805404,0.9643496939711995},
        {1.0,-1.965110177295426,0.9708401194196212},
        {1.0,-1.9888635684671483,0.9895865156687846}
    }
};

static const BandpassFilterCoefficients BANDPASS_FILTER_COEFFICIENTS_MID = {
    // Butterworth coefficients: 150 Hz - 500 Hz, order: 4, framerate: 12000
    .numerator = {
        {5.608664074761718e-05,0.00011217328149523436,5.608664074761718e-05},
        {1.0,2.0,1.0},
        {1.0,-2.0,1.0},
        {1.0,-2.0,1.0}
    },
    .denominator = {
        {1.0,-1.7705386764144164,0.8038524257138367},
        {1.0,-1.874685968253344,0.8854806607750835},
        {1.0,-1.8386386510482773,0.8996622527108628},
        {1.0,-1.9601661846586276,0.9666024859378615}
    }
};

static const BandpassFilterCoefficients BANDPASS_FILTER_COEFFICIENTS_HIGH = {
    // Butterworth coefficients: 2000 Hz - 5000 Hz, order: 4, framerate: 12000
    .numerator = {
        {0.09398085143379449,-0.18796170286758898,0.09398085143379449},
        {1.0,2.0,1.0},
        {1.0,-2.0,1.0},
        {1.0,2.0,1.0}
    },
    .denominator = {
        {1.0,-0.2980882766400064,0.12197872109365554},
        {1.0,1.0301390842088836,0.32436911570995286},
        {1.0,-0.7673189938478389,0.5992565735507717},
        {1.0,1.4993698014167158,0.745027609002712}
    }
};

class BandpassFilter : public FilterStage {
private:
    const BandpassFilterCoefficients& coefficients;

    double applyBandpass(double sample, uint8_t section);
    Ringbuffer** prev_samples_per_section;
    Ringbuffer** prev_filtered_per_section;

public:
    BandpassFilter(const BandpassFilterCoefficients& coefficients);
    ~BandpassFilter();
    double apply(const double sample);

};

} // namespace jellED

#endif
