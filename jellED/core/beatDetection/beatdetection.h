#ifndef _BEAT_DETECTOR_JELLED_H_
#define _BEAT_DETECTOR_JELLED_H_

#include <stdint.h>
#include "include/beatDetectionConfig.h"
#include "include/ringbuffer.h"
#include "include/automaticGainControl.h"
#include "include/bandpassFilter.h"
#include "include/envelopeDetector.h"
#include "include/peakdetection.h"
#include "include/multibandFusion.h"
#include "pUtils/IPlatformUtils.h"

namespace jellED {

struct BandConfig {
    const BandpassFilterCoefficients& coeffs;
    double envelopeGain;
    double absoluteMin;
    double relativeThreshold;
    double weight;
    double envelopeAttackTime;
    double envelopeReleaseTime;

    // Peak detector timing (per-band)
    double baselineAttackTime;
    double baselineReleaseTime;
    double thresholdRelaxTime;
    double onsetRatio;

    // Peak detector tuning (global, same for all bands)
    double minRelativeThresholdFactor;
    double risingThresholdScale;
    double fallingThresholdScale;
};

struct BandState {
    BandpassFilter filter;
    EnvelopeDetector envelope;
    PeakDetector peakDetector;
    double weight;
    double rollingMedian;
    Ringbuffer* envelopeWindow;
    bool statsReady;

    BandState(const BandConfig& cfg,
              uint32_t sampleRate,
              uint32_t envelopeDownsampleRatio,
              double maxBpm)
        : filter(cfg.coeffs),
          envelope(sampleRate, envelopeDownsampleRatio, cfg.envelopeGain,
                   cfg.envelopeAttackTime, cfg.envelopeReleaseTime),
          peakDetector(PeakDetectorConfig{
              cfg.absoluteMin, cfg.relativeThreshold, maxBpm,
              cfg.baselineAttackTime, cfg.baselineReleaseTime,
              cfg.thresholdRelaxTime, cfg.onsetRatio,
              cfg.minRelativeThresholdFactor,
              cfg.risingThresholdScale, cfg.fallingThresholdScale
          }, sampleRate),
          weight(cfg.weight),
          rollingMedian(0.0),
          statsReady(false) {
        envelopeWindow = new Ringbuffer(128, true);
        envelopeWindow->clear();
    }

    ~BandState() {
        delete envelopeWindow;
    }

    double applyBandpassFilter(double sample) {
        return filter.apply(sample);
    }

    double applyEnvelopeFilter(double sample) {
        double finalEnvelopeSample = envelope.apply(sample);
        if (finalEnvelopeSample != -1.0) {
            updateEnvelopeStats(finalEnvelopeSample);
        }
        return finalEnvelopeSample;
    }

    double applyPeakDetector(double sample, double currentTime) {
        bool isPeak = peakDetector.is_peak(sample, currentTime);
        if (isPeak) {
            double normalized = this->normalize(sample);
            return normalized * this->weight;
        }
        return 0.0;
    }

private:
    void updateEnvelopeStats(double envelopeSample) {
        const double SIGNAL_THRESHOLD = 0.005;
        
        if (envelopeSample > SIGNAL_THRESHOLD) {
            envelopeWindow->append(envelopeSample);
            
            if (envelopeWindow-> size() >= 128) {
                rollingMedian = envelopeWindow->median();
                
                if (rollingMedian > SIGNAL_THRESHOLD * 0.5) {
                    statsReady = true;
                }
            }
        }
    }

    double normalize(double peakValue) const {
        if (!statsReady || rollingMedian < 1e-6) {
            return 0.0;
        }
        return peakValue / rollingMedian;
    }
};

class BeatDetector {

public:
    explicit BeatDetector(int sampleRate, const BeatDetectionConfig& config);
    ~BeatDetector();

    bool is_beat(const double sample);

    // Applies hot parameter changes in-place without losing internal state.
    // Returns true if all changes were applied successfully.
    // Returns false if a structural parameter changed that requires reconstruction
    // (currently only envelopeDownsampleRatio).
    bool applyConfig(const BeatDetectionConfig& newConfig);

    double getFilteredSampleLow();
    double getFilteredSampleMid();
    double getFilteredSampleHigh();
    double getEnvelopeLow();
    double getEnvelopeMid();
    double getEnvelopeHigh();
    bool isPeakLow();
    bool isPeakMid();
    bool isPeakHigh();
    double getThresholdLow();
    double getThresholdMid();
    double getThresholdHigh();
    double getCurrentTime();

private:
    int sampleRate_;
    BeatDetectionConfig config_;
    uint32_t totalSamplesReceived_;

    double filteredSampleLow_;
    double filteredSampleMid_;
    double filteredSampleHigh_;
    double envelopeSampleLow_;
    double envelopeSampleMid_;
    double envelopeSampleHigh_;
    bool peakDetectedLow_;
    bool peakDetectedMid_;
    bool peakDetectedHigh_;
    double currentTime_;

    BandConfig bandConfigLow_;
    BandConfig bandConfigMid_;
    BandConfig bandConfigHigh_;

    BandState bandStateLow_;
    BandState bandStateMid_;
    BandState bandStateHigh_;

    MultiBandFusion multibandFusion_;
};

} // namespace jellED

#endif
