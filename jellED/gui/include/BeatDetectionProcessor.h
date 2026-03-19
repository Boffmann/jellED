#ifndef __BEAT_DETECTION_PROCESSOR_H__
#define __BEAT_DETECTION_PROCESSOR_H__

#include <QThread>
#include <memory>
#include <stdexcept>
#include "beatdetection.h"
#include "include/downsampler.h"
#include "include/noiseGate.h"
#include "sound/soundinput.h"

class AudioDisplay;

class BeatDetectionProcessor : public QThread {
    Q_OBJECT

public:
    explicit BeatDetectionProcessor(
        AudioDisplay* display,
        jellED::SoundInput* soundInput,
        const jellED::BeatDetectionConfig& config,
        int signalDownsampleRatio,
        QObject* parent);

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
    jellED::SoundInput* soundInput_;
    bool shouldStop_;

    jellED::BeatDetector* beatDetector_;

    jellED::Downsampler downsampler_;
    jellED::NoiseGate noiseGate_;
    jellED::AutomaticGainControl automaticGainControl_;
};

#endif
