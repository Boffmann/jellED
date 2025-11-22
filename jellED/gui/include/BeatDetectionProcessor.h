#ifndef __BEAT_DETECTION_PROCESSOR_H__
#define __BEAT_DETECTION_PROCESSOR_H__

#include <QThread>
#include <memory>
#include <stdexcept>
#include "include/bandpassFilter.h"
#include "include/bandpassFilter.h"
#include "include/envelopeDetector.h"
#include "include/peakdetection.h"
#include "include/automaticGainControl.h"
#include "include/downsampler.h"
#include "MultibandFusion.h"
#include "sound/raspi/usbMicro.h"

class AudioDisplay;

struct BandConfig {
    const jellED::BandpassFilterCoefficients& coeffs;
    double noveltyGain;
    double absoluteMin;
    double relativeThreshold;
    double weight;
};

struct BandState {
    jellED::BandpassFilter filter;
    jellED::EnvelopeDetector envelope;
    jellED::PeakDetector peakDetector;
    double weight;
    double rollingMedian;
    std::deque<double> envelopeWindow;
    bool statsReady;

    BandState(const BandConfig& cfg,
              uint32_t sampleRate,
              uint32_t envelopeDownsampleRatio,
              double maxBpm)
        : filter(cfg.coeffs),
          envelope(sampleRate, envelopeDownsampleRatio, cfg.noveltyGain),
          peakDetector(cfg.absoluteMin, cfg.relativeThreshold, maxBpm,
                       sampleRate),
          weight(cfg.weight),
          rollingMedian(0.0),
          statsReady(false) {}

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
            envelopeWindow.push_back(envelopeSample);
            if (envelopeWindow.size() > 128) envelopeWindow.pop_front();
            
            // Compute median from all recent envelope values
            // Require 128 samples of actual signal (not silence)
            if (envelopeWindow.size() >= 128) {
                std::vector<double> tmp(envelopeWindow.begin(), envelopeWindow.end());
                std::nth_element(tmp.begin(), tmp.begin() + tmp.size() / 2, tmp.end());
                rollingMedian = tmp[tmp.size() / 2];
                
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
            return 0.0;  // Not enough data yet - return 0 to prevent false positives
        }
        double normalized = peakValue / rollingMedian;
        // Cap normalized value to prevent extreme scores
        // Use 3x instead of 5x for tighter control
        return std::min(normalized, 3.0);
    }
};

class BeatDetectionProcessor : public QThread {
    Q_OBJECT

public:
    class Builder;

    void stop() {
        shouldStop_ = true;
    }

    void start() {
        shouldStop_ = false;
        QThread::start();
    }

protected:
    void run() override;

private:
    // Private constructor - use Builder to construct
    explicit BeatDetectionProcessor(const Builder& builder);

    AudioDisplay* display_;
    jellED::UsbMicro* usbMicro_;
    bool shouldStop_;
    int signalDownsampleRatio_;
    uint32_t totalSamplesReceived_;
    jellED::Downsampler downsampler_;
    jellED::AutomaticGainControl automaticGainControl_;

    BandConfig bandConfigLow_;
    BandConfig bandConfigMid_;
    BandConfig bandConfigHigh_;

    BandState bandStateLow_;
    BandState bandStateMid_;
    BandState bandStateHigh_;

    MultiBandFusion multibandFusion_;

    friend class Builder;
};

class BeatDetectionProcessor::Builder {
public:
    Builder()
        : display_(nullptr)
        , usbMicro_(nullptr)
        , signalDownsampleRatio_(1)
        , envelopeDownsampleRatio_(1)
        , noveltyGain_(1.0)
        , downsampleCutoffFrequency_(0.45)
        , automaticGainControlTargetLevel_(0.8)
        , peakDetectionAbsoluteMinThreshold_(0.1)
        , peakDetectionThresholdRel_(0.5)
        , peakDetectionMinPeakDistance_(0.3)
        , peakDetectionMaxBpm_(200.0)
        , parent_(nullptr) {}

    // Allow BeatDetectionProcessor to access builder's private state
    friend class BeatDetectionProcessor;

    Builder& setDisplay(AudioDisplay* display) {
        display_ = display;
        return *this;
    }

    Builder& setUsbMicro(jellED::UsbMicro* usbMicro) {
        usbMicro_ = usbMicro;
        return *this;
    }

    Builder& setSignalDownsampleRatio(int ratio) {
        signalDownsampleRatio_ = ratio;
        return *this;
    }

    Builder& setEnvelopeDownsampleRatio(int ratio) {
        envelopeDownsampleRatio_ = ratio;
        return *this;
    }

    Builder& setNoveltyGain(double gain) {
        noveltyGain_ = gain;
        return *this;
    }

    Builder& setDownsampleCutoffFrequency(double frequency) {
        downsampleCutoffFrequency_ = frequency;
        return *this;
    }

    Builder& setAutomaticGainControlTargetLevel(double level) {
        automaticGainControlTargetLevel_ = level;
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

    Builder& setParent(QObject* parent) {
        parent_ = parent;
        return *this;
    }

    BeatDetectionProcessor* build() {
        if (!display_) {
            throw std::invalid_argument("Display must be set before building BeatDetectionProcessor");
        }
        if (!usbMicro_) {
            throw std::invalid_argument("UsbMicro must be set before building BeatDetectionProcessor");
        }

        return new BeatDetectionProcessor(*this);
    }

private:
    AudioDisplay* display_;
    jellED::UsbMicro* usbMicro_;
    int signalDownsampleRatio_;
    int envelopeDownsampleRatio_;
    double noveltyGain_;
    double downsampleCutoffFrequency_;
    double automaticGainControlTargetLevel_;
    double peakDetectionAbsoluteMinThreshold_;
    double peakDetectionThresholdRel_;
    double peakDetectionMinPeakDistance_;
    double peakDetectionMaxBpm_;
    QObject* parent_;
};

#endif
