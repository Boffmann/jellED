#ifndef __AUDIODISPLAY_H__
#define __AUDIODISPLAY_H__

#include <QMainWindow>
#include <QLabel>
#include "WaveformWidget.h"

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
    WaveformProcessor* processorThread_;
    QLabel* statusLabel_;
    const int sampleRate_;
    const int displaySeconds_;
    const int refreshRate_;
    uint64_t currentSamplesReceived_;
    uint64_t totalSamplesReceived_;

    void setupUi();

private slots:
    void onClearClicked();
    void updateDisplay();
    void updateStatusBar();

public:
    AudioDisplay(int sampleRate = 44100, int displaySeconds = 10, int refreshRate = 30, QWidget* parent = nullptr);
    ~AudioDisplay();

    void addOriginalSample(const double sample);
    void addLowpassFilteredSample(const double sample);
};

#endif