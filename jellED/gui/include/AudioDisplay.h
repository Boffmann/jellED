#ifndef __AUDIODISPLAY_H__
#define __AUDIODISPLAY_H__

#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include "WaveformWidget.h"
#include "EnvelopePeakWidget.h"
#include "BeatDetectionProcessor.h"

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
    QLabel* envelopeDownsampleRateValueLabel_;
    QSlider* envelopeDownsampleRateSlider_;
    QLabel* downsampleCutoffFrequencyValueLabel_;
    QSlider* downsampleCutoffFrequencySlider_;

    QLineEdit* automaticGainControlTargetLevelTextField_;
    QLineEdit* noveltyGainTextField_;

    QLineEdit* peakDetectionAbsoluteMinThresholdTextField_;
    QLineEdit* peakDetectionThresholdRelTextField_;
    QLineEdit* peakDetectionMinPeakDistanceTextField_;
    QLineEdit* peakDetectionMaxBpmTextField_;

    BeatIndicatorWidget* beatIndicatorWidget_;
    
    int sampleRate_;
    jellED::UsbMicro* usbMicro_;
    const int displaySeconds_;
    const int refreshRate_;
    uint64_t currentSamplesReceived_;
    uint64_t totalSamplesReceived_;
    double currentDetectedBpm_;

    void setupUi();
    QWidget* setupInfoPanel();
    QWidget* setupParameterControls();
    QWidget* setupPeakDetectionControls();
    QWidget* setupWaveformDisplays();
    void setupStatusBar();

private slots:
    void onClearClicked();
    void onStartStopClicked();
    void updateDisplay();
    void updateStatusBar();
    void onEnvelopeDownsampleRateSliderChanged(int value);
    void onDownsampleCutoffFrequencySliderChanged(int value);
    void onApplyButtonClicked();

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
    void addCombinedPeak();
    void addCurrentDetectedBpm(const double bpm);
};

#endif