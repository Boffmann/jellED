#ifndef __ONSET_STRENGTH_FUNCTION_JELLED_H__
#define __ONSET_STRENGTH_FUNCTION_JELLED_H__

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "beatDetectionConfig.h"
#include "peakdetection.h"

namespace jellED {

// One frame's worth of per-band data the OSF needs from each band: the
// envelope sample (already half-wave-rectified by EnvelopeDetector), whether
// the band's stats have warmed up, the per-band rolling median (used as a
// fallback divisor when adaptive whitening is disabled), and the band's
// fusion weight (already tilt-biased by the caller if applicable).
struct OsfBandFrame {
    float envelope;
    bool  statsReady;
    float median;
    float weight;
};

// Per-band novelty state. Tracks two slow-moving averages of the envelope:
//
//   - baseline: the recent *floor* (asymmetric one-pole, slow attack ~180 ms,
//               slow release ~800 ms). Subtracted from the envelope to produce
//               the half-wave-rectified novelty signal.
//   - whiteningAvg: the recent envelope average (symmetric one-pole, ~1.5 s).
//                   Acts as the divisor for adaptive whitening (Stowell &
//                   Plumbley 2007), making each band's contribution
//                   level-normalized so a sub-bass null doesn't kill the
//                   band's signal.
class OsfBandNovelty {
public:
    OsfBandNovelty()
        : baseline_(0.0F),
          baselineAttackCoeff_(0.0F),
          baselineReleaseCoeff_(0.0F),
          whiteningAvg_(0.0F),
          whiteningCoeff_(0.0F) {}

    // Pass the *envelope rate* (sampleRate / envelopeDownsampleRatio).
    void setBaselineTimings(float attackSec, float releaseSec, uint32_t envelopeRate) {
        baselineAttackCoeff_  = computeCoeff(envelopeRate, attackSec);
        baselineReleaseCoeff_ = computeCoeff(envelopeRate, releaseSec);
    }

    void setWhiteningTime(float timeSec, uint32_t envelopeRate) {
        whiteningCoeff_ = computeCoeff(envelopeRate, timeSec);
    }

    // Update the slow baseline + whitening average, then return the
    // half-wave-rectified, normalized novelty for this frame. Returns 0
    // until statsReady so that warm-up transients do not produce false beats.
    float compute(float envelope,
                  bool  statsReady,
                  float fallbackMedian,
                  bool  useWhitening,
                  float whiteningFloor) {
        const float coeff = (envelope > baseline_) ? baselineAttackCoeff_
                                                   : baselineReleaseCoeff_;
        baseline_     += coeff * (envelope - baseline_);
        whiteningAvg_ += whiteningCoeff_ * (envelope - whiteningAvg_);

        if (!statsReady) return 0.0F;
        const float diff = envelope - baseline_;
        if (diff <= 0.0F) return 0.0F;

        const float divisor = useWhitening
            ? std::max(whiteningAvg_, whiteningFloor)
            : std::max(fallbackMedian, whiteningFloor);
        return diff / divisor;
    }

    void reset() {
        baseline_     = 0.0F;
        whiteningAvg_ = 0.0F;
    }

    // Observability for tuning + tests.
    float getBaseline() const     { return baseline_; }
    float getWhiteningAvg() const { return whiteningAvg_; }

private:
    static float computeCoeff(uint32_t rate, float timeSec) {
        if (rate == 0 || timeSec <= 0.0F) return 1.0F;
        return 1.0F - std::exp(-1.0F / (static_cast<float>(rate) * timeSec));
    }

    float baseline_;
    float baselineAttackCoeff_;
    float baselineReleaseCoeff_;
    float whiteningAvg_;
    float whiteningCoeff_;
};

// Single-class home for the OSF beat detection pipeline:
//
//   per-band envelopes ──► OsfBandNovelty (×3) ──► weighted sum
//                                                     │
//                                                     ▼
//                                            one-pole smoother
//                                                     │
//                                                     ▼
//                                              PeakDetector  ──► is_beat
//
// All OSF-related state lives here so tuning, hot-update, and observability
// can flow through one object. Operates at the *envelope rate*
// (sampleRate / envelopeDownsampleRatio); pass the wrong rate and time
// constants will be off by the downsample ratio.
class OnsetStrengthFunction {
public:
    OnsetStrengthFunction(uint32_t envelopeRate, const BeatDetectionConfig& config)
        : peakDetector_(makePeakConfig(config), envelopeRate),
          osfSmoothed_(0.0F),
          osfInstantaneous_(0.0F),
          smoothingCoeff_(computeCoeff(envelopeRate, config.osfSmoothingTime)),
          useAdaptiveWhitening_(config.osfUseAdaptiveWhitening),
          whiteningFloor_(config.osfWhiteningFloor) {
        configureBands(config, envelopeRate);
    }

    // Process one envelope frame from each band. When `gateOpen` is false
    // (e.g. short-term energy below the level gate) the per-band novelty
    // trackers and peak detector are skipped, but the smoother is still fed
    // zeros so it doesn't snap on resume. Returns true when a beat peak was
    // detected.
    bool process(const OsfBandFrame& low,
                 const OsfBandFrame& mid,
                 const OsfBandFrame& high,
                 float currentTime,
                 bool  gateOpen) {
        if (!gateOpen) {
            osfInstantaneous_ = 0.0F;
            osfSmoothed_ += smoothingCoeff_ * (0.0F - osfSmoothed_);
            return false;
        }

        const float nL = noveltyLow_.compute(
            low.envelope,  low.statsReady,  low.median,
            useAdaptiveWhitening_, whiteningFloor_);
        const float nM = noveltyMid_.compute(
            mid.envelope,  mid.statsReady,  mid.median,
            useAdaptiveWhitening_, whiteningFloor_);
        const float nH = noveltyHigh_.compute(
            high.envelope, high.statsReady, high.median,
            useAdaptiveWhitening_, whiteningFloor_);

        osfInstantaneous_ =
              low.weight  * nL
            + mid.weight  * nM
            + high.weight * nH;
        osfSmoothed_ += smoothingCoeff_ * (osfInstantaneous_ - osfSmoothed_);

        return peakDetector_.is_peak(osfSmoothed_, currentTime);
    }

    void applyConfig(const BeatDetectionConfig& config, uint32_t envelopeRate) {
        smoothingCoeff_       = computeCoeff(envelopeRate, config.osfSmoothingTime);
        useAdaptiveWhitening_ = config.osfUseAdaptiveWhitening;
        whiteningFloor_       = config.osfWhiteningFloor;
        configureBands(config, envelopeRate);

        peakDetector_.setAbsoluteMinThreshold(config.osfAbsoluteMinThreshold);
        peakDetector_.setThresholdRel(config.osfThresholdRel);
        peakDetector_.setMaxBpm(config.peakDetectionMaxBpm);
        peakDetector_.setOnsetRatio(config.osfOnsetRatio);
        peakDetector_.setTimingParams(config.osfBaselineAttackTimeFinal,
                                      config.osfBaselineReleaseTimeFinal,
                                      config.osfThresholdRelaxTime);
        peakDetector_.setMinRelativeThresholdFactor(config.minRelativeThresholdFactor);
        peakDetector_.setHysteresisScales(config.risingThresholdScale,
                                          config.fallingThresholdScale);
        peakDetector_.setThresholdMode(config.thresholdMode,
                                       config.osfThresholdWindowMs,
                                       config.osfThresholdDelta);
    }

    void reset() {
        noveltyLow_.reset();
        noveltyMid_.reset();
        noveltyHigh_.reset();
        osfSmoothed_      = 0.0F;
        osfInstantaneous_ = 0.0F;
    }

    float getCurrentOsf()        const { return osfSmoothed_; }
    float getCurrentInstantaneous() const { return osfInstantaneous_; }
    float getThreshold()         const { return peakDetector_.getLastThreshold(); }

    // Observability for the GUI / tuning.
    const OsfBandNovelty& bandLow()  const { return noveltyLow_; }
    const OsfBandNovelty& bandMid()  const { return noveltyMid_; }
    const OsfBandNovelty& bandHigh() const { return noveltyHigh_; }

private:
    static float computeCoeff(uint32_t rate, float timeSec) {
        if (rate == 0 || timeSec <= 0.0F) return 1.0F;
        return 1.0F - std::exp(-1.0F / (static_cast<float>(rate) * timeSec));
    }

    static PeakDetectorConfig makePeakConfig(const BeatDetectionConfig& config) {
        return PeakDetectorConfig{
            config.osfAbsoluteMinThreshold,
            config.osfThresholdRel,
            config.peakDetectionMaxBpm,
            config.osfBaselineAttackTimeFinal,
            config.osfBaselineReleaseTimeFinal,
            config.osfThresholdRelaxTime,
            config.osfOnsetRatio,
            config.minRelativeThresholdFactor,
            config.risingThresholdScale,
            config.fallingThresholdScale,
            config.thresholdMode,
            config.osfThresholdWindowMs,
            config.osfThresholdDelta
        };
    }

    void configureBands(const BeatDetectionConfig& config, uint32_t envelopeRate) {
        for (OsfBandNovelty* band : {&noveltyLow_, &noveltyMid_, &noveltyHigh_}) {
            band->setBaselineTimings(config.osfBaselineAttackTime,
                                     config.osfBaselineReleaseTime,
                                     envelopeRate);
            band->setWhiteningTime(config.osfWhiteningTime, envelopeRate);
        }
    }

    OsfBandNovelty noveltyLow_;
    OsfBandNovelty noveltyMid_;
    OsfBandNovelty noveltyHigh_;
    PeakDetector   peakDetector_;

    float osfSmoothed_;
    float osfInstantaneous_;
    float smoothingCoeff_;
    bool  useAdaptiveWhitening_;
    float whiteningFloor_;
};

} // namespace jellED

#endif
