#ifndef __AUDIODISPLAY_H__
#define __AUDIODISPLAY_H__

#include <QMainWindow>
#include <QLabel>
#include <atomic>
#include "WaveformWidget.h"
#include "EnvelopePeakWidget.h"
#include "BeatDetectionProcessor.h"
#include "ConfiguratorWindow.h"
#include "VolumeDisplayWidget.h"
#include "BipolarLedWidget.h"

class QPushButton;
class BeatIndicatorWidget;

class WaveformProcessor : public QThread {
    Q_OBJECT

public:
    WaveformProcessor(int refreshRate, QObject* parent = nullptr)
        : QThread(parent),
        refreshRate_(refreshRate),
        shouldStop_(false)
    {}

    void stop() {
        shouldStop_ = true;
    }

signals:
    void displayDataReady();

protected:
    void run() override {
        while (!shouldStop_) {
            emit displayDataReady();
            QThread::msleep(1000 / refreshRate_);
        }
    }
private:
    const int refreshRate_;
    bool shouldStop_;
};

class AudioDisplay : public QMainWindow {
    Q_OBJECT

private:
    int updateRotation_;

    QPushButton* clearButton_;
    QPushButton* startStopButton_;
    QPushButton* muteButton_;
    QPushButton* configureButton_;
    
    WaveformWidget* originalSamplesWaveformWidgetLow_;
    WaveformWidget* lowpassFilteredWaveformWidgetLow_;
    EnvelopePeakWidget* envelopePeakWaveformWidgetLow_;

    WaveformWidget* originalSamplesWaveformWidgetMid_;
    WaveformWidget* lowpassFilteredWaveformWidgetMid_;
    EnvelopePeakWidget* envelopePeakWaveformWidgetMid_;

    WaveformWidget* originalSamplesWaveformWidgetHigh_;
    WaveformWidget* lowpassFilteredWaveformWidgetHigh_;
    EnvelopePeakWidget* envelopePeakWaveformWidgetHigh_;

    WaveformProcessor* processorThread_;
    BeatDetectionProcessor* beatDetectionProcessor_;
    QLabel* infoLabel_;
    QLabel* statusLabel_;

    BeatIndicatorWidget* beatIndicatorWidget_;
    VolumeDisplayWidget* volumeLowWidget_;
    VolumeDisplayWidget* volumeMidWidget_;
    VolumeDisplayWidget* volumeHighWidget_;
    VolumeDisplayWidget* volumeOverallWidget_;
    BipolarLedWidget*    volumeTrendWidget_;
    BipolarLedWidget*    spectralTiltWidget_;
    ConfiguratorWindow* configuratorWindow_;

    std::atomic<double> currentVolumeLow_;
    std::atomic<double> currentVolumeMid_;
    std::atomic<double> currentVolumeHigh_;
    std::atomic<double> currentOverallVolume_;
    std::atomic<double> currentVolumeTrend_;
    std::atomic<double> currentSpectralTilt_;

    int sampleRate_;
    jellED::SoundInput* soundInput_;
    AudioInputMode currentInputMode_;
    std::string currentSourcePath_;
    const int displaySeconds_;
    const int refreshRate_;
    uint64_t currentSamplesReceived_;
    uint64_t totalSamplesReceived_;
    double currentDetectedBpm_;

    void setupUi();
    QWidget* setupInfoPanel();
    QWidget* setupWaveformDisplays();
    void setupStatusBar();

private slots:
    void onClearClicked();
    void onStartStopClicked();
    void onMuteClicked();
    void updateDisplay();
    void updateStatusBar();
    void onConfigureClicked();
    void onApplyConfig(const jellED::BeatDetectionConfig& config);

public:
    AudioDisplay(std::string microphone_device_id, int displaySeconds = 10, int refreshRate = 30, QWidget* parent = nullptr);
    ~AudioDisplay();

    void startBeatDetectionProcessor();

    void addOriginalSample(const double sample);
    void addLowpassFilteredSampleLow(const double sample);
    void addEnvelopeFilteredSampleLow(const double sample);
    void addLowpassFilteredSampleMid(const double sample);
    void addEnvelopeFilteredSampleMid(const double sample);
    void addLowpassFilteredSampleHigh(const double sample);
    void addEnvelopeFilteredSampleHigh(const double sample);
    void addPeakLow();
    void addPeakMid();
    void addPeakHigh();
    void setThresholdLow(double threshold);
    void setThresholdMid(double threshold);
    void setThresholdHigh(double threshold);
    void addCombinedPeak();
    void addCurrentDetectedBpm(const double bpm);

    void setVolumeLow(double volume);
    void setVolumeMid(double volume);
    void setVolumeHigh(double volume);
    void setOverallVolume(double volume);
    void setVolumeTrend(double trend);
    void setSpectralTilt(double tilt);
};

#endif
