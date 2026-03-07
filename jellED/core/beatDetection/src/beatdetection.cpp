#include "beatdetection.h"

#include "include/bandpassFilter.h"
#include "include/envelopeDetector.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

// Enable to debug beat detection
static constexpr bool DEBUG_BEAT_DETECTION = false;

namespace jellED {

BeatDetector::BeatDetector(const Builder& builder)
    : sampleRate_(builder.sampleRate_)
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
    // , platformUtils(builder.platformUtils) // TODO 
    // BandConfig: {coefficients, envelopeGain, absoluteMin, relativeThreshold, weight}
    // Now detecting peaks in envelope directly (not novelty), so gains are ~1.0
    , bandConfigLow_{BANDPASS_FILTER_COEFFICIENTS_LOW, 1.0, 0.05, 0.15, 1.0}   // Low freq (kick)
    , bandConfigMid_{BANDPASS_FILTER_COEFFICIENTS_MID, 1.0, 0.03, 0.15, 0.5}   // Mid freq
    , bandConfigHigh_{BANDPASS_FILTER_COEFFICIENTS_HIGH, 1.0, 0.02, 0.15, 0.3} // High freq (hi-hats)
    , bandStateLow_(bandConfigLow_, sampleRate_, builder.envelopeDownsampleRatio_, builder.peakDetectionMaxBpm_)
    , bandStateMid_(bandConfigMid_, sampleRate_, builder.envelopeDownsampleRatio_, builder.peakDetectionMaxBpm_)
    , bandStateHigh_(bandConfigHigh_, sampleRate_, builder.envelopeDownsampleRatio_, builder.peakDetectionMaxBpm_)
    , multibandFusion_(0.15, builder.peakDetectionMaxBpm_)
{
    std::cout << "BeatDetector initialized:" << std::endl;
    std::cout << "  Sample Rate: " << sampleRate_ << " Hz" << std::endl;
    std::cout << "  Max BPM: " << builder.peakDetectionMaxBpm_ << std::endl;
    std::cout << "  Min Beat Interval: " << (60.0 / builder.peakDetectionMaxBpm_) << " s" << std::endl;
    std::cout << "  Coincidence Window: 0.15 s" << std::endl;
}

BeatDetector::~BeatDetector() {
}

bool BeatDetector::is_beat(const double sample) {
    this->totalSamplesReceived_++;

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
            // if (multibandFusion_.push({this->currentTime_, strength, BAND_ID::LOW})) {
            //     anyPeakDetected = true;
            // }
        }
    }

    if (this->envelopeSampleMid_ != -1.0) {
        double strength = bandStateMid_.applyPeakDetector(this->envelopeSampleMid_, this->currentTime_);
        if (strength > 0.0) {
            this->peakDetectedMid_ = true;
            // if (multibandFusion_.push({this->currentTime_, strength, BAND_ID::MID})) {
            //     anyPeakDetected = true;
            // }
        }
    }

    if (this->envelopeSampleHigh_ != -1.0) {
        double strength = bandStateHigh_.applyPeakDetector(this->envelopeSampleHigh_, this->currentTime_);
        if (strength > 0.0) {
            this->peakDetectedHigh_ = true;
            // if (multibandFusion_.push({this->currentTime_, strength, BAND_ID::HIGH})) {
            //     anyPeakDetected = true;
            // }
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

double BeatDetector::getCurrentTime() {
    return this->currentTime_;
}

} // end namespace jellED
