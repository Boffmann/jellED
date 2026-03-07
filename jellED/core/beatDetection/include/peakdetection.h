#ifndef __PEAKDETECTION_JELLED_H__
#define __PEAKDETECTION_JELLED_H__

#include "include/ringbuffer.h"
#include <cstdint>
#include <vector>
#include <memory>

namespace jellED {

struct PeakDetectorConfig {
    double absoluteMinThreshold;
    double thresholdRel;
    double maxBpm;

    // Per-band timing — see BeatDetectionConfig for descriptions
    double baselineAttackTime;
    double baselineReleaseTime;
    double thresholdRelaxTime;
    double onsetRatio;

    // Global tuning — see BeatDetectionConfig for descriptions
    double minRelativeThresholdFactor;
    double risingThresholdScale;
    double fallingThresholdScale;
};

class PeakDetector {
private:
    // Configuration parameters
    double absolute_min_threshold;
    double threshold_rel;
    double max_bpm;
    double onset_ratio;
    double min_relative_threshold_factor;
    double rising_threshold_scale;
    double falling_threshold_scale;
    double threshold_baseline; 

    // Envelope follower state
    double envelope;
    double baseline_attack_coeff;
    double baseline_release_coeff;
    double dynamic_threshold_rel;
    double min_dynamic_threshold_rel;
    double threshold_relax_coeff;
    
    // Peak detection state
    double prev_env;
    bool is_rising;
    double last_peak_time;
    
    // Onset detection: track recent minimum to require valley before peak
    double recent_min;
    double recent_min_decay_coeff;

    // Last computed threshold (updated each is_peak call)
    double last_threshold_;

    uint32_t sample_rate_;

    double update_envelope(double sample);

public:
    PeakDetector(const PeakDetectorConfig& config, uint32_t sample_rate);
    
    bool is_peak(double sample, double current_time);

    double getLastThreshold() const { return last_threshold_; }

    void setAbsoluteMinThreshold(double threshold);
    void setThresholdRel(double threshold);
    void setMaxBpm(double bpm);
    void setOnsetRatio(double ratio);
    void setTimingParams(double baselineAttackTime, double baselineReleaseTime,
                         double thresholdRelaxTime);
    void setHysteresisScales(double risingScale, double fallingScale);
    void setMinRelativeThresholdFactor(double factor);
};

} // end namespace jellED

#endif
