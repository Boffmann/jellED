#include "include/multibandFusion.h"
#include <iostream>
#include <iomanip>

namespace jellED {

// Enable to debug double-beat issues
// WARNING: Keep this false in production - stdout can block the audio thread on Raspberry Pi!
static constexpr bool DEBUG_MULTIBAND_FUSION = false;

constexpr float SECONDS_PER_MINUTE = 60.0f;

MultiBandFusion::MultiBandFusion(float coincidenceWindow, float maxBpm)
    : coincidenceWindow_(coincidenceWindow),
      lastBeatTime_(-1.0f),
      minBeatInterval_(SECONDS_PER_MINUTE / maxBpm),
      head_(0),
      count_(0) {}

void MultiBandFusion::removeExpiredPeaks(float currentTime) {
    float cutoffTime = currentTime - coincidenceWindow_;
    
    // Remove peaks from head that are too old
    while (count_ > 0 && peaks_[head_].time < cutoffTime) {
        head_ = (head_ + 1) % MAX_PEAKS;
        count_--;
    }
}

void MultiBandFusion::addPeak(const BandPeak& peak) {
    if (count_ >= MAX_PEAKS) {
        // Buffer full - overwrite oldest
        head_ = (head_ + 1) % MAX_PEAKS;
        count_--;
    }
    
    size_t tail = (head_ + count_) % MAX_PEAKS;
    peaks_[tail] = peak;
    count_++;
}

bool MultiBandFusion::push(const BandPeak& peak) {
    // Remove expired peaks first
    removeExpiredPeaks(peak.time);
    
    // Add new peak
    addPeak(peak);
    
    // Analyze peaks in window
    bool hasLow = false;
    float totalStrength = 0.0f;

    for (size_t i = 0; i < count_; ++i) {
        size_t idx = (head_ + i) % MAX_PEAKS;
        const BandPeak& p = peaks_[idx];

        if (p.bandId == BAND_ID::LOW) {
            hasLow = true;
        }
        totalStrength += p.strength;
    }

    size_t numPeaks = count_;

    // Apply kick drum (low band) bonus
    float kickBonus = hasLow ? 0.5f : 0.0f;

    // Adaptive threshold based on number of peaks and kick presence
    float threshold = 1.5f - (static_cast<float>(numPeaks) * 0.2f) - kickBonus;
    threshold = std::max(0.3f, threshold);

    if (numPeaks >= 2 && totalStrength > threshold) {
        // Check minimum beat interval to prevent double-triggering
        float timeSinceLastBeat = peak.time - lastBeatTime_;
        if (lastBeatTime_ >= 0.0f && timeSinceLastBeat < minBeatInterval_) {
            if constexpr (DEBUG_MULTIBAND_FUSION) {
                std::cout << "[MultiBand] BLOCKED: timeSince=" << std::fixed << std::setprecision(4) 
                          << timeSinceLastBeat << "s < minInterval=" << minBeatInterval_ << "s" << std::endl;
            }
            return false;
        }

        if constexpr (DEBUG_MULTIBAND_FUSION) {
            std::cout << "[MultiBand] BEAT at t=" << std::fixed << std::setprecision(4) << peak.time 
                      << "s, peaks=" << numPeaks << ", strength=" << std::setprecision(2) << totalStrength 
                      << ", timeSince=" << std::setprecision(4) << timeSinceLastBeat << "s" << std::endl;
        }
        
        lastBeatTime_ = peak.time;
        return true;
    }
    
    return false;
}

void MultiBandFusion::setCoincidenceWindow(float window) {
    coincidenceWindow_ = window;
}

void MultiBandFusion::setMaxBpm(float maxBpm) {
    minBeatInterval_ = SECONDS_PER_MINUTE / maxBpm;
}

} // namespace jellED
