#include "beatdetection.h"

#include "include/bandpassFilter.h"
#include "include/envelopeDetector.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

// Enable to debug beat detection
static constexpr bool DEBUG_BEAT_DETECTION = false;

namespace jellED {

BeatDetector::BeatDetector(int sampleRate, const BeatDetectionConfig& config)
    : sampleRate_(sampleRate)
    , config_(config)
    , totalSamplesReceived_(0)
    , filteredSampleLow_(0.0)
    , filteredSampleMid_(0.0)
    , filteredSampleHigh_(0.0)
    , envelopeSampleLow_(0.0)
    , envelopeSampleMid_(0.0)
    , envelopeSampleHigh_(0.0)
    , peakDetectedLow_(false)
    , peakDetectedMid_(false)
    , peakDetectedHigh_(false)
    , currentTime_(0.0)
    , bandConfigLow_{BANDPASS_FILTER_COEFFICIENTS_LOW, 1.0,
                     config.absoluteMinThresholdLow,
                     config.thresholdRelLow,
                     config.bandWeightLow,
                     config.envelopeAttackTimeLow, config.envelopeReleaseTimeLow,
                     config.baselineAttackTimeLow, config.baselineReleaseTimeLow,
                     config.thresholdRelaxTimeLow, config.onsetRatioLow,
                     config.minRelativeThresholdFactor,
                     config.risingThresholdScale, config.fallingThresholdScale}
    , bandConfigMid_{BANDPASS_FILTER_COEFFICIENTS_MID, 1.0,
                     config.absoluteMinThresholdMid,
                     config.thresholdRelMid,
                     config.bandWeightMid,
                     config.envelopeAttackTimeMid, config.envelopeReleaseTimeMid,
                     config.baselineAttackTimeMid, config.baselineReleaseTimeMid,
                     config.thresholdRelaxTimeMid, config.onsetRatioMid,
                     config.minRelativeThresholdFactor,
                     config.risingThresholdScale, config.fallingThresholdScale}
    , bandConfigHigh_{BANDPASS_FILTER_COEFFICIENTS_HIGH, 1.0,
                      config.absoluteMinThresholdHigh,
                      config.thresholdRelHigh,
                      config.bandWeightHigh,
                      config.envelopeAttackTimeHigh, config.envelopeReleaseTimeHigh,
                      config.baselineAttackTimeHigh, config.baselineReleaseTimeHigh,
                      config.thresholdRelaxTimeHigh, config.onsetRatioHigh,
                      config.minRelativeThresholdFactor,
                      config.risingThresholdScale, config.fallingThresholdScale}
    , bandStateLow_(bandConfigLow_, sampleRate_, config.envelopeDownsampleRatio, config.peakDetectionMaxBpm)
    , bandStateMid_(bandConfigMid_, sampleRate_, config.envelopeDownsampleRatio, config.peakDetectionMaxBpm)
    , bandStateHigh_(bandConfigHigh_, sampleRate_, config.envelopeDownsampleRatio, config.peakDetectionMaxBpm)
    , multibandFusion_(config.coincidenceWindow, config.peakDetectionMaxBpm)
    , shortTermEnergy_(0.0)
    , longTermEnergy_(0.0)
    , shortTermCoeff_(1.0 - std::exp(-1.0 / (sampleRate * 0.05)))
    , longTermCoeff_ (1.0 - std::exp(-1.0 / (sampleRate * 2.0)))
{
    std::cout << "BeatDetector initialized:" << std::endl;
    std::cout << "  Sample Rate: " << sampleRate_ << " Hz" << std::endl;
    std::cout << "  Max BPM: " << config.peakDetectionMaxBpm << std::endl;
    std::cout << "  Min Beat Interval: " << (60.0 / config.peakDetectionMaxBpm) << " s" << std::endl;
    std::cout << "  Coincidence Window: " << config.coincidenceWindow << " s" << std::endl;
    std::cout << "  Band Weights: L=" << config.bandWeightLow << " M=" << config.bandWeightMid << " H=" << config.bandWeightHigh << std::endl;
    std::cout << "  Envelope Attack: L=" << config.envelopeAttackTimeLow << "s M=" << config.envelopeAttackTimeMid << "s H=" << config.envelopeAttackTimeHigh << "s" << std::endl;
    std::cout << "  Envelope Release: L=" << config.envelopeReleaseTimeLow << "s M=" << config.envelopeReleaseTimeMid << "s H=" << config.envelopeReleaseTimeHigh << "s" << std::endl;
}

BeatDetector::~BeatDetector() {
}

bool BeatDetector::applyConfig(const BeatDetectionConfig& newConfig) {
    if (newConfig.envelopeDownsampleRatio != config_.envelopeDownsampleRatio) {
        return false;
    }

    bandStateLow_.weight = newConfig.bandWeightLow;
    bandStateMid_.weight = newConfig.bandWeightMid;
    bandStateHigh_.weight = newConfig.bandWeightHigh;

    bandStateLow_.peakDetector.setAbsoluteMinThreshold(newConfig.absoluteMinThresholdLow);
    bandStateLow_.peakDetector.setThresholdRel(newConfig.thresholdRelLow);
    bandStateLow_.peakDetector.setMaxBpm(newConfig.peakDetectionMaxBpm);
    bandStateLow_.peakDetector.setOnsetRatio(newConfig.onsetRatioLow);
    bandStateLow_.peakDetector.setTimingParams(
        newConfig.baselineAttackTimeLow, newConfig.baselineReleaseTimeLow,
        newConfig.thresholdRelaxTimeLow);

    bandStateMid_.peakDetector.setAbsoluteMinThreshold(newConfig.absoluteMinThresholdMid);
    bandStateMid_.peakDetector.setThresholdRel(newConfig.thresholdRelMid);
    bandStateMid_.peakDetector.setMaxBpm(newConfig.peakDetectionMaxBpm);
    bandStateMid_.peakDetector.setOnsetRatio(newConfig.onsetRatioMid);
    bandStateMid_.peakDetector.setTimingParams(
        newConfig.baselineAttackTimeMid, newConfig.baselineReleaseTimeMid,
        newConfig.thresholdRelaxTimeMid);

    bandStateHigh_.peakDetector.setAbsoluteMinThreshold(newConfig.absoluteMinThresholdHigh);
    bandStateHigh_.peakDetector.setThresholdRel(newConfig.thresholdRelHigh);
    bandStateHigh_.peakDetector.setMaxBpm(newConfig.peakDetectionMaxBpm);
    bandStateHigh_.peakDetector.setOnsetRatio(newConfig.onsetRatioHigh);
    bandStateHigh_.peakDetector.setTimingParams(
        newConfig.baselineAttackTimeHigh, newConfig.baselineReleaseTimeHigh,
        newConfig.thresholdRelaxTimeHigh);

    // Global peak detector params (same for all bands)
    bandStateLow_.peakDetector.setMinRelativeThresholdFactor(newConfig.minRelativeThresholdFactor);
    bandStateLow_.peakDetector.setHysteresisScales(newConfig.risingThresholdScale, newConfig.fallingThresholdScale);
    bandStateMid_.peakDetector.setMinRelativeThresholdFactor(newConfig.minRelativeThresholdFactor);
    bandStateMid_.peakDetector.setHysteresisScales(newConfig.risingThresholdScale, newConfig.fallingThresholdScale);
    bandStateHigh_.peakDetector.setMinRelativeThresholdFactor(newConfig.minRelativeThresholdFactor);
    bandStateHigh_.peakDetector.setHysteresisScales(newConfig.risingThresholdScale, newConfig.fallingThresholdScale);

    bandStateLow_.envelope.setTimings(newConfig.envelopeAttackTimeLow, newConfig.envelopeReleaseTimeLow);
    bandStateMid_.envelope.setTimings(newConfig.envelopeAttackTimeMid, newConfig.envelopeReleaseTimeMid);
    bandStateHigh_.envelope.setTimings(newConfig.envelopeAttackTimeHigh, newConfig.envelopeReleaseTimeHigh);

    multibandFusion_.setCoincidenceWindow(newConfig.coincidenceWindow);
    multibandFusion_.setMaxBpm(newConfig.peakDetectionMaxBpm);

    config_ = newConfig;
    return true;
}

bool BeatDetector::is_beat(const double sample) {
    this->totalSamplesReceived_++;

    const double absSample = std::abs(sample);
    shortTermEnergy_ += shortTermCoeff_ * (absSample - shortTermEnergy_);
    longTermEnergy_  += longTermCoeff_  * (absSample - longTermEnergy_);

    this->filteredSampleLow_ = this->bandStateLow_.applyBandpassFilter(sample);
    this->filteredSampleMid_ = this->bandStateMid_.applyBandpassFilter(sample);
    this->filteredSampleHigh_ = this->bandStateHigh_.applyBandpassFilter(sample);

    this->envelopeSampleLow_ = this->bandStateLow_.applyEnvelopeFilter(this->filteredSampleLow_);
    this->envelopeSampleMid_ = this->bandStateMid_.applyEnvelopeFilter(this->filteredSampleMid_);
    this->envelopeSampleHigh_ = this->bandStateHigh_.applyEnvelopeFilter(this->filteredSampleHigh_);

    this->currentTime_ = static_cast<double>(this->totalSamplesReceived_) / this->sampleRate_;
    bool anyPeakDetected = false;
    this->peakDetectedLow_ = false;
    this->peakDetectedMid_ = false;
    this->peakDetectedHigh_ = false;

    if (this->envelopeSampleLow_ != -1.0) {
        double strength = bandStateLow_.applyPeakDetector(this->envelopeSampleLow_, this->currentTime_);
        if (strength > 0.0) {
            this->peakDetectedLow_ = true;
            anyPeakDetected = true;
        }
    }

    if (this->envelopeSampleMid_ != -1.0) {
        double strength = bandStateMid_.applyPeakDetector(this->envelopeSampleMid_, this->currentTime_);
        if (strength > 0.0) {
            this->peakDetectedMid_ = true;
        }
    }

    if (this->envelopeSampleHigh_ != -1.0) {
        double strength = bandStateHigh_.applyPeakDetector(this->envelopeSampleHigh_, this->currentTime_);
        if (strength > 0.0) {
            this->peakDetectedHigh_ = true;
        }
    }

    if constexpr (DEBUG_BEAT_DETECTION) {
        if (anyPeakDetected) {
            std::cout << "[BeatDetector] is_beat=TRUE at t=" << std::fixed << std::setprecision(4) 
                      << currentTime_ << "s, sample#=" << totalSamplesReceived_ << std::endl;
        }
    }
    
    return anyPeakDetected;
}

double BeatDetector::getFilteredSampleLow() {
    return this->filteredSampleLow_;
}

double BeatDetector::getFilteredSampleMid() {
    return this->filteredSampleMid_;
}

double BeatDetector::getFilteredSampleHigh() {
    return this->filteredSampleHigh_;
}

double BeatDetector::getEnvelopeLow() {
    return this->envelopeSampleLow_;
}

double BeatDetector::getEnvelopeMid() {
    return this->envelopeSampleMid_;
}

double BeatDetector::getEnvelopeHigh() {
    return this->envelopeSampleHigh_;
}

bool BeatDetector::isPeakLow() {
    return this->peakDetectedLow_;
}

bool BeatDetector::isPeakMid() {
    return this->peakDetectedMid_;
}

bool BeatDetector::isPeakHigh() {
    return this->peakDetectedHigh_;
}

double BeatDetector::getThresholdLow() {
    return this->bandStateLow_.peakDetector.getLastThreshold();
}

double BeatDetector::getThresholdMid() {
    return this->bandStateMid_.peakDetector.getLastThreshold();
}

double BeatDetector::getThresholdHigh() {
    return this->bandStateHigh_.peakDetector.getLastThreshold();
}

double BeatDetector::getCurrentTime() {
    return this->currentTime_;
}

double BeatDetector::getVolumeLow() const {
    return bandStateLow_.getVolume();
}

double BeatDetector::getVolumeMid() const {
    return bandStateMid_.getVolume();
}

double BeatDetector::getVolumeHigh() const {
    return bandStateHigh_.getVolume();
}

double BeatDetector::getOverallLevel() const {
    return shortTermEnergy_;
}

double BeatDetector::getVolumeTrend() const {
    if (longTermEnergy_ < 1e-6) return 0.0;
    return (shortTermEnergy_ / longTermEnergy_) - 1.0;
}

double BeatDetector::getSpectralTilt() const {
    const double low  = bandStateLow_.getVolume();
    const double high = bandStateHigh_.getVolume();
    const double sum  = low + high;
    if (sum < 1e-6) return 0.0;
    return (low - high) / sum;
}

} // end namespace jellED
