#ifndef __BEAT_DETECTION_PROCESSOR_H__
#define __BEAT_DETECTION_PROCESSOR_H__

#include <QThread>
#include "include/bandpassFilter.h"
#include "include/bandpassFilter.h"
#include "include/envelopeDetector.h"
#include "include/peakdetection.h"
#include "include/downsampler.h"
#include "sound/raspi/usbMicro.h"

class AudioDisplay;

class BeatDetectionProcessor : public QThread {
    Q_OBJECT

public:
    BeatDetectionProcessor(AudioDisplay* display,
        jellED::UsbMicro* usbMicro,
        int signalDownsampleRatio,
        int envelopeDownsampleRatio,
        double downsampleCutoffFrequency,
        double peakDetectionAbsoluteMinThreshold,
        double peakDetectionThresholdRel,
        double peakDetectionMinPeakDistance,
        double peakDetectionMaxBpm,
        QObject* parent = nullptr);

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
    AudioDisplay* display_;
    jellED::UsbMicro* usbMicro_;
    bool shouldStop_;
    int signalDownsampleRatio_;
    uint32_t totalSamplesReceived_;
    jellED::Downsampler downsampler_;
    jellED::BandpassFilter bandpassFilter_;
    jellED::EnvelopeDetector envelopeDetector_;
    jellED::PeakDetector peakDetector_;
};

#endif