#ifndef __PEAKDETECTION_JELLED_H__
#define __PEAKDETECTION_JELLED_H__

#include "include/ringbuffer.h"
#include <cstdint>
#include <vector>
#include <memory>

namespace jellED {

struct PeakDetectorConfig {
    float absoluteMinThreshold;
    float thresholdRel;
    float maxBpm;

    // Per-band timing — see BeatDetectionConfig for descriptions
    float baselineAttackTime;
    float baselineReleaseTime;
    float thresholdRelaxTime;
    float onsetRatio;

    // Global tuning — see BeatDetectionConfig for descriptions
    float minRelativeThresholdFactor;
    float risingThresholdScale;
    float fallingThresholdScale;
};

class PeakDetector {
private:
    // Configuration parameters
    float absolute_min_threshold;
    float threshold_rel;
    float max_bpm;
    float onset_ratio;
    float min_relative_threshold_factor;
    float rising_threshold_scale;
    float falling_threshold_scale;
    float threshold_baseline;

    // Envelope follower state
    float envelope;
    float baseline_attack_coeff;
    float baseline_release_coeff;
    float dynamic_threshold_rel;
    float min_dynamic_threshold_rel;
    float threshold_relax_coeff;

    // Peak detection state
    float prev_env;
    bool is_rising;
    float last_peak_time;

    // Onset detection: track recent minimum to require valley before peak
    float recent_min;
    float recent_min_decay_coeff;

    // Last computed threshold (updated each is_peak call)
    float last_threshold_;

    uint32_t sample_rate_;

    float update_envelope(float sample);

public:
    PeakDetector(const PeakDetectorConfig& config, uint32_t sample_rate);

    bool is_peak(float sample, float current_time);

    float getLastThreshold() const { return last_threshold_; }

    void setAbsoluteMinThreshold(float threshold);
    void setThresholdRel(float threshold);
    void setMaxBpm(float bpm);
    void setOnsetRatio(float ratio);
    void setTimingParams(float baselineAttackTime, float baselineReleaseTime,
                         float thresholdRelaxTime);
    void setHysteresisScales(float risingScale, float fallingScale);
    void setMinRelativeThresholdFactor(float factor);
};

} // end namespace jellED

#endif
