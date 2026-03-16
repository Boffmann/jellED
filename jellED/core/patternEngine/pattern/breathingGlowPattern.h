#ifndef _BREATHING_GLOW_PATTERN_H_
#define _BREATHING_GLOW_PATTERN_H_

#include "patternBlueprint.h"

namespace jellED {

// All LEDs show the same color and brightness, breathing in and out.
// Color tracks spectral tilt (bass=warm orange, treble=cool blue via HSV).
// Brightness follows smoothed bass volume; a beat spikes to 255 and decays.
class BreathingGlowPattern : public PatternBlueprint {
private:
    float smoothed_brightness;   // IIR-smoothed target [0..255]
    float beat_brightness;       // current beat spike level [0..255]

    static constexpr float SMOOTHING_ALPHA  = 0.08f; // low-pass for breathing feel
    static constexpr float BEAT_DECAY_RATE  = 12.0f; // units/ms decay after beat

public:
    BreathingGlowPattern(unsigned long startTime,
                         unsigned long pattern_duration_micros = 10000000UL);

    void update_pattern(const AudioFeatures& features, unsigned long current_time_micros,
                        pattern_color* output, int num_leds) override;
};

} // namespace jellED

#endif
