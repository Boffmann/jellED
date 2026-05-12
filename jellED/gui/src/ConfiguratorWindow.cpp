#include "ConfiguratorWindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>

#include <cmath>

#include "sound/raspi/usbMicro.h"

static const QString CONFIG_FILE_FILTER = "jellED Config (*.json);;All Files (*)";

static const jellED::BeatDetectionConfig DEFAULTS;

static const QString STYLE_LINEEDIT =
    "QLineEdit { background-color: #E8E8E8; border: 1px solid #BBBBBB; border-radius: 3px; padding: 2px 4px; }";

static const QString STYLE_GENERAL =
    "QGroupBox { border: 2px solid #5B8DBE; border-radius: 4px; margin-top: 0.8em; font-weight: bold; }"
    "QGroupBox::title { color: #5B8DBE; subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    + STYLE_LINEEDIT;

static const QString STYLE_ENVELOPE =
    "QGroupBox { border: 2px solid #4CAF50; border-radius: 4px; margin-top: 0.8em; font-weight: bold; }"
    "QGroupBox::title { color: #4CAF50; subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    + STYLE_LINEEDIT;

static const QString STYLE_PEAK =
    "QGroupBox { border: 2px solid #E67E22; border-radius: 4px; margin-top: 0.8em; font-weight: bold; }"
    "QGroupBox::title { color: #E67E22; subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    + STYLE_LINEEDIT;

static QJsonObject configToJson(const jellED::BeatDetectionConfig& c) {
    QJsonObject obj;
    obj["envelopeDownsampleRatio"] = c.envelopeDownsampleRatio;
    obj["downsampleCutoffFrequency"] = c.downsampleCutoffFrequency;
    obj["automaticGainControlTargetLevel"] = c.automaticGainControlTargetLevel;
    obj["noiseGateThreshold"] = c.noiseGateThreshold;
    obj["absoluteMinThresholdLow"] = c.absoluteMinThresholdLow;
    obj["absoluteMinThresholdMid"] = c.absoluteMinThresholdMid;
    obj["absoluteMinThresholdHigh"] = c.absoluteMinThresholdHigh;
    obj["thresholdRelLow"] = c.thresholdRelLow;
    obj["thresholdRelMid"] = c.thresholdRelMid;
    obj["thresholdRelHigh"] = c.thresholdRelHigh;
    obj["peakDetectionMaxBpm"] = c.peakDetectionMaxBpm;
    obj["bandWeightLow"] = c.bandWeightLow;
    obj["bandWeightMid"] = c.bandWeightMid;
    obj["bandWeightHigh"] = c.bandWeightHigh;
    obj["envelopeAttackTimeLow"] = c.envelopeAttackTimeLow;
    obj["envelopeAttackTimeMid"] = c.envelopeAttackTimeMid;
    obj["envelopeAttackTimeHigh"] = c.envelopeAttackTimeHigh;
    obj["envelopeReleaseTimeLow"] = c.envelopeReleaseTimeLow;
    obj["envelopeReleaseTimeMid"] = c.envelopeReleaseTimeMid;
    obj["envelopeReleaseTimeHigh"] = c.envelopeReleaseTimeHigh;
    obj["coincidenceWindow"] = c.coincidenceWindow;
    obj["baselineAttackTimeLow"] = c.baselineAttackTimeLow;
    obj["baselineAttackTimeMid"] = c.baselineAttackTimeMid;
    obj["baselineAttackTimeHigh"] = c.baselineAttackTimeHigh;
    obj["baselineReleaseTimeLow"] = c.baselineReleaseTimeLow;
    obj["baselineReleaseTimeMid"] = c.baselineReleaseTimeMid;
    obj["baselineReleaseTimeHigh"] = c.baselineReleaseTimeHigh;
    obj["thresholdRelaxTimeLow"] = c.thresholdRelaxTimeLow;
    obj["thresholdRelaxTimeMid"] = c.thresholdRelaxTimeMid;
    obj["thresholdRelaxTimeHigh"] = c.thresholdRelaxTimeHigh;
    obj["onsetRatioLow"] = c.onsetRatioLow;
    obj["onsetRatioMid"] = c.onsetRatioMid;
    obj["onsetRatioHigh"] = c.onsetRatioHigh;
    obj["minRelativeThresholdFactor"] = c.minRelativeThresholdFactor;
    obj["risingThresholdScale"] = c.risingThresholdScale;
    obj["fallingThresholdScale"] = c.fallingThresholdScale;
    obj["useTempoLock"] = c.useTempoLock;
    obj["tempoLockMinBpm"] = c.tempoLockMinBpm;
    obj["tempoLockMaxBpm"] = c.tempoLockMaxBpm;
    obj["tempoLockTolerance"] = c.tempoLockTolerance;
    obj["tempoLockStaleResetTime"] = c.tempoLockStaleResetTime;
    obj["useOsfFusion"] = c.useOsfFusion;
    obj["osfBaselineAttackTime"] = c.osfBaselineAttackTime;
    obj["osfBaselineReleaseTime"] = c.osfBaselineReleaseTime;
    obj["osfUseAdaptiveWhitening"] = c.osfUseAdaptiveWhitening;
    obj["osfWhiteningTime"] = c.osfWhiteningTime;
    obj["osfWhiteningFloor"] = c.osfWhiteningFloor;
    obj["osfSmoothingTime"] = c.osfSmoothingTime;
    obj["osfAbsoluteMinThreshold"] = c.osfAbsoluteMinThreshold;
    obj["osfThresholdRel"] = c.osfThresholdRel;
    obj["osfOnsetRatio"] = c.osfOnsetRatio;
    obj["osfBaselineAttackTimeFinal"] = c.osfBaselineAttackTimeFinal;
    obj["osfBaselineReleaseTimeFinal"] = c.osfBaselineReleaseTimeFinal;
    obj["osfThresholdRelaxTime"] = c.osfThresholdRelaxTime;
    obj["osfSpectralTiltWeighting"] = c.osfSpectralTiltWeighting;
    obj["osfTiltGain"] = c.osfTiltGain;
    obj["osfOverallLevelGate"] = c.osfOverallLevelGate;
    obj["thresholdMode"] = static_cast<int>(c.thresholdMode);
    obj["thresholdWindowMsLow"] = c.thresholdWindowMsLow;
    obj["thresholdWindowMsMid"] = c.thresholdWindowMsMid;
    obj["thresholdWindowMsHigh"] = c.thresholdWindowMsHigh;
    obj["thresholdDelta"] = c.thresholdDelta;
    obj["osfThresholdWindowMs"] = c.osfThresholdWindowMs;
    obj["osfThresholdDelta"] = c.osfThresholdDelta;
    return obj;
}

static bool jsonToConfig(const QJsonObject& obj, jellED::BeatDetectionConfig& c) {
    auto getDouble = [&obj](const char* key, double defaultVal) -> double {
        if (!obj.contains(key)) return defaultVal;
        QJsonValue v = obj[key];
        if (v.isDouble()) return v.toDouble();
        if (v.isString()) return v.toString().toDouble();
        return defaultVal;
    };
    auto getInt = [&obj](const char* key, int defaultVal) -> int {
        if (!obj.contains(key)) return defaultVal;
        QJsonValue v = obj[key];
        if (v.isDouble()) return static_cast<int>(v.toDouble());
        if (v.isString()) return v.toString().toInt();
        return defaultVal;
    };

    c.envelopeDownsampleRatio = getInt("envelopeDownsampleRatio", DEFAULTS.envelopeDownsampleRatio);
    c.downsampleCutoffFrequency = getDouble("downsampleCutoffFrequency", DEFAULTS.downsampleCutoffFrequency);
    c.automaticGainControlTargetLevel = getDouble("automaticGainControlTargetLevel", DEFAULTS.automaticGainControlTargetLevel);
    c.noiseGateThreshold = getDouble("noiseGateThreshold", DEFAULTS.noiseGateThreshold);
    c.absoluteMinThresholdLow = getDouble("absoluteMinThresholdLow", DEFAULTS.absoluteMinThresholdLow);
    c.absoluteMinThresholdMid = getDouble("absoluteMinThresholdMid", DEFAULTS.absoluteMinThresholdMid);
    c.absoluteMinThresholdHigh = getDouble("absoluteMinThresholdHigh", DEFAULTS.absoluteMinThresholdHigh);
    c.thresholdRelLow = getDouble("thresholdRelLow", DEFAULTS.thresholdRelLow);
    c.thresholdRelMid = getDouble("thresholdRelMid", DEFAULTS.thresholdRelMid);
    c.thresholdRelHigh = getDouble("thresholdRelHigh", DEFAULTS.thresholdRelHigh);
    c.peakDetectionMaxBpm = getDouble("peakDetectionMaxBpm", DEFAULTS.peakDetectionMaxBpm);
    c.bandWeightLow = getDouble("bandWeightLow", DEFAULTS.bandWeightLow);
    c.bandWeightMid = getDouble("bandWeightMid", DEFAULTS.bandWeightMid);
    c.bandWeightHigh = getDouble("bandWeightHigh", DEFAULTS.bandWeightHigh);
    c.envelopeAttackTimeLow = getDouble("envelopeAttackTimeLow", DEFAULTS.envelopeAttackTimeLow);
    c.envelopeAttackTimeMid = getDouble("envelopeAttackTimeMid", DEFAULTS.envelopeAttackTimeMid);
    c.envelopeAttackTimeHigh = getDouble("envelopeAttackTimeHigh", DEFAULTS.envelopeAttackTimeHigh);
    c.envelopeReleaseTimeLow = getDouble("envelopeReleaseTimeLow", DEFAULTS.envelopeReleaseTimeLow);
    c.envelopeReleaseTimeMid = getDouble("envelopeReleaseTimeMid", DEFAULTS.envelopeReleaseTimeMid);
    c.envelopeReleaseTimeHigh = getDouble("envelopeReleaseTimeHigh", DEFAULTS.envelopeReleaseTimeHigh);
    c.coincidenceWindow = getDouble("coincidenceWindow", DEFAULTS.coincidenceWindow);
    c.baselineAttackTimeLow = getDouble("baselineAttackTimeLow", DEFAULTS.baselineAttackTimeLow);
    c.baselineAttackTimeMid = getDouble("baselineAttackTimeMid", DEFAULTS.baselineAttackTimeMid);
    c.baselineAttackTimeHigh = getDouble("baselineAttackTimeHigh", DEFAULTS.baselineAttackTimeHigh);
    c.baselineReleaseTimeLow = getDouble("baselineReleaseTimeLow", DEFAULTS.baselineReleaseTimeLow);
    c.baselineReleaseTimeMid = getDouble("baselineReleaseTimeMid", DEFAULTS.baselineReleaseTimeMid);
    c.baselineReleaseTimeHigh = getDouble("baselineReleaseTimeHigh", DEFAULTS.baselineReleaseTimeHigh);
    c.thresholdRelaxTimeLow = getDouble("thresholdRelaxTimeLow", DEFAULTS.thresholdRelaxTimeLow);
    c.thresholdRelaxTimeMid = getDouble("thresholdRelaxTimeMid", DEFAULTS.thresholdRelaxTimeMid);
    c.thresholdRelaxTimeHigh = getDouble("thresholdRelaxTimeHigh", DEFAULTS.thresholdRelaxTimeHigh);
    c.onsetRatioLow = getDouble("onsetRatioLow", DEFAULTS.onsetRatioLow);
    c.onsetRatioMid = getDouble("onsetRatioMid", DEFAULTS.onsetRatioMid);
    c.onsetRatioHigh = getDouble("onsetRatioHigh", DEFAULTS.onsetRatioHigh);
    c.minRelativeThresholdFactor = getDouble("minRelativeThresholdFactor", DEFAULTS.minRelativeThresholdFactor);
    c.risingThresholdScale = getDouble("risingThresholdScale", DEFAULTS.risingThresholdScale);
    c.fallingThresholdScale = getDouble("fallingThresholdScale", DEFAULTS.fallingThresholdScale);
    auto getBool = [&obj](const char* key, bool defaultVal) -> bool {
        if (!obj.contains(key)) return defaultVal;
        QJsonValue v = obj[key];
        if (v.isBool()) return v.toBool();
        if (v.isDouble()) return v.toDouble() != 0.0;
        if (v.isString()) return v.toString().compare("true", Qt::CaseInsensitive) == 0;
        return defaultVal;
    };
    c.useTempoLock = getBool("useTempoLock", DEFAULTS.useTempoLock);
    c.tempoLockMinBpm = getDouble("tempoLockMinBpm", DEFAULTS.tempoLockMinBpm);
    c.tempoLockMaxBpm = getDouble("tempoLockMaxBpm", DEFAULTS.tempoLockMaxBpm);
    c.tempoLockTolerance = getDouble("tempoLockTolerance", DEFAULTS.tempoLockTolerance);
    c.tempoLockStaleResetTime = getDouble("tempoLockStaleResetTime", DEFAULTS.tempoLockStaleResetTime);

    c.useOsfFusion = getBool("useOsfFusion", DEFAULTS.useOsfFusion);
    c.osfBaselineAttackTime = getDouble("osfBaselineAttackTime", DEFAULTS.osfBaselineAttackTime);
    c.osfBaselineReleaseTime = getDouble("osfBaselineReleaseTime", DEFAULTS.osfBaselineReleaseTime);
    c.osfUseAdaptiveWhitening = getBool("osfUseAdaptiveWhitening", DEFAULTS.osfUseAdaptiveWhitening);
    c.osfWhiteningTime = getDouble("osfWhiteningTime", DEFAULTS.osfWhiteningTime);
    c.osfWhiteningFloor = getDouble("osfWhiteningFloor", DEFAULTS.osfWhiteningFloor);
    c.osfSmoothingTime = getDouble("osfSmoothingTime", DEFAULTS.osfSmoothingTime);
    c.osfAbsoluteMinThreshold = getDouble("osfAbsoluteMinThreshold", DEFAULTS.osfAbsoluteMinThreshold);
    c.osfThresholdRel = getDouble("osfThresholdRel", DEFAULTS.osfThresholdRel);
    c.osfOnsetRatio = getDouble("osfOnsetRatio", DEFAULTS.osfOnsetRatio);
    c.osfBaselineAttackTimeFinal = getDouble("osfBaselineAttackTimeFinal", DEFAULTS.osfBaselineAttackTimeFinal);
    c.osfBaselineReleaseTimeFinal = getDouble("osfBaselineReleaseTimeFinal", DEFAULTS.osfBaselineReleaseTimeFinal);
    c.osfThresholdRelaxTime = getDouble("osfThresholdRelaxTime", DEFAULTS.osfThresholdRelaxTime);
    c.osfSpectralTiltWeighting = getBool("osfSpectralTiltWeighting", DEFAULTS.osfSpectralTiltWeighting);
    c.osfTiltGain = getDouble("osfTiltGain", DEFAULTS.osfTiltGain);
    c.osfOverallLevelGate = getDouble("osfOverallLevelGate", DEFAULTS.osfOverallLevelGate);
    c.thresholdMode = static_cast<jellED::ThresholdMode>(
        getInt("thresholdMode", static_cast<int>(DEFAULTS.thresholdMode)));
    c.thresholdWindowMsLow  = getDouble("thresholdWindowMsLow",  DEFAULTS.thresholdWindowMsLow);
    c.thresholdWindowMsMid  = getDouble("thresholdWindowMsMid",  DEFAULTS.thresholdWindowMsMid);
    c.thresholdWindowMsHigh = getDouble("thresholdWindowMsHigh", DEFAULTS.thresholdWindowMsHigh);
    c.thresholdDelta        = getDouble("thresholdDelta",        DEFAULTS.thresholdDelta);
    c.osfThresholdWindowMs  = getDouble("osfThresholdWindowMs",  DEFAULTS.osfThresholdWindowMs);
    c.osfThresholdDelta     = getDouble("osfThresholdDelta",     DEFAULTS.osfThresholdDelta);
    return true;
}

ConfiguratorWindow::ConfiguratorWindow(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("jellED - Configuration");
    resize(1200, 1000);
    setupUi();
}

jellED::BeatDetectionConfig ConfiguratorWindow::currentConfig() const {
    jellED::BeatDetectionConfig config;
    config.envelopeDownsampleRatio = 1 << envelopeDownsampleRateSlider_->value();
    config.downsampleCutoffFrequency = 0.1 * downsampleCutoffFrequencySlider_->value();
    config.automaticGainControlTargetLevel = automaticGainControlTargetLevelTextField_->text().toDouble();
    config.noiseGateThreshold = noiseGateThresholdTextField_->text().toDouble();
    config.absoluteMinThresholdLow = absoluteMinThresholdLowTextField_->text().toDouble();
    config.absoluteMinThresholdMid = absoluteMinThresholdMidTextField_->text().toDouble();
    config.absoluteMinThresholdHigh = absoluteMinThresholdHighTextField_->text().toDouble();
    config.thresholdRelLow = thresholdRelLowTextField_->text().toDouble();
    config.thresholdRelMid = thresholdRelMidTextField_->text().toDouble();
    config.thresholdRelHigh = thresholdRelHighTextField_->text().toDouble();
    config.peakDetectionMaxBpm = peakDetectionMaxBpmTextField_->text().toDouble();
    config.bandWeightLow = bandWeightLowTextField_->text().toDouble();
    config.bandWeightMid = bandWeightMidTextField_->text().toDouble();
    config.bandWeightHigh = bandWeightHighTextField_->text().toDouble();
    config.envelopeAttackTimeLow = envelopeAttackTimeLowTextField_->text().toDouble();
    config.envelopeAttackTimeMid = envelopeAttackTimeMidTextField_->text().toDouble();
    config.envelopeAttackTimeHigh = envelopeAttackTimeHighTextField_->text().toDouble();
    config.envelopeReleaseTimeLow = envelopeReleaseTimeLowTextField_->text().toDouble();
    config.envelopeReleaseTimeMid = envelopeReleaseTimeMidTextField_->text().toDouble();
    config.envelopeReleaseTimeHigh = envelopeReleaseTimeHighTextField_->text().toDouble();
    config.coincidenceWindow = coincidenceWindowTextField_->text().toDouble();

    config.baselineAttackTimeLow = baselineAttackTimeLowTextField_->text().toDouble();
    config.baselineAttackTimeMid = baselineAttackTimeMidTextField_->text().toDouble();
    config.baselineAttackTimeHigh = baselineAttackTimeHighTextField_->text().toDouble();
    config.baselineReleaseTimeLow = baselineReleaseTimeLowTextField_->text().toDouble();
    config.baselineReleaseTimeMid = baselineReleaseTimeMidTextField_->text().toDouble();
    config.baselineReleaseTimeHigh = baselineReleaseTimeHighTextField_->text().toDouble();
    config.thresholdRelaxTimeLow = thresholdRelaxTimeLowTextField_->text().toDouble();
    config.thresholdRelaxTimeMid = thresholdRelaxTimeMidTextField_->text().toDouble();
    config.thresholdRelaxTimeHigh = thresholdRelaxTimeHighTextField_->text().toDouble();
    config.onsetRatioLow = onsetRatioLowTextField_->text().toDouble();
    config.onsetRatioMid = onsetRatioMidTextField_->text().toDouble();
    config.onsetRatioHigh = onsetRatioHighTextField_->text().toDouble();

    config.minRelativeThresholdFactor = minRelativeThresholdFactorTextField_->text().toDouble();
    config.risingThresholdScale = risingThresholdScaleTextField_->text().toDouble();
    config.fallingThresholdScale = fallingThresholdScaleTextField_->text().toDouble();

    config.useTempoLock = useTempoLockCheckBox_->isChecked();
    config.tempoLockMinBpm = tempoLockMinBpmTextField_->text().toDouble();
    config.tempoLockMaxBpm = tempoLockMaxBpmTextField_->text().toDouble();
    config.tempoLockTolerance = tempoLockToleranceTextField_->text().toDouble();
    config.tempoLockStaleResetTime = tempoLockStaleResetTimeTextField_->text().toDouble();

    config.useOsfFusion = useOsfFusionCheckBox_->isChecked();
    config.osfUseAdaptiveWhitening = osfUseAdaptiveWhiteningCheckBox_->isChecked();
    config.osfSpectralTiltWeighting = osfSpectralTiltWeightingCheckBox_->isChecked();
    config.osfBaselineAttackTime = osfBaselineAttackTimeTextField_->text().toDouble();
    config.osfBaselineReleaseTime = osfBaselineReleaseTimeTextField_->text().toDouble();
    config.osfWhiteningTime = osfWhiteningTimeTextField_->text().toDouble();
    config.osfWhiteningFloor = osfWhiteningFloorTextField_->text().toDouble();
    config.osfSmoothingTime = osfSmoothingTimeTextField_->text().toDouble();
    config.osfAbsoluteMinThreshold = osfAbsoluteMinThresholdTextField_->text().toDouble();
    config.osfThresholdRel = osfThresholdRelTextField_->text().toDouble();
    config.osfOnsetRatio = osfOnsetRatioTextField_->text().toDouble();
    config.osfBaselineAttackTimeFinal = osfBaselineAttackTimeFinalTextField_->text().toDouble();
    config.osfBaselineReleaseTimeFinal = osfBaselineReleaseTimeFinalTextField_->text().toDouble();
    config.osfThresholdRelaxTime = osfThresholdRelaxTimeTextField_->text().toDouble();
    config.osfTiltGain = osfTiltGainTextField_->text().toDouble();
    config.osfOverallLevelGate = osfOverallLevelGateTextField_->text().toDouble();

    config.thresholdMode = static_cast<jellED::ThresholdMode>(thresholdModeComboBox_->currentIndex());
    config.thresholdWindowMsLow  = thresholdWindowMsLowTextField_->text().toDouble();
    config.thresholdWindowMsMid  = thresholdWindowMsMidTextField_->text().toDouble();
    config.thresholdWindowMsHigh = thresholdWindowMsHighTextField_->text().toDouble();
    config.thresholdDelta        = thresholdDeltaTextField_->text().toDouble();
    config.osfThresholdWindowMs  = osfThresholdWindowMsTextField_->text().toDouble();
    config.osfThresholdDelta     = osfThresholdDeltaTextField_->text().toDouble();

    return config;
}

void ConfiguratorWindow::applyConfigToUi(const jellED::BeatDetectionConfig& config) {
    int envelopeSliderPos = 0;
    if (config.envelopeDownsampleRatio >= 1) {
        envelopeSliderPos = std::min(4, static_cast<int>(std::round(std::log2(config.envelopeDownsampleRatio))));
    }
    envelopeDownsampleRateSlider_->setValue(envelopeSliderPos);
    envelopeDownsampleRateValueLabel_->setText(QString::number(config.envelopeDownsampleRatio));

    downsampleCutoffFrequencySlider_->setValue(std::max(0, std::min(10, static_cast<int>(config.downsampleCutoffFrequency * 10))));
    downsampleCutoffFrequencyValueLabel_->setText(QString::number(config.downsampleCutoffFrequency));

    automaticGainControlTargetLevelTextField_->setText(QString::number(config.automaticGainControlTargetLevel));
    noiseGateThresholdTextField_->setText(QString::number(config.noiseGateThreshold));
    absoluteMinThresholdLowTextField_->setText(QString::number(config.absoluteMinThresholdLow));
    absoluteMinThresholdMidTextField_->setText(QString::number(config.absoluteMinThresholdMid));
    absoluteMinThresholdHighTextField_->setText(QString::number(config.absoluteMinThresholdHigh));
    thresholdRelLowTextField_->setText(QString::number(config.thresholdRelLow));
    thresholdRelMidTextField_->setText(QString::number(config.thresholdRelMid));
    thresholdRelHighTextField_->setText(QString::number(config.thresholdRelHigh));
    peakDetectionMaxBpmTextField_->setText(QString::number(config.peakDetectionMaxBpm));
    bandWeightLowTextField_->setText(QString::number(config.bandWeightLow));
    bandWeightMidTextField_->setText(QString::number(config.bandWeightMid));
    bandWeightHighTextField_->setText(QString::number(config.bandWeightHigh));
    envelopeAttackTimeLowTextField_->setText(QString::number(config.envelopeAttackTimeLow));
    envelopeAttackTimeMidTextField_->setText(QString::number(config.envelopeAttackTimeMid));
    envelopeAttackTimeHighTextField_->setText(QString::number(config.envelopeAttackTimeHigh));
    envelopeReleaseTimeLowTextField_->setText(QString::number(config.envelopeReleaseTimeLow));
    envelopeReleaseTimeMidTextField_->setText(QString::number(config.envelopeReleaseTimeMid));
    envelopeReleaseTimeHighTextField_->setText(QString::number(config.envelopeReleaseTimeHigh));
    coincidenceWindowTextField_->setText(QString::number(config.coincidenceWindow));
    baselineAttackTimeLowTextField_->setText(QString::number(config.baselineAttackTimeLow));
    baselineAttackTimeMidTextField_->setText(QString::number(config.baselineAttackTimeMid));
    baselineAttackTimeHighTextField_->setText(QString::number(config.baselineAttackTimeHigh));
    baselineReleaseTimeLowTextField_->setText(QString::number(config.baselineReleaseTimeLow));
    baselineReleaseTimeMidTextField_->setText(QString::number(config.baselineReleaseTimeMid));
    baselineReleaseTimeHighTextField_->setText(QString::number(config.baselineReleaseTimeHigh));
    thresholdRelaxTimeLowTextField_->setText(QString::number(config.thresholdRelaxTimeLow));
    thresholdRelaxTimeMidTextField_->setText(QString::number(config.thresholdRelaxTimeMid));
    thresholdRelaxTimeHighTextField_->setText(QString::number(config.thresholdRelaxTimeHigh));
    onsetRatioLowTextField_->setText(QString::number(config.onsetRatioLow));
    onsetRatioMidTextField_->setText(QString::number(config.onsetRatioMid));
    onsetRatioHighTextField_->setText(QString::number(config.onsetRatioHigh));
    minRelativeThresholdFactorTextField_->setText(QString::number(config.minRelativeThresholdFactor));
    risingThresholdScaleTextField_->setText(QString::number(config.risingThresholdScale));
    fallingThresholdScaleTextField_->setText(QString::number(config.fallingThresholdScale));

    useTempoLockCheckBox_->setChecked(config.useTempoLock);
    tempoLockMinBpmTextField_->setText(QString::number(config.tempoLockMinBpm));
    tempoLockMaxBpmTextField_->setText(QString::number(config.tempoLockMaxBpm));
    tempoLockToleranceTextField_->setText(QString::number(config.tempoLockTolerance));
    tempoLockStaleResetTimeTextField_->setText(QString::number(config.tempoLockStaleResetTime));

    useOsfFusionCheckBox_->setChecked(config.useOsfFusion);
    osfUseAdaptiveWhiteningCheckBox_->setChecked(config.osfUseAdaptiveWhitening);
    osfSpectralTiltWeightingCheckBox_->setChecked(config.osfSpectralTiltWeighting);
    osfBaselineAttackTimeTextField_->setText(QString::number(config.osfBaselineAttackTime));
    osfBaselineReleaseTimeTextField_->setText(QString::number(config.osfBaselineReleaseTime));
    osfWhiteningTimeTextField_->setText(QString::number(config.osfWhiteningTime));
    osfWhiteningFloorTextField_->setText(QString::number(config.osfWhiteningFloor));
    osfSmoothingTimeTextField_->setText(QString::number(config.osfSmoothingTime));
    osfAbsoluteMinThresholdTextField_->setText(QString::number(config.osfAbsoluteMinThreshold));
    osfThresholdRelTextField_->setText(QString::number(config.osfThresholdRel));
    osfOnsetRatioTextField_->setText(QString::number(config.osfOnsetRatio));
    osfBaselineAttackTimeFinalTextField_->setText(QString::number(config.osfBaselineAttackTimeFinal));
    osfBaselineReleaseTimeFinalTextField_->setText(QString::number(config.osfBaselineReleaseTimeFinal));
    osfThresholdRelaxTimeTextField_->setText(QString::number(config.osfThresholdRelaxTime));
    osfTiltGainTextField_->setText(QString::number(config.osfTiltGain));
    osfOverallLevelGateTextField_->setText(QString::number(config.osfOverallLevelGate));

    thresholdModeComboBox_->setCurrentIndex(static_cast<int>(config.thresholdMode));
    thresholdWindowMsLowTextField_->setText(QString::number(config.thresholdWindowMsLow));
    thresholdWindowMsMidTextField_->setText(QString::number(config.thresholdWindowMsMid));
    thresholdWindowMsHighTextField_->setText(QString::number(config.thresholdWindowMsHigh));
    thresholdDeltaTextField_->setText(QString::number(config.thresholdDelta));
    osfThresholdWindowMsTextField_->setText(QString::number(config.osfThresholdWindowMs));
    osfThresholdDeltaTextField_->setText(QString::number(config.osfThresholdDelta));
}

void ConfiguratorWindow::setupUi() {
    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* scrollContent = new QWidget(scrollArea);
    QVBoxLayout* mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Audio I/O row
    QHBoxLayout* audioRow = new QHBoxLayout();
    audioRow->addWidget(setupAudioInputControls(), 1);
    audioRow->addWidget(setupAudioOutputControls(), 1);
    mainLayout->addLayout(audioRow);

    // Signal conditioning row: downsample controls side by side
    QHBoxLayout* downsampleRow = new QHBoxLayout();
    downsampleRow->addWidget(setupEnvelopeControls(), 1);
    downsampleRow->addWidget(setupDownsampleControls(), 1);
    mainLayout->addLayout(downsampleRow);

    // Global scalars row: AGC, Max BPM, Coincidence Window
    QHBoxLayout* scalarsRow = new QHBoxLayout();
    scalarsRow->addWidget(setupGainControls(), 1);
    scalarsRow->addWidget(setupMaxBpmControls(), 1);
    scalarsRow->addWidget(setupFusionControls(), 1);
    mainLayout->addLayout(scalarsRow);

    // Band weights alongside global scalars
    mainLayout->addWidget(setupBandWeightControls());

    // Tempo lock controls (full width, sits next to band weights)
    mainLayout->addWidget(setupTempoLockControls());

    // OSF (Onset Strength Function) controls (full width)
    mainLayout->addWidget(setupOsfControls());

    // Per-band sections (full width)
    mainLayout->addWidget(setupEnvelopeTimingControls());
    mainLayout->addWidget(setupPeakDetectionControls());
    mainLayout->addWidget(setupPeakDetectorTimingControls());
    mainLayout->addWidget(setupPeakDetectorTuningControls());
    mainLayout->addWidget(setupMovingMeanControls());

    QHBoxLayout* buttonRow = new QHBoxLayout();
    buttonRow->addStretch();
    QPushButton* loadButton = new QPushButton("Load...", this);
    loadButton->setFixedWidth(80);
    connect(loadButton, &QPushButton::clicked, this, &ConfiguratorWindow::onLoadConfigClicked);
    buttonRow->addWidget(loadButton);
    QPushButton* saveButton = new QPushButton("Save...", this);
    saveButton->setFixedWidth(80);
    connect(saveButton, &QPushButton::clicked, this, &ConfiguratorWindow::onSaveConfigClicked);
    buttonRow->addWidget(saveButton);
    QPushButton* applyButton = new QPushButton("Apply", this);
    applyButton->setFixedWidth(80);
    connect(applyButton, &QPushButton::clicked, this, &ConfiguratorWindow::onApplyButtonClicked);
    buttonRow->addWidget(applyButton);
    mainLayout->addLayout(buttonRow);

    mainLayout->addStretch();

    scrollArea->setWidget(scrollContent);
    outerLayout->addWidget(scrollArea);
}

AudioInputMode ConfiguratorWindow::currentAudioInputMode() const {
    return microphoneRadioButton_->isChecked() ? AudioInputMode::Microphone : AudioInputMode::WavFile;
}

QString ConfiguratorWindow::currentAudioSource() const {
    if (microphoneRadioButton_->isChecked()) {
        QVariant data = microphoneComboBox_->currentData();
        if (data.isValid()) return data.toString();
        return microphoneComboBox_->currentText();
    }
    return wavFilePath_;
}

void ConfiguratorWindow::setWavFilePath(const QString& fullPath) {
    wavFilePath_ = fullPath;
    QFileInfo fileInfo(fullPath);
    wavFilePathEdit_->setText(fileInfo.fileName());
}

QWidget* ConfiguratorWindow::setupAudioInputControls() {
    QGroupBox* group = new QGroupBox("Audio Input", this);
    group->setStyleSheet(STYLE_GENERAL);
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    QHBoxLayout* modeLayout = new QHBoxLayout();
    microphoneRadioButton_ = new QRadioButton("Microphone", this);
    wavFileRadioButton_ = new QRadioButton("WAV File", this);
    wavFileRadioButton_->setChecked(true);
    modeLayout->addWidget(microphoneRadioButton_);
    modeLayout->addWidget(wavFileRadioButton_);
    modeLayout->addStretch();
    layout->addLayout(modeLayout);

    audioInputStack_ = new QStackedWidget(this);

    // Page 0: Microphone selector
    QWidget* micPage = new QWidget(this);
    QHBoxLayout* micLayout = new QHBoxLayout(micPage);
    micLayout->setContentsMargins(0, 0, 0, 0);
    microphoneComboBox_ = new QComboBox(this);
    auto devices = jellED::UsbMicro::getAvailableInputDevices(SoundIoBackendCoreAudio);
    for (const auto& dev : devices) {
        microphoneComboBox_->addItem(
            QString::fromStdString(dev.name),
            QString::fromStdString(dev.id));
    }
    micLayout->addWidget(new QLabel("Device:", this));
    micLayout->addWidget(microphoneComboBox_, 1);
    audioInputStack_->addWidget(micPage);

    // Page 1: WAV file selector
    QWidget* filePage = new QWidget(this);
    QHBoxLayout* fileLayout = new QHBoxLayout(filePage);
    fileLayout->setContentsMargins(0, 0, 0, 0);
    wavFilePathEdit_ = new QLineEdit(this);
    wavFilePathEdit_->setReadOnly(true);
    wavFilePathEdit_->setPlaceholderText("Select a .wav file...");
    QPushButton* browseButton = new QPushButton("Browse...", this);
    fileLayout->addWidget(new QLabel("File:", this));
    fileLayout->addWidget(wavFilePathEdit_, 1);
    fileLayout->addWidget(browseButton);
    audioInputStack_->addWidget(filePage);

    audioInputStack_->setCurrentIndex(1);
    layout->addWidget(audioInputStack_);

    connect(microphoneRadioButton_, &QRadioButton::toggled,
            this, &ConfiguratorWindow::onAudioInputModeChanged);
    connect(browseButton, &QPushButton::clicked,
            this, &ConfiguratorWindow::onBrowseWavFileClicked);

    return group;
}

QString ConfiguratorWindow::currentOutputDevice() const {
    QVariant data = outputDeviceComboBox_->currentData();
    if (data.isValid()) return data.toString();
    return outputDeviceComboBox_->currentText();
}

QWidget* ConfiguratorWindow::setupAudioOutputControls() {
    QGroupBox* group = new QGroupBox("Audio Output", this);
    group->setStyleSheet(STYLE_GENERAL);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    outputDeviceComboBox_ = new QComboBox(this);
    auto devices = jellED::UsbMicro::getAvailableOutputDevices(SoundIoBackendCoreAudio);
    for (const auto& dev : devices) {
        outputDeviceComboBox_->addItem(
            QString::fromStdString(dev.name),
            QString::fromStdString(dev.id));
    }
    layout->addWidget(new QLabel("Device:", this));
    layout->addWidget(outputDeviceComboBox_, 1);

    return group;
}

void ConfiguratorWindow::onAudioInputModeChanged() {
    audioInputStack_->setCurrentIndex(microphoneRadioButton_->isChecked() ? 0 : 1);
}

void ConfiguratorWindow::onBrowseWavFileClicked() {
    QString startDir = wavFilePath_.isEmpty() ? QString() : QFileInfo(wavFilePath_).absolutePath();
    QString filePath = QFileDialog::getOpenFileName(
        this, "Select WAV File", startDir,
        "WAV Files (*.wav);;All Files (*)",
        nullptr, QFileDialog::DontUseNativeDialog);
    if (!filePath.isEmpty()) {
        setWavFilePath(filePath);
    }
}

QWidget* ConfiguratorWindow::setupEnvelopeControls() {
    QGroupBox* group = new QGroupBox("Envelope Downsample Rate", this);
    group->setStyleSheet(STYLE_ENVELOPE);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    envelopeDownsampleRateSlider_ = new QSlider(Qt::Horizontal, this);
    envelopeDownsampleRateSlider_->setRange(0, 4);
    int initialPosition = 0;
    int temp = DEFAULTS.envelopeDownsampleRatio;
    while (temp > 1) {
        temp >>= 1;
        initialPosition++;
    }
    envelopeDownsampleRateSlider_->setValue(initialPosition);
    envelopeDownsampleRateSlider_->setSingleStep(1);
    envelopeDownsampleRateSlider_->setTickPosition(QSlider::TicksBelow);
    envelopeDownsampleRateSlider_->setTickInterval(1);

    envelopeDownsampleRateValueLabel_ = new QLabel(QString::number(DEFAULTS.envelopeDownsampleRatio), this);
    envelopeDownsampleRateValueLabel_->setFixedWidth(30);

    layout->addWidget(envelopeDownsampleRateSlider_);
    layout->addWidget(envelopeDownsampleRateValueLabel_);

    connect(envelopeDownsampleRateSlider_, &QSlider::valueChanged,
            this, &ConfiguratorWindow::onEnvelopeDownsampleRateSliderChanged);

    return group;
}

QWidget* ConfiguratorWindow::setupDownsampleControls() {
    QGroupBox* group = new QGroupBox("Downsample Cutoff Frequency", this);
    group->setStyleSheet(STYLE_GENERAL);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    downsampleCutoffFrequencySlider_ = new QSlider(Qt::Horizontal, this);
    downsampleCutoffFrequencySlider_->setRange(0, 10);
    downsampleCutoffFrequencySlider_->setValue(static_cast<int>(DEFAULTS.downsampleCutoffFrequency * 10));
    downsampleCutoffFrequencySlider_->setSingleStep(1);
    downsampleCutoffFrequencySlider_->setTickPosition(QSlider::TicksBelow);
    downsampleCutoffFrequencySlider_->setTickInterval(1);

    downsampleCutoffFrequencyValueLabel_ = new QLabel(QString::number(DEFAULTS.downsampleCutoffFrequency), this);
    downsampleCutoffFrequencyValueLabel_->setFixedWidth(30);

    layout->addWidget(downsampleCutoffFrequencySlider_);
    layout->addWidget(downsampleCutoffFrequencyValueLabel_);

    connect(downsampleCutoffFrequencySlider_, &QSlider::valueChanged,
            this, &ConfiguratorWindow::onDownsampleCutoffFrequencySliderChanged);

    return group;
}

QWidget* ConfiguratorWindow::setupGainControls() {
    QGroupBox* group = new QGroupBox("Signal Conditioning", this);
    group->setStyleSheet(STYLE_GENERAL);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(10);
    layout->setContentsMargins(5, 5, 5, 5);

    QVBoxLayout* agcCol = new QVBoxLayout();
    agcCol->addWidget(new QLabel("AGC Target Level", this));
    automaticGainControlTargetLevelTextField_ = new QLineEdit(QString::number(DEFAULTS.automaticGainControlTargetLevel), this);
    agcCol->addWidget(automaticGainControlTargetLevelTextField_);
    layout->addLayout(agcCol);

    QVBoxLayout* gateCol = new QVBoxLayout();
    gateCol->addWidget(new QLabel("Noise Gate Threshold", this));
    noiseGateThresholdTextField_ = new QLineEdit(QString::number(DEFAULTS.noiseGateThreshold), this);
    gateCol->addWidget(noiseGateThresholdTextField_);
    layout->addLayout(gateCol);

    return group;
}

QWidget* ConfiguratorWindow::setupMaxBpmControls() {
    QGroupBox* group = new QGroupBox("Max BPM", this);
    group->setStyleSheet(STYLE_PEAK);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);
    peakDetectionMaxBpmTextField_ = new QLineEdit(QString::number(DEFAULTS.peakDetectionMaxBpm), this);
    layout->addWidget(peakDetectionMaxBpmTextField_);
    return group;
}

QWidget* ConfiguratorWindow::setupPeakDetectionControls() {
    QGroupBox* group = new QGroupBox("Peak Detection Thresholds", this);
    group->setStyleSheet(STYLE_PEAK);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(0);
    layout->setContentsMargins(5, 5, 5, 5);

    layout->addWidget(setupPerBandRow("Absolute Min Threshold",
        DEFAULTS.absoluteMinThresholdLow, DEFAULTS.absoluteMinThresholdMid, DEFAULTS.absoluteMinThresholdHigh,
        absoluteMinThresholdLowTextField_, absoluteMinThresholdMidTextField_, absoluteMinThresholdHighTextField_));

    layout->addWidget(setupPerBandRow("Threshold Rel",
        DEFAULTS.thresholdRelLow, DEFAULTS.thresholdRelMid, DEFAULTS.thresholdRelHigh,
        thresholdRelLowTextField_, thresholdRelMidTextField_, thresholdRelHighTextField_));

    return group;
}

QWidget* ConfiguratorWindow::setupPeakDetectorTimingControls() {
    QGroupBox* group = new QGroupBox("Peak Detector Timing (per-band)", this);
    group->setStyleSheet(STYLE_PEAK);
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(0);
    layout->setContentsMargins(5, 5, 5, 5);

    // Two rows of two per-band controls side by side
    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->addWidget(setupPerBandRow("Baseline Attack (s)",
        DEFAULTS.baselineAttackTimeLow, DEFAULTS.baselineAttackTimeMid, DEFAULTS.baselineAttackTimeHigh,
        baselineAttackTimeLowTextField_, baselineAttackTimeMidTextField_, baselineAttackTimeHighTextField_));
    topRow->addWidget(setupPerBandRow("Baseline Release (s)",
        DEFAULTS.baselineReleaseTimeLow, DEFAULTS.baselineReleaseTimeMid, DEFAULTS.baselineReleaseTimeHigh,
        baselineReleaseTimeLowTextField_, baselineReleaseTimeMidTextField_, baselineReleaseTimeHighTextField_));
    layout->addLayout(topRow);

    QHBoxLayout* bottomRow = new QHBoxLayout();
    bottomRow->addWidget(setupPerBandRow("Threshold Relax (s)",
        DEFAULTS.thresholdRelaxTimeLow, DEFAULTS.thresholdRelaxTimeMid, DEFAULTS.thresholdRelaxTimeHigh,
        thresholdRelaxTimeLowTextField_, thresholdRelaxTimeMidTextField_, thresholdRelaxTimeHighTextField_));
    bottomRow->addWidget(setupPerBandRow("Onset Ratio",
        DEFAULTS.onsetRatioLow, DEFAULTS.onsetRatioMid, DEFAULTS.onsetRatioHigh,
        onsetRatioLowTextField_, onsetRatioMidTextField_, onsetRatioHighTextField_));
    layout->addLayout(bottomRow);

    return group;
}

QWidget* ConfiguratorWindow::setupPeakDetectorTuningControls() {
    QGroupBox* group = new QGroupBox("Peak Detector Tuning (global)", this);
    group->setStyleSheet(STYLE_PEAK);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(10);
    layout->setContentsMargins(5, 5, 5, 5);

    auto addField = [&](const QString& label, const QString& tooltip, double defaultValue) -> QLineEdit* {
        QVBoxLayout* col = new QVBoxLayout();
        col->setSpacing(2);
        QLabel* lbl = new QLabel(label, this);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setToolTip(tooltip);
        QLineEdit* field = new QLineEdit(QString::number(defaultValue), this);
        field->setToolTip(tooltip);
        col->addWidget(lbl);
        col->addWidget(field);
        layout->addLayout(col);
        return field;
    };

    minRelativeThresholdFactorTextField_ = addField(
        "Min Threshold Factor",
        "Floor for dynamic threshold (fraction of thresholdRel)",
        DEFAULTS.minRelativeThresholdFactor);
    risingThresholdScaleTextField_ = addField(
        "Rising Scale",
        "Hysteresis upper bound multiplier",
        DEFAULTS.risingThresholdScale);
    fallingThresholdScaleTextField_ = addField(
        "Falling Scale",
        "Hysteresis lower bound multiplier",
        DEFAULTS.fallingThresholdScale);

    return group;
}

QWidget* ConfiguratorWindow::setupBandWeightControls() {
    QGroupBox* group = new QGroupBox("Band Weights", this);
    group->setStyleSheet(STYLE_GENERAL);
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(0);
    layout->setContentsMargins(5, 5, 5, 5);

    layout->addWidget(setupPerBandRow("Weight",
        DEFAULTS.bandWeightLow, DEFAULTS.bandWeightMid, DEFAULTS.bandWeightHigh,
        bandWeightLowTextField_, bandWeightMidTextField_, bandWeightHighTextField_));

    return group;
}

QWidget* ConfiguratorWindow::setupPerBandRow(const QString& title,
    double defaultLow, double defaultMid, double defaultHigh,
    QLineEdit*& lowField, QLineEdit*& midField, QLineEdit*& highField) {

    QGroupBox* row = new QGroupBox(title, this);
    QHBoxLayout* rowLayout = new QHBoxLayout(row);
    rowLayout->setSpacing(10);
    rowLayout->setContentsMargins(5, 5, 5, 5);

    auto addBandField = [&](const QString& label, double value) -> QLineEdit* {
        QVBoxLayout* bandLayout = new QVBoxLayout();
        bandLayout->setSpacing(2);
        QLabel* lbl = new QLabel(label, this);
        lbl->setAlignment(Qt::AlignCenter);
        QLineEdit* field = new QLineEdit(QString::number(value), this);
        bandLayout->addWidget(lbl);
        bandLayout->addWidget(field);
        rowLayout->addLayout(bandLayout);
        return field;
    };

    lowField = addBandField("Low (50-150 Hz)", defaultLow);
    midField = addBandField("Mid (150-500 Hz)", defaultMid);
    highField = addBandField("High (2000-5000 Hz)", defaultHigh);

    return row;
}

QWidget* ConfiguratorWindow::setupEnvelopeTimingControls() {
    QGroupBox* group = new QGroupBox("Envelope Timing (per-band)", this);
    group->setStyleSheet(STYLE_ENVELOPE);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(0);
    layout->setContentsMargins(5, 5, 5, 5);

    layout->addWidget(setupPerBandRow("Attack (s)",
        DEFAULTS.envelopeAttackTimeLow, DEFAULTS.envelopeAttackTimeMid, DEFAULTS.envelopeAttackTimeHigh,
        envelopeAttackTimeLowTextField_, envelopeAttackTimeMidTextField_, envelopeAttackTimeHighTextField_));

    layout->addWidget(setupPerBandRow("Release (s)",
        DEFAULTS.envelopeReleaseTimeLow, DEFAULTS.envelopeReleaseTimeMid, DEFAULTS.envelopeReleaseTimeHigh,
        envelopeReleaseTimeLowTextField_, envelopeReleaseTimeMidTextField_, envelopeReleaseTimeHighTextField_));

    return group;
}

QWidget* ConfiguratorWindow::setupFusionControls() {
    QGroupBox* group = new QGroupBox("Multiband Fusion", this);
    group->setStyleSheet(STYLE_GENERAL);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    QGroupBox* subGroup = new QGroupBox("Coincidence Window (seconds)", this);
    QHBoxLayout* subLayout = new QHBoxLayout(subGroup);
    subLayout->setSpacing(5);
    subLayout->setContentsMargins(5, 5, 5, 5);
    coincidenceWindowTextField_ = new QLineEdit(QString::number(DEFAULTS.coincidenceWindow), this);
    subLayout->addWidget(coincidenceWindowTextField_);
    layout->addWidget(subGroup);

    return group;
}

QWidget* ConfiguratorWindow::setupOsfControls() {
    QGroupBox* group = new QGroupBox("Onset Strength Function (OSF)", this);
    group->setStyleSheet(STYLE_PEAK);
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(8);
    layout->setContentsMargins(5, 5, 5, 5);

    // Top row: master toggles (checkboxes)
    QHBoxLayout* togglesRow = new QHBoxLayout();
    useOsfFusionCheckBox_ = new QCheckBox("Use OSF Fusion", this);
    useOsfFusionCheckBox_->setChecked(DEFAULTS.useOsfFusion);
    togglesRow->addWidget(useOsfFusionCheckBox_);

    osfUseAdaptiveWhiteningCheckBox_ = new QCheckBox("Adaptive Whitening", this);
    osfUseAdaptiveWhiteningCheckBox_->setChecked(DEFAULTS.osfUseAdaptiveWhitening);
    togglesRow->addWidget(osfUseAdaptiveWhiteningCheckBox_);

    osfSpectralTiltWeightingCheckBox_ = new QCheckBox("Spectral-Tilt Weighting", this);
    osfSpectralTiltWeightingCheckBox_->setChecked(DEFAULTS.osfSpectralTiltWeighting);
    togglesRow->addWidget(osfSpectralTiltWeightingCheckBox_);
    togglesRow->addStretch();
    layout->addLayout(togglesRow);

    auto addField = [this](QHBoxLayout* row, const QString& title, double defaultVal,
                           QLineEdit*& outField) {
        QVBoxLayout* col = new QVBoxLayout();
        col->addWidget(new QLabel(title, this));
        outField = new QLineEdit(QString::number(defaultVal), this);
        col->addWidget(outField);
        row->addLayout(col);
    };

    // Novelty row
    QHBoxLayout* noveltyRow = new QHBoxLayout();
    noveltyRow->addWidget(new QLabel("Novelty:", this));
    addField(noveltyRow, "Baseline Attack (s)",  DEFAULTS.osfBaselineAttackTime,  osfBaselineAttackTimeTextField_);
    addField(noveltyRow, "Baseline Release (s)", DEFAULTS.osfBaselineReleaseTime, osfBaselineReleaseTimeTextField_);
    addField(noveltyRow, "Whitening Time (s)",   DEFAULTS.osfWhiteningTime,       osfWhiteningTimeTextField_);
    addField(noveltyRow, "Whitening Floor",      DEFAULTS.osfWhiteningFloor,      osfWhiteningFloorTextField_);
    addField(noveltyRow, "Smoothing Time (s)",   DEFAULTS.osfSmoothingTime,       osfSmoothingTimeTextField_);
    layout->addLayout(noveltyRow);

    // Peak detector row
    QHBoxLayout* peakRow = new QHBoxLayout();
    peakRow->addWidget(new QLabel("Peak:", this));
    addField(peakRow, "Abs Min",            DEFAULTS.osfAbsoluteMinThreshold,    osfAbsoluteMinThresholdTextField_);
    addField(peakRow, "Threshold Rel",      DEFAULTS.osfThresholdRel,            osfThresholdRelTextField_);
    addField(peakRow, "Onset Ratio",        DEFAULTS.osfOnsetRatio,              osfOnsetRatioTextField_);
    addField(peakRow, "Baseline Attack (s)",  DEFAULTS.osfBaselineAttackTimeFinal,  osfBaselineAttackTimeFinalTextField_);
    addField(peakRow, "Baseline Release (s)", DEFAULTS.osfBaselineReleaseTimeFinal, osfBaselineReleaseTimeFinalTextField_);
    addField(peakRow, "Threshold Relax (s)",  DEFAULTS.osfThresholdRelaxTime,       osfThresholdRelaxTimeTextField_);
    addField(peakRow, "MM Window (ms)",  DEFAULTS.osfThresholdWindowMs,  osfThresholdWindowMsTextField_);
    addField(peakRow, "MM Delta (δ)",    DEFAULTS.osfThresholdDelta,     osfThresholdDeltaTextField_);
    layout->addLayout(peakRow);

    // Misc row
    QHBoxLayout* miscRow = new QHBoxLayout();
    miscRow->addWidget(new QLabel("Misc:", this));
    addField(miscRow, "Tilt Gain",       DEFAULTS.osfTiltGain,         osfTiltGainTextField_);
    addField(miscRow, "Level Gate",      DEFAULTS.osfOverallLevelGate, osfOverallLevelGateTextField_);
    miscRow->addStretch();
    layout->addLayout(miscRow);

    return group;
}

QWidget* ConfiguratorWindow::setupTempoLockControls() {
    QGroupBox* group = new QGroupBox("Tempo Lock", this);
    group->setStyleSheet(STYLE_PEAK);
    QHBoxLayout* layout = new QHBoxLayout(group);
    layout->setSpacing(10);
    layout->setContentsMargins(5, 5, 5, 5);

    useTempoLockCheckBox_ = new QCheckBox("Enabled", this);
    useTempoLockCheckBox_->setChecked(DEFAULTS.useTempoLock);
    layout->addWidget(useTempoLockCheckBox_);

    auto addField = [this, layout](const QString& title, double defaultVal,
                                   QLineEdit*& outField) {
        QVBoxLayout* col = new QVBoxLayout();
        col->addWidget(new QLabel(title, this));
        outField = new QLineEdit(QString::number(defaultVal), this);
        col->addWidget(outField);
        layout->addLayout(col);
    };

    addField("Min BPM", DEFAULTS.tempoLockMinBpm, tempoLockMinBpmTextField_);
    addField("Max BPM", DEFAULTS.tempoLockMaxBpm, tempoLockMaxBpmTextField_);
    addField("Tolerance", DEFAULTS.tempoLockTolerance, tempoLockToleranceTextField_);
    addField("Stale Reset (s)", DEFAULTS.tempoLockStaleResetTime,
             tempoLockStaleResetTimeTextField_);

    return group;
}

QWidget* ConfiguratorWindow::setupMovingMeanControls() {
    QGroupBox* group = new QGroupBox("Moving-Mean Threshold (Böck/Schedl ISMIR 2012)", this);
    group->setStyleSheet(STYLE_PEAK);
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(8);
    layout->setContentsMargins(5, 5, 5, 5);

    // Mode selector + global delta
    QHBoxLayout* modeRow = new QHBoxLayout();
    modeRow->addWidget(new QLabel("Mode:", this));
    thresholdModeComboBox_ = new QComboBox(this);
    thresholdModeComboBox_->addItem("Envelope (current)");
    thresholdModeComboBox_->addItem("Moving Mean (Böck 2012)");
    thresholdModeComboBox_->setCurrentIndex(static_cast<int>(DEFAULTS.thresholdMode));
    thresholdModeComboBox_->setToolTip(
        "Envelope: asymmetric IIR follower (current behavior).\n"
        "Moving Mean: causal mean window — threshold = mean(W) + δ.\n"
        "Eliminates post-peak masking; see plan/moving_mean_threshold.html.");
    modeRow->addWidget(thresholdModeComboBox_);

    modeRow->addSpacing(20);
    modeRow->addWidget(new QLabel("Delta (δ):", this));
    thresholdDeltaTextField_ = new QLineEdit(QString::number(DEFAULTS.thresholdDelta), this);
    thresholdDeltaTextField_->setFixedWidth(70);
    thresholdDeltaTextField_->setToolTip("Additive offset above the local mean (Böck 'delta').");
    modeRow->addWidget(thresholdDeltaTextField_);
    modeRow->addStretch();
    layout->addLayout(modeRow);

    // Per-band window row
    layout->addWidget(setupPerBandRow("Window (ms)",
        DEFAULTS.thresholdWindowMsLow, DEFAULTS.thresholdWindowMsMid, DEFAULTS.thresholdWindowMsHigh,
        thresholdWindowMsLowTextField_, thresholdWindowMsMidTextField_, thresholdWindowMsHighTextField_));

    return group;
}

void ConfiguratorWindow::onEnvelopeDownsampleRateSliderChanged(int value) {
    int envelopeDownsampleRate = 1 << value;
    envelopeDownsampleRateValueLabel_->setText(QString::number(envelopeDownsampleRate));
}

void ConfiguratorWindow::onDownsampleCutoffFrequencySliderChanged(int value) {
    double downsampleCutoffFrequency = 0.1 * value;
    downsampleCutoffFrequencyValueLabel_->setText(QString::number(downsampleCutoffFrequency));
}

void ConfiguratorWindow::onApplyButtonClicked() {
    emit applyRequested(currentConfig());
}

void ConfiguratorWindow::onSaveConfigClicked() {
    QString path = QFileDialog::getSaveFileName(
        this, "Save Configuration", QString(),
        CONFIG_FILE_FILTER, nullptr, QFileDialog::DontUseNativeDialog);
    if (path.isEmpty()) return;

    if (!path.endsWith(".json", Qt::CaseInsensitive)) {
        path += ".json";
    }

    QJsonObject obj = configToJson(currentConfig());
    QJsonDocument doc(obj);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Failed",
            "Could not open file for writing:\n" + path);
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    QMessageBox::information(this, "Configuration Saved",
        "Configuration saved to:\n" + path);
}

void ConfiguratorWindow::onLoadConfigClicked() {
    QString path = QFileDialog::getOpenFileName(
        this, "Load Configuration", QString(),
        CONFIG_FILE_FILTER, nullptr, QFileDialog::DontUseNativeDialog);
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Load Failed",
            "Could not open file:\n" + path);
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::warning(this, "Load Failed",
            "Invalid JSON in file:\n" + parseError.errorString());
        return;
    }

    jellED::BeatDetectionConfig config;
    if (!jsonToConfig(doc.object(), config)) {
        QMessageBox::warning(this, "Load Failed", "Could not parse configuration.");
        return;
    }

    applyConfigToUi(config);
    QMessageBox::information(this, "Configuration Loaded",
        "Configuration loaded from:\n" + path);
}
