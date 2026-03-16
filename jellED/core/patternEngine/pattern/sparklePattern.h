#ifndef _SPARKLE_PATTERN_H_
#define _SPARKLE_PATTERN_H_

#include "patternBlueprint.h"

namespace jellED {

// Each LED has independent sparkle state — randomly lit pixels that flicker
// based on treble volume. Low-end volume sets a dim base glow for all LEDs.
// Uses a lightweight xorshift32 RNG (no stdlib rand needed on embedded targets).
class SparklePattern : public PatternBlueprint {
private:
    static constexpr int MAX_LEDS = 32;

    float sparkle_level[MAX_LEDS]; // per-LED brightness [0..1]
    uint32_t rng_state;            // xorshift32 state

    static constexpr float SPARKLE_DECAY     = 0.88f;  // per-frame multiplier
    static constexpr float SPARKLE_THRESHOLD = 0.003f; // probability floor
    static constexpr float BASE_GLOW_SCALE   = 0.5f;   // ambient from volumeLow

    uint32_t xorshift32();

public:
    SparklePattern(unsigned long startTime, int num_leds,
                   unsigned long pattern_duration_micros = 10000000UL);

    void update_pattern(const AudioFeatures& features, unsigned long current_time_micros,
                        pattern_color* output, int num_leds) override;
};

} // namespace jellED

#endif
