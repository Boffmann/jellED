#ifndef __BEAT_DETECTION_PROCESSOR_H__
#define __BEAT_DETECTION_PROCESSOR_H__

#include <QThread>
#include <atomic>
#include <memory>
#include <stdexcept>

#include "beatdetection.h"
#include "include/downsampler.h"
#include "include/noiseGate.h"
#include "sound/soundinput.h"

#include "GuiPlatformUtils.h"
#include "patternEngine.h"
#include "patternType.h"

class AudioDisplay;

class BeatDetectionProcessor : public QThread {
    Q_OBJECT

public:
    explicit BeatDetectionProcessor(
        AudioDisplay* display,
        jellED::SoundInput* soundInput,
        const jellED::BeatDetectionConfig& config,
        int signalDownsampleRatio,
        int numLeds,
        QObject* parent);

    void stop() {
        shouldStop_ = true;
    }

    void start() {
        shouldStop_ = false;
        QThread::start();
    }

    // Thread-safe. Both values are read at the top of the next pattern tick,
    // so there's no locking required on the audio thread.
    void selectPattern(jellED::PatternType t) { selectedPatternType_.store(t); }
    void setReactToBeat(bool on)              { reactToBeat_.store(on); }

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

    jellED::GuiPlatformUtils platformUtils_;
    std::unique_ptr<jellED::PatternEngine> patternEngine_;
    int numLeds_;

    std::atomic<jellED::PatternType> selectedPatternType_;
    std::atomic<bool> reactToBeat_;
};

#endif
