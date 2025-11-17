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
#include "sound/raspi/usbMicro.h"

class AudioDisplay;

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
    jellED::BandpassFilter bandpassFilter_;
    jellED::EnvelopeDetector envelopeDetector_;
    jellED::PeakDetector peakDetector_;

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