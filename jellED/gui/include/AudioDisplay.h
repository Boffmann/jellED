#ifndef __AUDIODISPLAY_H__
#define __AUDIODISPLAY_H__

#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include "WaveformWidget.h"
#include "EnvelopePeakWidget.h"
#include "BeatDetectionProcessor.h"

class QPushButton;

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
    QPushButton* clearButton_;
    WaveformWidget* originalSamplesWaveformWidget_;
    WaveformWidget* lowpassFilteredWaveformWidget_;
    EnvelopePeakWidget* envelopePeakWaveformWidget_;
    WaveformProcessor* processorThread_;
    BeatDetectionProcessor* beatDetectionProcessor_;
    QLabel* infoLabel_;
    QLabel* statusLabel_;
    QLabel* downsampleRateValueLabel_;
    QLabel* envelopeDownsampleRateValueLabel_;
    QSlider* downsampleRateSlider_;
    QSlider* envelopeDownsampleRateSlider_;
    
    int sampleRate_;
    jellED::UsbMicro* usbMicro_;
    const int displaySeconds_;
    const int refreshRate_;
    uint64_t currentSamplesReceived_;
    uint64_t totalSamplesReceived_;

    void setupUi();
    QWidget* setupInfoPanel();
    QWidget* setupParameterControls();
    QWidget* setupWaveformDisplays();
    void setupStatusBar();

private slots:
    void onClearClicked();
    void updateDisplay();
    void updateStatusBar();
    void onDownsampleRateSliderChanged(int value);
    void onEnvelopeDownsampleRateSliderChanged(int value);
    void onApplyButtonClicked();

public:
    AudioDisplay(std::string microphone_device_id, int displaySeconds = 10, int refreshRate = 30, QWidget* parent = nullptr);
    ~AudioDisplay();

    void startBeatDetectionProcessor();

    void addOriginalSample(const double sample);
    void addLowpassFilteredSample(const double sample);
    void addEnvelopeFilteredSample(const double sample);
    void addPeak();
};

#endif