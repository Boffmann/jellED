#ifndef __BANDPASS_FILTER_JELLED_H__
#define __BANDPASS_FILTER_JELLED_H__

#include "filterStage.h"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace jellED {

static constexpr uint8_t NUM_SECTIONS = 4;

// Coefficients for one bandpass filter, expressed as a cascade of NUM_SECTIONS
// second-order sections (biquads). The layout matches scipy's SOS output
// (signal.butter(order, [lo, hi], 'band', output='sos')): each section row is
// [b0, b1, b2] in `numerator` and [a0, a1, a2] in `denominator`.
struct BandpassFilterCoefficients {
  float numerator[NUM_SECTIONS][3];
  float denominator[NUM_SECTIONS][3];
};

// The low/mid/high bandpass filters that form one analysis front-end, all
// designed for a single sample rate.
//
// Coefficients are sample-rate dependent: the same passband (e.g. 50-150 Hz)
// maps to different biquad coefficients at 12000 Hz than at 11025 Hz, because
// the cutoffs are normalized against the Nyquist frequency. Define one bank per
// supported rate below, register it in BANDPASS_FILTER_BANKS, and look it up
// via BandpassFilter::coefficientsForSampleRate().
struct BandpassFilterBank {
  uint32_t sampleRate;
  BandpassFilterCoefficients low;
  BandpassFilterCoefficients mid;
  BandpassFilterCoefficients high;
};

// ---------------------------------------------------------------------------
// 12000 Hz
// ---------------------------------------------------------------------------
static const BandpassFilterBank BANDPASS_FILTERS_12000 = {
    .sampleRate = 12000,
    // Butterworth: 50 Hz - 150 Hz, order 4
    .low = {.numerator = {{4.39093236e-07f, 8.78186472e-07f, 4.39093236e-07f},
                          {1.0f, 2.0f, 1.0f},
                          {1.0f, -2.0f, 1.0f},
                          {1.0f, -2.0f, 1.0f}},
            .denominator = {{1.0f, -1.93798878f, 0.94131409f},
                            {1.0f, -1.96313762f, 0.96434969f},
                            {1.0f, -1.96511018f, 0.97084012f},
                            {1.0f, -1.98886357f, 0.98958652f}}},
    // Butterworth: 150 Hz - 500 Hz, order 4
    .mid = {.numerator = {{5.60866407e-05f, 1.12173281e-04f, 5.60866407e-05f},
                          {1.0f, 2.0f, 1.0f},
                          {1.0f, -2.0f, 1.0f},
                          {1.0f, -2.0f, 1.0f}},
            .denominator = {{1.0f, -1.77053868f, 0.80385243f},
                            {1.0f, -1.87468597f, 0.88548066f},
                            {1.0f, -1.83863865f, 0.89966225f},
                            {1.0f, -1.96016618f, 0.96660249f}}},
    // Butterworth: 2000 Hz - 5000 Hz, order 4
    .high = {.numerator = {{0.09398085f, -0.18796170f, 0.09398085f},
                           {1.0f, 2.0f, 1.0f},
                           {1.0f, -2.0f, 1.0f},
                           {1.0f, 2.0f, 1.0f}},
             .denominator = {{1.0f, -0.29808828f, 0.12197872f},
                             {1.0f, 1.03013908f, 0.32436912f},
                             {1.0f, -0.76731899f, 0.59925657f},
                             {1.0f, 1.49936980f, 0.74502761f}}}};

// ---------------------------------------------------------------------------
// 11025 Hz
// ---------------------------------------------------------------------------
static const BandpassFilterBank BANDPASS_FILTERS_11025 = {
    .sampleRate = 11025,
    // Butterworth coefficients: 50 Hz - 150 Hz, order: 4
    .low = {.numerator = {{6.12649430e-07f, 1.22529886e-06f, 6.12649430e-07f},
                          {1.0f, 2.0f, 1.0f},
                          {1.0f, -2.0f, 1.0f},
                          {1.0f, -2.0f, 1.0f}},
            .denominator = {{1.0f, -1.9323602255070662f, 0.9362895085929855f},
                            {1.0f, -1.9598226435425998f, 0.9612563071434637f},
                            {1.0f, -1.9615267103273473f, 0.9683056519333086f},
                            {1.0f, -1.9878142705706652f, 0.9886703361155026f}}},
    // Butterworth coefficients: 150 Hz - 500 Hz, order: 4
    .mid = {.numerator = {{7.72193223e-05f, 1.54438645e-04f, 7.72193223e-05f},
                          {1.0f, 2.0f, 1.0f},
                          {1.0f, -2.0f, 1.0f},
                          {1.0f, -2.0f, 1.0f}},
            .denominator = {{1.0f, -1.7492059551654542f, 0.7883395117974024f},
                            {1.0f, -1.8632033748970132f, 0.8759256632651519f},
                            {1.0f, -1.819487860306651f, 0.8914043066624061f},
                            {1.0f, -1.9560738350109024f, 0.9636867631519238f}}},
    // Butterworth coefficients: 2000 Hz - 5000 Hz, order: 4
    .high = {
        .numerator = {{0.12270262522778112f, -0.24540525045556225f,
                       0.12270262522778112f},
                      {1.0f, 2.0f, 1.0f},
                      {1.0f, -2.0f, 1.0f},
                      {1.0f, 2.0f, 1.0f}},
        .denominator = {{1.0f, -0.2854340502424792f, 0.08224603131223178f},
                        {1.0f, 1.44810888916173f, 0.5398127758707263f},
                        {1.0f, -0.6193060342249346f, 0.5443657482643461f},
                        {1.0f, 1.7467579179530632f, 0.8272552974804415f}}}};

// Registry of every sample rate that has defined coefficients. Add one line per
// bank; coefficientsForSampleRate() scans this list and matches on .sampleRate.
static const BandpassFilterBank *const BANDPASS_FILTER_BANKS[] = {
    &BANDPASS_FILTERS_12000,
    &BANDPASS_FILTERS_11025,
};

class BandpassFilter : public FilterStage {
private:
  const BandpassFilterCoefficients &coefficients;
  // Direct Form II Transposed state: w[section][0..1]
  float w[NUM_SECTIONS][2];

public:
  explicit BandpassFilter(const BandpassFilterCoefficients &coefficients);
  float apply(const float sample) override;

  // Returns the low/mid/high bandpass coefficient bank designed for the given
  // sample rate. Throws std::invalid_argument if no bank has been registered
  // for that rate (see BANDPASS_FILTER_BANKS above).
  static const BandpassFilterBank &
  coefficientsForSampleRate(uint32_t sampleRate) {
    for (const BandpassFilterBank *bank : BANDPASS_FILTER_BANKS) {
      if (bank->sampleRate == sampleRate) {
        return *bank;
      }
    }
    throw std::invalid_argument(
        "No bandpass filter coefficients defined for sample rate " +
        std::to_string(sampleRate) + " Hz");
  }
};

} // namespace jellED

#endif
