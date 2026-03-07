#ifndef _BEAT_DETECTOR_JELLED_H_
#define _BEAT_DETECTOR_JELLED_H_

#include <stdint.h>
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
    double envelopeGain;      // Scales the envelope output (1.0 = no scaling)
    double absoluteMin;       // Minimum envelope value to consider as a peak
    double relativeThreshold; // Peak must exceed baseline by this fraction
    double weight;            // Weight for multiband fusion
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
          envelope(sampleRate, envelopeDownsampleRatio, cfg.envelopeGain),
          peakDetector(cfg.absoluteMin, cfg.relativeThreshold, maxBpm,
                       sampleRate),
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
        // Only collect statistics when there's actual signal (not just noise)
        // This prevents using the silent period to calculate the median
        const double SIGNAL_THRESHOLD = 0.005;  // Only collect stats when envelope > 5mV equivalent
        
        if (envelopeSample > SIGNAL_THRESHOLD) {
            envelopeWindow->append(envelopeSample);
            
            // Compute median from all recent envelope values
            // Require 128 samples of actual signal (not silence)
            if (envelopeWindow-> size() >= 128) {
                rollingMedian = envelopeWindow->median();
                
                // Only mark as ready if median is reasonable (not noise)
                if (rollingMedian > SIGNAL_THRESHOLD * 0.5) {
                    statsReady = true;
                }
            }
        }
    }

    // Normalize peak value by envelope median, with capping to prevent extremes
    double normalize(double peakValue) const {
        if (!statsReady || rollingMedian < 1e-6) {
            return 0.0;  // Not enough data yet
        }
        return peakValue / rollingMedian;
    }
};

class BeatDetector {

public:
    class Builder;

    ~BeatDetector();
    bool is_beat(const double sample);

    double getFilteredSampleLow();
    double getFilteredSampleMid();
    double getFilteredSampleHigh();
    double getEnvelopeLow();
    double getEnvelopeMid();
    double getEnvelopeHigh();
    bool isPeakLow();
    bool isPeakMid();
    bool isPeakHigh();
    double getCurrentTime();

private:

    // Private constructor - use Builder to construct
    BeatDetector(const Builder& builder);

    int sampleRate_;
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

    // IPlatformUtils& platformUtils; // TODO

    BandConfig bandConfigLow_;
    BandConfig bandConfigMid_;
    BandConfig bandConfigHigh_;

    BandState bandStateLow_;
    BandState bandStateMid_;
    BandState bandStateHigh_;

    MultiBandFusion multibandFusion_;

    friend class Builder;
};

class BeatDetector::Builder {
public:
    Builder(int sampleRate)
        : sampleRate_(sampleRate)
        , envelopeDownsampleRatio_(1)
        , noveltyGain_(1.0)
        , peakDetectionAbsoluteMinThreshold_(0.1)
        , peakDetectionThresholdRel_(0.5)
        , peakDetectionMinPeakDistance_(0.3)
        , peakDetectionMaxBpm_(200.0) {}

    // Allow BeatDetector to access builder's private state
    friend class BeatDetector;

    Builder& setEnvelopeDownsampleRatio(int ratio) {
        envelopeDownsampleRatio_ = ratio;
        return *this;
    }

    Builder& setNoveltyGain(double gain) {
        noveltyGain_ = gain;
        return *this;
    }

    Builder& setPeakDetectionAbsoluteMinThreshold(double threshold) {
        peakDetectionAbsoluteMinThreshold_ = threshold;
        return *this;
    }

    Builder& setPeakDetectionThresholdRel(double threshold) {
        peakDetectionThresholdRel_ = threshold;
        return *this;
    }

    Builder& setPeakDetectionMinPeakDistance(double distance) {
        peakDetectionMinPeakDistance_ = distance;
        return *this;
    }

    Builder& setPeakDetectionMaxBpm(double bpm) {
        peakDetectionMaxBpm_ = bpm;
        return *this;
    }

    BeatDetector* build() {
        return new BeatDetector(*this);
    }

private:

    int sampleRate_;
    int envelopeDownsampleRatio_;
    double noveltyGain_;
    double peakDetectionAbsoluteMinThreshold_;
    double peakDetectionThresholdRel_;
    double peakDetectionMinPeakDistance_;
    double peakDetectionMaxBpm_;
};

} // namespace jellED

#endif
