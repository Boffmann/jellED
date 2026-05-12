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
    , filteredSampleLow_(0.0f)
    , filteredSampleMid_(0.0f)
    , filteredSampleHigh_(0.0f)
    , envelopeSampleLow_(0.0f)
    , envelopeSampleMid_(0.0f)
    , envelopeSampleHigh_(0.0f)
    , peakDetectedLow_(false)
    , peakDetectedMid_(false)
    , peakDetectedHigh_(false)
    , peakDetectedOsf_(false)
    , currentTime_(0.0f)
    , bandConfigLow_{BANDPASS_FILTER_COEFFICIENTS_LOW, 1.0f,
                     config.absoluteMinThresholdLow,
                     config.thresholdRelLow,
                     config.bandWeightLow,
                     config.envelopeAttackTimeLow, config.envelopeReleaseTimeLow,
                     config.baselineAttackTimeLow, config.baselineReleaseTimeLow,
                     config.thresholdRelaxTimeLow, config.onsetRatioLow,
                     config.minRelativeThresholdFactor,
                     config.risingThresholdScale, config.fallingThresholdScale,
                     config.thresholdMode, config.thresholdWindowMsLow, config.thresholdDelta}
    , bandConfigMid_{BANDPASS_FILTER_COEFFICIENTS_MID, 1.0f,
                     config.absoluteMinThresholdMid,
                     config.thresholdRelMid,
                     config.bandWeightMid,
                     config.envelopeAttackTimeMid, config.envelopeReleaseTimeMid,
                     config.baselineAttackTimeMid, config.baselineReleaseTimeMid,
                     config.thresholdRelaxTimeMid, config.onsetRatioMid,
                     config.minRelativeThresholdFactor,
                     config.risingThresholdScale, config.fallingThresholdScale,
                     config.thresholdMode, config.thresholdWindowMsMid, config.thresholdDelta}
    , bandConfigHigh_{BANDPASS_FILTER_COEFFICIENTS_HIGH, 1.0f,
                      config.absoluteMinThresholdHigh,
                      config.thresholdRelHigh,
                      config.bandWeightHigh,
                      config.envelopeAttackTimeHigh, config.envelopeReleaseTimeHigh,
                      config.baselineAttackTimeHigh, config.baselineReleaseTimeHigh,
                      config.thresholdRelaxTimeHigh, config.onsetRatioHigh,
                      config.minRelativeThresholdFactor,
                      config.risingThresholdScale, config.fallingThresholdScale,
                      config.thresholdMode, config.thresholdWindowMsHigh, config.thresholdDelta}
    , bandStateLow_(bandConfigLow_, sampleRate_, config.envelopeDownsampleRatio, config.peakDetectionMaxBpm)
    , bandStateMid_(bandConfigMid_, sampleRate_, config.envelopeDownsampleRatio, config.peakDetectionMaxBpm)
    , bandStateHigh_(bandConfigHigh_, sampleRate_, config.envelopeDownsampleRatio, config.peakDetectionMaxBpm)
    , multibandFusion_(config.coincidenceWindow, config.peakDetectionMaxBpm)
    , osf_(static_cast<uint32_t>(sampleRate_) / config.envelopeDownsampleRatio, config)
    , tempoTracker_(12, config.tempoLockMinBpm, config.tempoLockMaxBpm, config.tempoLockTolerance)
    , lastAcceptedBeatTime_(-1.0f)
    , lastPeakRejectedByTempo_(false)
    , shortTermEnergy_(0.0f)
    , longTermEnergy_(0.0f)
    , shortTermCoeff_(1.0f - std::exp(-1.0f / (sampleRate * 0.05f)))
    , longTermCoeff_ (1.0f - std::exp(-1.0f / (sampleRate * 2.0f)))
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

    bandStateLow_.peakDetector.setThresholdMode(
        newConfig.thresholdMode, newConfig.thresholdWindowMsLow, newConfig.thresholdDelta);
    bandStateMid_.peakDetector.setThresholdMode(
        newConfig.thresholdMode, newConfig.thresholdWindowMsMid, newConfig.thresholdDelta);
    bandStateHigh_.peakDetector.setThresholdMode(
        newConfig.thresholdMode, newConfig.thresholdWindowMsHigh, newConfig.thresholdDelta);

    bandStateLow_.envelope.setTimings(newConfig.envelopeAttackTimeLow, newConfig.envelopeReleaseTimeLow);
    bandStateMid_.envelope.setTimings(newConfig.envelopeAttackTimeMid, newConfig.envelopeReleaseTimeMid);
    bandStateHigh_.envelope.setTimings(newConfig.envelopeAttackTimeHigh, newConfig.envelopeReleaseTimeHigh);

    multibandFusion_.setCoincidenceWindow(newConfig.coincidenceWindow);
    multibandFusion_.setMaxBpm(newConfig.peakDetectionMaxBpm);

    // OSF hot updates. Coefficients depend on the envelope rate, not the
    // input rate (the OSF only updates when the envelope downsampler emits
    // a new sample).
    const uint32_t envelopeRate =
        static_cast<uint32_t>(sampleRate_) / config_.envelopeDownsampleRatio;
    osf_.applyConfig(newConfig, envelopeRate);

    tempoTracker_.setMinBpm(newConfig.tempoLockMinBpm);
    tempoTracker_.setMaxBpm(newConfig.tempoLockMaxBpm);
    tempoTracker_.setTolerance(newConfig.tempoLockTolerance);

    config_ = newConfig;
    return true;
}

bool BeatDetector::is_beat(const float sample) {
    this->totalSamplesReceived_++;

    const float absSample = std::abs(sample);
    shortTermEnergy_ += shortTermCoeff_ * (absSample - shortTermEnergy_);
    longTermEnergy_  += longTermCoeff_  * (absSample - longTermEnergy_);

    this->filteredSampleLow_ = this->bandStateLow_.applyBandpassFilter(sample);
    this->filteredSampleMid_ = this->bandStateMid_.applyBandpassFilter(sample);
    this->filteredSampleHigh_ = this->bandStateHigh_.applyBandpassFilter(sample);

    this->envelopeSampleLow_ = this->bandStateLow_.applyEnvelopeFilter(this->filteredSampleLow_);
    this->envelopeSampleMid_ = this->bandStateMid_.applyEnvelopeFilter(this->filteredSampleMid_);
    this->envelopeSampleHigh_ = this->bandStateHigh_.applyEnvelopeFilter(this->filteredSampleHigh_);

    this->currentTime_ = static_cast<float>(this->totalSamplesReceived_) / this->sampleRate_;
    bool anyPeakDetected = false;
    this->peakDetectedLow_ = false;
    this->peakDetectedMid_ = false;
    this->peakDetectedHigh_ = false;
    this->peakDetectedOsf_ = false;

    if (this->envelopeSampleLow_ != -1.0f) {
        float strength = bandStateLow_.applyPeakDetector(this->envelopeSampleLow_, this->currentTime_);
        if (strength > 0.0f) {
            this->peakDetectedLow_ = true;
            if (!config_.useOsfFusion) anyPeakDetected = true;
        }
    }

    if (this->envelopeSampleMid_ != -1.0f) {
        float strength = bandStateMid_.applyPeakDetector(this->envelopeSampleMid_, this->currentTime_);
        if (strength > 0.0f) {
            this->peakDetectedMid_ = true;
        }
    }

    if (this->envelopeSampleHigh_ != -1.0f) {
        float strength = bandStateHigh_.applyPeakDetector(this->envelopeSampleHigh_, this->currentTime_);
        if (strength > 0.0f) {
            this->peakDetectedHigh_ = true;
        }
    }

    // Run the OSF unconditionally on every envelope frame so the GUI can
    // visualize OSF behavior even when useOsfFusion is false. Only the
    // *beat decision* is gated on useOsfFusion: when true, the OSF peak
    // replaces the low-band peak as the fused beat trigger.
    if (envelopeSampleLow_  != -1.0f
        && envelopeSampleMid_  != -1.0f
        && envelopeSampleHigh_ != -1.0f) {

        float wL = config_.bandWeightLow;
        float wM = config_.bandWeightMid;
        float wH = config_.bandWeightHigh;
        if (config_.osfSpectralTiltWeighting) {
            const float tilt = getSpectralTilt();   // [-1, 1]: +1 bass, -1 treble
            wL *= (1.0f + config_.osfTiltGain * std::max(0.0f,  tilt));
            wH *= (1.0f + config_.osfTiltGain * std::max(0.0f, -tilt));
        }

        const bool gateOpen = shortTermEnergy_ >= config_.osfOverallLevelGate;
        peakDetectedOsf_ = osf_.process(
            {envelopeSampleLow_,  bandStateLow_.statsReady,  bandStateLow_.rollingMedian,  wL},
            {envelopeSampleMid_,  bandStateMid_.statsReady,  bandStateMid_.rollingMedian,  wM},
            {envelopeSampleHigh_, bandStateHigh_.statsReady, bandStateHigh_.rollingMedian, wH},
            currentTime_,
            gateOpen);
        if (config_.useOsfFusion && peakDetectedOsf_) {
            anyPeakDetected = true;
        }
    }

    // Tempo-locked acceptance. The TempoTracker is always fed by detected
    // peaks so getCurrentBpm() works regardless of useTempoLock; the flag
    // only controls whether the tracker also gates the return value.
    if (lastAcceptedBeatTime_ >= 0.0f
        && (currentTime_ - lastAcceptedBeatTime_) > config_.tempoLockStaleResetTime) {
        tempoTracker_.reset();
        lastAcceptedBeatTime_ = -1.0f;
    }

    bool gatedBeat = anyPeakDetected;
    if (anyPeakDetected) {
        const double t = static_cast<double>(currentTime_);
        const bool consistent = !tempoTracker_.hasEstablishedTempo()
                              || tempoTracker_.isTempoConsistent(t);
        if (config_.useTempoLock && !consistent) {
            gatedBeat = false;
            lastPeakRejectedByTempo_ = true;
        } else {
            lastPeakRejectedByTempo_ = false;
            tempoTracker_.addBeat(t);
            lastAcceptedBeatTime_ = currentTime_;
        }
    }

    if constexpr (DEBUG_BEAT_DETECTION) {
        if (gatedBeat) {
            std::cout << "[BeatDetector] is_beat=TRUE at t=" << std::fixed << std::setprecision(4)
                      << currentTime_ << "s, sample#=" << totalSamplesReceived_ << std::endl;
        }
    }

    return gatedBeat;
}

float BeatDetector::getFilteredSampleLow() {
    return this->filteredSampleLow_;
}

float BeatDetector::getFilteredSampleMid() {
    return this->filteredSampleMid_;
}

float BeatDetector::getFilteredSampleHigh() {
    return this->filteredSampleHigh_;
}

float BeatDetector::getEnvelopeLow() {
    return this->envelopeSampleLow_;
}

float BeatDetector::getEnvelopeMid() {
    return this->envelopeSampleMid_;
}

float BeatDetector::getEnvelopeHigh() {
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

float BeatDetector::getThresholdLow() {
    return this->bandStateLow_.peakDetector.getLastThreshold();
}

float BeatDetector::getThresholdMid() {
    return this->bandStateMid_.peakDetector.getLastThreshold();
}

float BeatDetector::getThresholdHigh() {
    return this->bandStateHigh_.peakDetector.getLastThreshold();
}

float BeatDetector::getCurrentTime() {
    return this->currentTime_;
}

float BeatDetector::getVolumeLow() const {
    return bandStateLow_.getVolume();
}

float BeatDetector::getVolumeMid() const {
    return bandStateMid_.getVolume();
}

float BeatDetector::getVolumeHigh() const {
    return bandStateHigh_.getVolume();
}

float BeatDetector::getOverallLevel() const {
    return shortTermEnergy_;
}

float BeatDetector::getVolumeTrend() const {
    if (longTermEnergy_ < 1e-6f) return 0.0f;
    return (shortTermEnergy_ / longTermEnergy_) - 1.0f;
}

float BeatDetector::getSpectralTilt() const {
    const float low  = bandStateLow_.getVolume();
    const float high = bandStateHigh_.getVolume();
    const float sum  = low + high;
    if (sum < 1e-6f) return 0.0f;
    return (low - high) / sum;
}

double BeatDetector::getCurrentBpm() const {
    return tempoTracker_.currentBpm();
}

bool BeatDetector::hasEstablishedTempo() const {
    return tempoTracker_.hasEstablishedTempo();
}

bool BeatDetector::wasLastPeakRejectedByTempo() const {
    return lastPeakRejectedByTempo_;
}

} // end namespace jellED
