#include "MultibandFusion.h"

constexpr double SECONDS_PER_MINUTE = 60.0;

MultiBandFusion::MultiBandFusion(double coincidenceWindow, double maxBpm)
                : coincidenceWindow_(coincidenceWindow), 
                  lastBeatTime_(-1.0), 
                  minBeatInterval_(SECONDS_PER_MINUTE / maxBpm) {}

bool MultiBandFusion::push(const BandPeak& peak) {
    // Add peak to history
    peaks_.push_back(peak);
    
    // Remove old peaks outside coincidence window
    while (!peaks_.empty() && peak.time - peaks_.front().time > coincidenceWindow_) {
        peaks_.pop_front();
    }
    
    // Count bands and calculate weighted strength
    bool hasLow = false;
    double totalStrength = 0.0;
    
    for (const auto& p : peaks_) {
        if (p.bandId == 0) hasLow = true;
        
        totalStrength += p.strength;
    }
    
    int numPeaks = peaks_.size();
    
    // Apply kick drum (low band) bonus
    double kickBonus = hasLow ? 0.5 : 0.0;
    
    // Require at least 2 peaks (multi-band agreement) and sufficient combined strength
    // Threshold is adaptive based on number of peaks and kick presence
    double threshold = 1.5 - (numPeaks * 0.2) - kickBonus;
    threshold = std::max(0.3, threshold);  // Lower minimum when kick is present
    
    if (numPeaks >= 2 && totalStrength > threshold) {
        // Check minimum beat interval to prevent double-triggering
        if (lastBeatTime_ >= 0.0 && (peak.time - lastBeatTime_) < minBeatInterval_) {
            return false;
        }

        lastBeatTime_ = peak.time;
        
        return true;
    }
    
    return false;
}
