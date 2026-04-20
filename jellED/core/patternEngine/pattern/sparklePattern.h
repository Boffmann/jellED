#ifndef _SPARKLE_PATTERN_H_
#define _SPARKLE_PATTERN_H_

#include "patternBlueprint.h"

namespace jellED {

// Each LED has independent sparkle state — randomly lit pixels that flicker
// based on treble volume. Low-end volume sets a dim base glow for all LEDs.
// Uses a lightweight xorshift32 RNG (no stdlib rand needed on embedded targets).
//
// Sparkle decay uses a time-aware exponential curve via ExpFilter::alphaFromDt.
// Per-LED ExpFilter instances would cost unnecessary memory when all sparkles
// share the same time constant, so we apply the shared α manually.
class SparklePattern : public PatternBlueprint {
private:
    static constexpr int MAX_LEDS = 32;

    float sparkle_level[MAX_LEDS];     // per-LED brightness [0..1]
    unsigned long last_update_micros_; // dt source for time-aware decay
    uint32_t rng_state;                // xorshift32 state

    // Time constant for sparkle decay (seconds). 80ms keeps sparkles visible
    // for ~250ms (5% residual at 3×tau) — comfortable for the eye to track.
    static constexpr float SPARKLE_TAU_DECAY_S = 0.080f;
    static constexpr float SPARKLE_THRESHOLD   = 0.003f; // probability floor
    static constexpr float BASE_GLOW_SCALE     = 0.5f;   // ambient from volumeLow

    uint32_t xorshift32();

public:
    SparklePattern(unsigned long startTime, int num_leds,
                   unsigned long pattern_duration_micros = 10000000UL);

    void update_pattern(const AudioFeatures& features, unsigned long current_time_micros,
                        pattern_color* output, int num_leds) override;
};

} // namespace jellED

#endif
