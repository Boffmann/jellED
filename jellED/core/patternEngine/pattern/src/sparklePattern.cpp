#include "sparklePattern.h"
#include "expFilter.h"
#include "pattern_colors.h"

namespace jellED {

SparklePattern::SparklePattern(unsigned long startTime, int num_leds,
                               unsigned long pattern_duration_micros)
    : PatternBlueprint(startTime, PatternType::SPARKLE, pattern_duration_micros),
      last_update_micros_(0),
      rng_state(0xDEADBEEFu) {
    int capped = (num_leds > MAX_LEDS) ? MAX_LEDS : num_leds;
    for (int i = 0; i < capped; ++i) {
        sparkle_level[i] = 0.0f;
    }
}

uint32_t SparklePattern::xorshift32() {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

void SparklePattern::update_pattern(const AudioFeatures& features,
                                    unsigned long current_time_micros,
                                    pattern_color* output, int num_leds) {
    int leds = (num_leds > MAX_LEDS) ? MAX_LEDS : num_leds;

    // ── dt bookkeeping ────────────────────────────────────────────────────────
    float dt_seconds;
    if (last_update_micros_ == 0) {
        dt_seconds = 0.0f;
    } else {
        dt_seconds = static_cast<float>(current_time_micros - last_update_micros_) * 1e-6f;
    }
    last_update_micros_ = current_time_micros;

    // Shared decay coefficient — one exp() call amortized over all LEDs.
    // Each sparkle_level shrinks by (1-alpha) per frame, which is the
    // exponential decay that ExpFilter would apply with input=0.
    const float alpha_decay = ExpFilter<float>::alphaFromDt(SPARKLE_TAU_DECAY_S, dt_seconds);
    const float decay_factor = 1.0f - alpha_decay;

    // Sparkle probability scales with treble volume: more highs → more sparks
    const float high_norm = static_cast<float>(features.volumeHigh) / 255.0f;
    const float spawn_prob = SPARKLE_THRESHOLD + high_norm * 0.08f;

    // Base glow tracks bass volume (dim warm white)
    const float base = static_cast<float>(features.volumeLow) / 255.0f * BASE_GLOW_SCALE;

    for (int i = 0; i < leds; ++i) {
        // Decay existing sparkle (time-aware; equivalent to ExpFilter toward 0)
        sparkle_level[i] *= decay_factor;

        // Randomly ignite new sparkle
        float rnd = static_cast<float>(xorshift32()) / static_cast<float>(0xFFFFFFFFu);
        if (rnd < spawn_prob) {
            sparkle_level[i] = 1.0f;
        }

        float level = sparkle_level[i];
        float total = level + base;
        if (total > 1.0f) total = 1.0f;

        // Sparkle color: cold white (biased toward blue/white)
        output[i] = pattern_color{
            static_cast<uint8_t>(180.0f * level + 80.0f * base),  // R: less on sparkle
            static_cast<uint8_t>(200.0f * level + 80.0f * base),  // G
            static_cast<uint8_t>(255.0f * level + 80.0f * base),  // B: most on sparkle
            static_cast<uint8_t>(total * 255.0f)
        };
    }
}

} // namespace jellED
