#include "sparklePattern.h"
#include "pattern_colors.h"

namespace jellED {

SparklePattern::SparklePattern(unsigned long startTime, int num_leds,
                               unsigned long pattern_duration_micros)
    : PatternBlueprint(startTime, PatternType::SPARKLE, pattern_duration_micros),
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
                                    unsigned long /*current_time_micros*/,
                                    pattern_color* output, int num_leds) {
    int leds = (num_leds > MAX_LEDS) ? MAX_LEDS : num_leds;

    // Sparkle probability scales with treble volume: more highs → more sparks
    float high_norm = static_cast<float>(features.volumeHigh) / 255.0f;
    float spawn_prob = SPARKLE_THRESHOLD + high_norm * 0.08f;

    // Base glow tracks bass volume (dim warm white)
    float base = static_cast<float>(features.volumeLow) / 255.0f * BASE_GLOW_SCALE;

    for (int i = 0; i < leds; ++i) {
        // Decay existing sparkle
        sparkle_level[i] *= SPARKLE_DECAY;

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
