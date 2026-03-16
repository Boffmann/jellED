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

TEST_F(RainbowPatternTest, TestRainbowPatternOddBrightnessDecay) {
    RainbowPattern rainbowPattern(0, 10000000, 12345678);
    rainbowPattern.shouldReactToBeat(true, 0);
    ASSERT_EQ(rainbowPattern.get_pattern_type(), PatternType::RAINBOW);

    pattern_color output[1];
    const AudioFeatures no_beat{};
    AudioFeatures beat{};
    beat.beatFlags = AudioFeatures::BEAT_FUSED;

    rainbowPattern.update_pattern(no_beat, 0, output, 1);
    ASSERT_EQ(output[0], RED);
    rainbowPattern.update_pattern(no_beat, 1000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 234}));
    rainbowPattern.update_pattern(no_beat, 2000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{203, 255, 0, 213}));
    rainbowPattern.update_pattern(beat, 3000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{50, 255, 0, 255}));
    rainbowPattern.update_pattern(no_beat, 4000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 255, 102, 234}));
    rainbowPattern.update_pattern(no_beat, 5000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 254, 255, 213}));
    rainbowPattern.update_pattern(no_beat, 6000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 101, 255, 193}));
    rainbowPattern.update_pattern(no_beat, 7000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{51, 0, 255, 172}));
    rainbowPattern.update_pattern(no_beat, 10000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 0, 0, 110})); // Full circle after 10000000 microseconds
    rainbowPattern.update_pattern(no_beat, 11000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 89}));
}

TEST_F(RainbowPatternTest, TestRainbowPatternBeat) {
    RainbowPattern rainbowPattern(0, 10000000, 2000000);
    ASSERT_EQ(rainbowPattern.get_pattern_type(), PatternType::RAINBOW);

    pattern_color output[1];
    const AudioFeatures no_beat{};
    AudioFeatures beat{};
    beat.beatFlags = AudioFeatures::BEAT_FUSED;

    rainbowPattern.update_pattern(no_beat, 0, output, 1);
    ASSERT_EQ(output[0], RED);
    rainbowPattern.update_pattern(no_beat, 1000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 127}));
    rainbowPattern.update_pattern(no_beat, 2000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{203, 255, 0, 0}));
    rainbowPattern.update_pattern(no_beat, 3000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{50, 255, 0, 0}));
    rainbowPattern.update_pattern(beat, 4000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 255, 102, 255}));
    rainbowPattern.update_pattern(no_beat, 5000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 254, 255, 127}));
    rainbowPattern.update_pattern(no_beat, 10000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 0, 0, 0})); // Full circle after 10000000 microseconds
    rainbowPattern.update_pattern(beat, 11000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 255}));
}

TEST_F(RainbowPatternTest, TestToggleReactToBeat) {
    RainbowPattern rainbowPattern(0, 10000000, 2000000);
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
    ASSERT_EQ(output[0], (pattern_color{0, 255, 102, 127}));
    rainbowPattern.update_pattern(no_beat, 5000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 254, 255, 0}));
    rainbowPattern.update_pattern(beat, 6000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{0, 101, 255, 255}));
    rainbowPattern.update_pattern(no_beat, 10000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 0, 0, 0})); // Full circle after 10000000 microseconds
    rainbowPattern.update_pattern(beat, 11000000, output, 1);
    ASSERT_EQ(output[0], (pattern_color{255, 153, 0, 255}));
}
