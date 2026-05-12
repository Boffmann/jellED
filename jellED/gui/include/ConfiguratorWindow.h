#ifndef __CONFIGURATOR_WINDOW_H__
#define __CONFIGURATOR_WINDOW_H__

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QStackedWidget>
#include <QScrollArea>

#include "include/beatDetectionConfig.h"

enum class AudioInputMode {
    Microphone,
    WavFile
};

class ConfiguratorWindow : public QWidget {
    Q_OBJECT

public:
    explicit ConfiguratorWindow(QWidget* parent = nullptr);

    jellED::BeatDetectionConfig currentConfig() const;
    void applyConfigToUi(const jellED::BeatDetectionConfig& config);
    AudioInputMode currentAudioInputMode() const;
    QString currentAudioSource() const;
    QString currentOutputDevice() const;
    void setWavFilePath(const QString& fullPath);

signals:
    void applyRequested(const jellED::BeatDetectionConfig& config);

private:
    QRadioButton* microphoneRadioButton_;
    QRadioButton* wavFileRadioButton_;
    QStackedWidget* audioInputStack_;
    QComboBox* microphoneComboBox_;
    QLineEdit* wavFilePathEdit_;
    QString wavFilePath_;
    QComboBox* outputDeviceComboBox_;

    QLabel* envelopeDownsampleRateValueLabel_;
    QSlider* envelopeDownsampleRateSlider_;
    QLabel* downsampleCutoffFrequencyValueLabel_;
    QSlider* downsampleCutoffFrequencySlider_;

    QLineEdit* automaticGainControlTargetLevelTextField_;
    QLineEdit* noiseGateThresholdTextField_;

    QLineEdit* absoluteMinThresholdLowTextField_;
    QLineEdit* absoluteMinThresholdMidTextField_;
    QLineEdit* absoluteMinThresholdHighTextField_;
    QLineEdit* thresholdRelLowTextField_;
    QLineEdit* thresholdRelMidTextField_;
    QLineEdit* thresholdRelHighTextField_;
    QLineEdit* peakDetectionMaxBpmTextField_;

    QLineEdit* bandWeightLowTextField_;
    QLineEdit* bandWeightMidTextField_;
    QLineEdit* bandWeightHighTextField_;

    QLineEdit* envelopeAttackTimeLowTextField_;
    QLineEdit* envelopeAttackTimeMidTextField_;
    QLineEdit* envelopeAttackTimeHighTextField_;
    QLineEdit* envelopeReleaseTimeLowTextField_;
    QLineEdit* envelopeReleaseTimeMidTextField_;
    QLineEdit* envelopeReleaseTimeHighTextField_;

    QLineEdit* coincidenceWindowTextField_;

    QCheckBox* useTempoLockCheckBox_;
    QLineEdit* tempoLockMinBpmTextField_;
    QLineEdit* tempoLockMaxBpmTextField_;
    QLineEdit* tempoLockToleranceTextField_;
    QLineEdit* tempoLockStaleResetTimeTextField_;

    // Peak detector per-band timing
    QLineEdit* baselineAttackTimeLowTextField_;
    QLineEdit* baselineAttackTimeMidTextField_;
    QLineEdit* baselineAttackTimeHighTextField_;
    QLineEdit* baselineReleaseTimeLowTextField_;
    QLineEdit* baselineReleaseTimeMidTextField_;
    QLineEdit* baselineReleaseTimeHighTextField_;
    QLineEdit* thresholdRelaxTimeLowTextField_;
    QLineEdit* thresholdRelaxTimeMidTextField_;
    QLineEdit* thresholdRelaxTimeHighTextField_;
    QLineEdit* onsetRatioLowTextField_;
    QLineEdit* onsetRatioMidTextField_;
    QLineEdit* onsetRatioHighTextField_;

    // Peak detector global tuning
    QLineEdit* minRelativeThresholdFactorTextField_;
    QLineEdit* risingThresholdScaleTextField_;
    QLineEdit* fallingThresholdScaleTextField_;

    // Moving-mean threshold (Böck 2012)
    QComboBox* thresholdModeComboBox_;
    QLineEdit* thresholdWindowMsLowTextField_;
    QLineEdit* thresholdWindowMsMidTextField_;
    QLineEdit* thresholdWindowMsHighTextField_;
    QLineEdit* thresholdDeltaTextField_;
    QLineEdit* osfThresholdWindowMsTextField_;
    QLineEdit* osfThresholdDeltaTextField_;

    // OSF (Onset Strength Function) fusion
    QCheckBox* useOsfFusionCheckBox_;
    QCheckBox* osfUseAdaptiveWhiteningCheckBox_;
    QCheckBox* osfSpectralTiltWeightingCheckBox_;
    QLineEdit* osfBaselineAttackTimeTextField_;
    QLineEdit* osfBaselineReleaseTimeTextField_;
    QLineEdit* osfWhiteningTimeTextField_;
    QLineEdit* osfWhiteningFloorTextField_;
    QLineEdit* osfSmoothingTimeTextField_;
    QLineEdit* osfAbsoluteMinThresholdTextField_;
    QLineEdit* osfThresholdRelTextField_;
    QLineEdit* osfOnsetRatioTextField_;
    QLineEdit* osfBaselineAttackTimeFinalTextField_;
    QLineEdit* osfBaselineReleaseTimeFinalTextField_;
    QLineEdit* osfThresholdRelaxTimeTextField_;
    QLineEdit* osfTiltGainTextField_;
    QLineEdit* osfOverallLevelGateTextField_;

    void setupUi();
    QWidget* setupAudioInputControls();
    QWidget* setupAudioOutputControls();
    QWidget* setupEnvelopeControls();
    QWidget* setupDownsampleControls();
    QWidget* setupGainControls();
    QWidget* setupMaxBpmControls();
    QWidget* setupPeakDetectionControls();
    QWidget* setupPeakDetectorTimingControls();
    QWidget* setupPeakDetectorTuningControls();
    QWidget* setupBandWeightControls();
    QWidget* setupEnvelopeTimingControls();
    QWidget* setupFusionControls();
    QWidget* setupTempoLockControls();
    QWidget* setupOsfControls();
    QWidget* setupMovingMeanControls();
    QWidget* setupPerBandRow(const QString& title,
        double defaultLow, double defaultMid, double defaultHigh,
        QLineEdit*& lowField, QLineEdit*& midField, QLineEdit*& highField);

private slots:
    void onAudioInputModeChanged();
    void onBrowseWavFileClicked();
    void onEnvelopeDownsampleRateSliderChanged(int value);
    void onDownsampleCutoffFrequencySliderChanged(int value);
    void onApplyButtonClicked();
    void onSaveConfigClicked();
    void onLoadConfigClicked();
};

#endif
