#include "pattern/rainbowPattern.h"

#include <gtest/gtest.h>

using namespace jellED;

class RainbowPatternTest : public testing::Test {
};

TEST_F(RainbowPatternTest, TestRainbowPatternNonBeat) {
    RainbowPattern rainbowPattern(0, 10000000);
    rainbowPattern.shouldReactToBeat(false, 0);
    ASSERT_EQ(rainbowPattern.get_pattern_type(), PatternType::RAINBOW);

    pattern_color output[1];
    const AudioFeatures no_beat{};

    rainbowPattern.update_pattern(no_beat, 0, output, 1);
    ASSERT_EQ(output[0], RED);
    rainbowPattern.update_pattern(no_beat, 1000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 255}));
    rainbowPattern.update_pattern(no_beat, 2000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{203, 255, 0, 255}));
    rainbowPattern.update_pattern(no_beat, 3000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{50, 255, 0, 255}));
    rainbowPattern.update_pattern(no_beat, 4000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 255, 102, 255}));
    rainbowPattern.update_pattern(no_beat, 5000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 254, 255, 255}));
    rainbowPattern.update_pattern(no_beat, 10000000, output, 1);
    ASSERT_EQ(output[0], RED); // Full circle after 10000000 microseconds
    rainbowPattern.update_pattern(no_beat, 11000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 255}));
}

// NOTE: Brightness values in these tests reflect the stateless exponential
// decay introduced by the ExpFilter refactor. tau = brightness_decay_micros/3,
// so brightness(t) = 255 * exp(-t/tau). The numeric values were updated from
// the previous linear-decay implementation; see audio_reactive_led_strip_
// learnings.md §4.1 for the rationale.

TEST_F(RainbowPatternTest, TestRainbowPatternOddBrightnessDecay) {
    RainbowPattern rainbowPattern(0, 10000000, 12345678); // tau = 4.1152 s
    rainbowPattern.shouldReactToBeat(true, 0);
    ASSERT_EQ(rainbowPattern.get_pattern_type(), PatternType::RAINBOW);

    pattern_color output[1];
    const AudioFeatures no_beat{};
    AudioFeatures beat{};
    beat.beatFlags = AudioFeatures::BEAT_FUSED;

    rainbowPattern.update_pattern(no_beat, 0, output, 1);
    ASSERT_EQ(output[0], RED);
    rainbowPattern.update_pattern(no_beat, 1000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 199}));
    rainbowPattern.update_pattern(no_beat, 2000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{203, 255, 0, 156}));
    rainbowPattern.update_pattern(beat, 3000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{50, 255, 0, 255}));
    rainbowPattern.update_pattern(no_beat, 4000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 255, 102, 199}));
    rainbowPattern.update_pattern(no_beat, 5000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 254, 255, 156}));
    rainbowPattern.update_pattern(no_beat, 6000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 101, 255, 123}));
    rainbowPattern.update_pattern(no_beat, 7000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{51, 0, 255, 96}));
    rainbowPattern.update_pattern(no_beat, 10000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 0, 0, 46})); // Full circle after 10000000 microseconds
    rainbowPattern.update_pattern(no_beat, 11000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 36}));
}

TEST_F(RainbowPatternTest, TestRainbowPatternBeat) {
    RainbowPattern rainbowPattern(0, 10000000, 2000000); // tau = 0.6667 s
    ASSERT_EQ(rainbowPattern.get_pattern_type(), PatternType::RAINBOW);

    pattern_color output[1];
    const AudioFeatures no_beat{};
    AudioFeatures beat{};
    beat.beatFlags = AudioFeatures::BEAT_FUSED;

    rainbowPattern.update_pattern(no_beat, 0, output, 1);
    ASSERT_EQ(output[0], RED);
    rainbowPattern.update_pattern(no_beat, 1000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 56}));
    rainbowPattern.update_pattern(no_beat, 2000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{203, 255, 0, 12}));
    rainbowPattern.update_pattern(no_beat, 3000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{50, 255, 0, 2}));
    rainbowPattern.update_pattern(beat, 4000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 255, 102, 255}));
    rainbowPattern.update_pattern(no_beat, 5000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 254, 255, 56}));
    rainbowPattern.update_pattern(no_beat, 10000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 0, 0, 0})); // Full circle after 10000000 microseconds
    rainbowPattern.update_pattern(beat, 11000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 255}));
}

TEST_F(RainbowPatternTest, TestToggleReactToBeat) {
    RainbowPattern rainbowPattern(0, 10000000, 2000000); // tau = 0.6667 s
    rainbowPattern.shouldReactToBeat(false, 0);
    ASSERT_EQ(rainbowPattern.get_pattern_type(), PatternType::RAINBOW);

    pattern_color output[1];
    const AudioFeatures no_beat{};
    AudioFeatures beat{};
    beat.beatFlags = AudioFeatures::BEAT_FUSED;

    rainbowPattern.update_pattern(no_beat, 0, output, 1);
    ASSERT_EQ(output[0], RED);
    rainbowPattern.update_pattern(no_beat, 1000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 255}));
    rainbowPattern.update_pattern(no_beat, 2000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{203, 255, 0, 255}));
    rainbowPattern.update_pattern(beat, 3000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{50, 255, 0, 255}));
    rainbowPattern.shouldReactToBeat(true, 3000000);
    rainbowPattern.update_pattern(no_beat, 4000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 255, 102, 56}));
    rainbowPattern.update_pattern(no_beat, 5000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 254, 255, 12}));
    rainbowPattern.update_pattern(beat, 6000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 101, 255, 255}));
    rainbowPattern.update_pattern(no_beat, 10000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 0, 0, 0})); // Full circle after 10000000 microseconds
    rainbowPattern.update_pattern(beat, 11000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 255}));
}
