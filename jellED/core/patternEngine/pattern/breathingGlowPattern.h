#ifndef _BREATHING_GLOW_PATTERN_H_
#define _BREATHING_GLOW_PATTERN_H_

#include "expFilter.h"
#include "patternBlueprint.h"

namespace jellED {

// All LEDs show the same color and brightness, breathing in and out.
// Color tracks spectral tilt (bass=warm orange, treble=cool blue via HSV).
// Brightness follows smoothed bass volume; a beat spikes to 255 and decays.
class BreathingGlowPattern : public PatternBlueprint {
private:
    // Asymmetric exponential smoother: snaps up fast to loud bass, decays
    // slowly for a sustained warm afterglow. Matches the word "breathing".
    ExpFilter<float> brightness_filter_;
    unsigned long last_update_micros_;  // 0 until first update — used to compute dt

    float beat_brightness;       // current beat spike level [0..255]

    // Time constants (seconds) for the brightness smoother.
    static constexpr float BRIGHTNESS_TAU_RISE_S  = 0.030f; // 30ms — snap up to beats
    static constexpr float BRIGHTNESS_TAU_DECAY_S = 0.500f; // 500ms — sustained afterglow
    static constexpr float BEAT_DECAY_RATE        = 12.0f;  // units/ms decay after beat

public:
    BreathingGlowPattern(unsigned long startTime,
                         unsigned long pattern_duration_micros = 10000000UL);

    void update_pattern(const AudioFeatures& features, unsigned long current_time_micros,
                        pattern_color* output, int num_leds) override;
};

} // namespace jellED

#endif
