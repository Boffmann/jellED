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
    float envelopeGain;
    float absoluteMin;
    float relativeThreshold;
    float weight;
    float envelopeAttackTime;
    float envelopeReleaseTime;

    // Peak detector timing (per-band)
    float baselineAttackTime;
    float baselineReleaseTime;
    float thresholdRelaxTime;
    float onsetRatio;

    // Peak detector tuning (global, same for all bands)
    float minRelativeThresholdFactor;
    float risingThresholdScale;
    float fallingThresholdScale;
};

struct BandState {
    BandpassFilter filter;
    EnvelopeDetector envelope;
    PeakDetector peakDetector;
    float weight;
    float rollingMedian;
    Ringbuffer* envelopeWindow;
    bool statsReady;

    BandState(const BandConfig& cfg,
              uint32_t sampleRate,
              uint32_t envelopeDownsampleRatio,
              float maxBpm)
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
          rollingMedian(0.0f),
          statsReady(false) {
        envelopeWindow = new Ringbuffer(128, true);
        envelopeWindow->clear();
    }

    ~BandState() {
        delete envelopeWindow;
    }

    float applyBandpassFilter(float sample) {
        return filter.apply(sample);
    }

    float applyEnvelopeFilter(float sample) {
        float finalEnvelopeSample = envelope.apply(sample);
        if (finalEnvelopeSample != -1.0f) {
            updateEnvelopeStats(finalEnvelopeSample);
        }
        return finalEnvelopeSample;
    }

    float applyPeakDetector(float sample, float currentTime) {
        bool isPeak = peakDetector.is_peak(sample, currentTime);
        if (isPeak) {
            float normalized = this->normalize(sample);
            return normalized * this->weight;
        }
        return 0.0f;
    }

    float getVolume() const { return rollingMedian; }

private:
    void updateEnvelopeStats(float envelopeSample) {
        const float SIGNAL_THRESHOLD = 0.005f;

        if (envelopeSample > SIGNAL_THRESHOLD) {
            envelopeWindow->append(envelopeSample);

            if (envelopeWindow->size() >= 128) {
                rollingMedian = envelopeWindow->median();

                if (rollingMedian > SIGNAL_THRESHOLD * 0.5f) {
                    statsReady = true;
                }
            }
        }
    }

    float normalize(float peakValue) const {
        if (!statsReady || rollingMedian < 1e-6f) {
            return 0.0f;
        }
        return peakValue / rollingMedian;
    }
};

class BeatDetector {

public:
    explicit BeatDetector(int sampleRate, const BeatDetectionConfig& config);
    ~BeatDetector();

    bool is_beat(const float sample);

    // Applies hot parameter changes in-place without losing internal state.
    // Returns true if all changes were applied successfully.
    // Returns false if a structural parameter changed that requires reconstruction
    // (currently only envelopeDownsampleRatio).
    bool applyConfig(const BeatDetectionConfig& newConfig);

    float getFilteredSampleLow();
    float getFilteredSampleMid();
    float getFilteredSampleHigh();
    float getEnvelopeLow();
    float getEnvelopeMid();
    float getEnvelopeHigh();
    bool isPeakLow();
    bool isPeakMid();
    bool isPeakHigh();
    float getThresholdLow();
    float getThresholdMid();
    float getThresholdHigh();
    float getCurrentTime();

    float getVolumeLow() const;
    float getVolumeMid() const;
    float getVolumeHigh() const;
    float getOverallLevel() const;
    float getVolumeTrend() const;
    float getSpectralTilt() const;

private:
    int sampleRate_;
    BeatDetectionConfig config_;
    uint32_t totalSamplesReceived_;

    float filteredSampleLow_;
    float filteredSampleMid_;
    float filteredSampleHigh_;
    float envelopeSampleLow_;
    float envelopeSampleMid_;
    float envelopeSampleHigh_;
    bool peakDetectedLow_;
    bool peakDetectedMid_;
    bool peakDetectedHigh_;
    float currentTime_;

    BandConfig bandConfigLow_;
    BandConfig bandConfigMid_;
    BandConfig bandConfigHigh_;

    BandState bandStateLow_;
    BandState bandStateMid_;
    BandState bandStateHigh_;

    MultiBandFusion multibandFusion_;

    float shortTermEnergy_;
    float longTermEnergy_;
    float shortTermCoeff_;
    float longTermCoeff_;
};

} // namespace jellED

#endif
