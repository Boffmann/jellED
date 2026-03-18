#include "include/bandpassFilter.h"

#include <cstring>

namespace jellED {

BandpassFilter::BandpassFilter(const BandpassFilterCoefficients& coefficients)
    : coefficients(coefficients)
{
    std::memset(w, 0, sizeof(w));
}

// Applies the filter as a cascade of NUM_SECTIONS second-order sections (biquads).
//
// Each section implements the standard IIR difference equation:
//   y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2]
//         - a1*y[n-1] - a2*y[n-2]
//
// This is computed using Direct Form II Transposed (TDF-II), which is
// algebraically equivalent but uses only 2 state variables per section
// instead of 4, and requires no ring buffer or index arithmetic:
//
//   y[n]   = b0*x[n] + w[0]          <- output
//   w[0]  += b1*x[n] - a1*y[n]       <- update state (w[1] feeds in from previous step)
//   w[1]   = b2*x[n] - a2*y[n]       <- update state
//
// w[s][0] and w[s][1] hold the two delay-line values for section s.
float BandpassFilter::apply(const float sample) {
    float x = sample;
    for (int s = 0; s < NUM_SECTIONS; ++s) {
        const float b0 = coefficients.numerator[s][0];
        const float b1 = coefficients.numerator[s][1];
        const float b2 = coefficients.numerator[s][2];
        const float a1 = coefficients.denominator[s][1];
        const float a2 = coefficients.denominator[s][2];

        const float y = b0 * x + w[s][0];
        w[s][0] = b1 * x - a1 * y + w[s][1];
        w[s][1] = b2 * x - a2 * y;
        x = y;
    }
    return x;
}

} // end namespace jellED
