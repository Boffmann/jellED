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
#include "LedStripWidget.h"
#include "pattern_colors.h"

class QPushButton;
class QComboBox;
class QCheckBox;
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

    // OSF column — mirrors the per-band layout. Row 1 echoes the raw audio
    // (same data as the per-band columns); row 2 shows the pre-smoother
    // weighted-novelty sum; row 3 shows the smoothed OSF + threshold + beats.
    WaveformWidget* originalSamplesWaveformWidgetOsf_;
    WaveformWidget* osfInstantaneousWaveformWidget_;
    EnvelopePeakWidget* osfEnvelopePeakWaveformWidget_;

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
    LedStripWidget*      ledStripWidget_;
    QComboBox*           patternSelector_;
    QCheckBox*           reactToBeatCheckbox_;
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

    // OSF column data feeds (called per envelope frame from BeatDetectionProcessor).
    void addOsfInstantaneousSample(double sample);
    void addOsfSmoothedSample(double sample);
    void setOsfThreshold(double threshold);
    void addOsfPeak();
    void addCurrentDetectedBpm(const double bpm);

    void setVolumeLow(double volume);
    void setVolumeMid(double volume);
    void setVolumeHigh(double volume);
    void setOverallVolume(double volume);
    void setVolumeTrend(double trend);
    void setSpectralTilt(double tilt);

    // Thread-safe — forwards to LedStripWidget::setColors.
    void setLedStripColors(const jellED::pattern_color* colors, int count);

private slots:
    void onPatternSelectionChanged(int index);
    void onReactToBeatToggled(bool checked);
};

#endif
