#include "patternEngine.h"
#include "../utils/TestPlatformUtils.h"

#include <gtest/gtest.h>

using namespace jellED;

class PatternEngineTest : public testing::Test {
};

// NOTE: Brightness values reflect the stateless exponential decay in
// RainbowPattern (tau = brightness_decay_micros/3). See
// audio_reactive_led_strip_learnings.md §4.1.

TEST_F(PatternEngineTest, GenerateBasicRainbowPattern) {
    TestPlatformUtils utils;
    PatternEngine patternEngine(utils, 1, 10000000, 2000000); // tau = 0.6667 s

    const AudioFeatures no_beat{};
    AudioFeatures beat{};
    beat.beatFlags = AudioFeatures::BEAT_FUSED;

    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{255, 153, 0, 56}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{203, 255, 0, 12}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{50, 255, 0, 2}));
    ASSERT_EQ(patternEngine.generate_pattern(beat).get_color(0), (pattern_color{0, 255, 102, 255}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{0, 254, 255, 56}));
    patternEngine.generate_pattern(no_beat);
    patternEngine.generate_pattern(no_beat);
    patternEngine.generate_pattern(no_beat);
    patternEngine.generate_pattern(no_beat);
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{255, 0, 0, 0})); // Full circle after 10000000 microseconds
    ASSERT_EQ(patternEngine.generate_pattern(beat).get_color(0), (pattern_color{255, 153, 0, 255}));
}

TEST_F(PatternEngineTest, TestToggleReactToBeat) {
    TestPlatformUtils utils;
    PatternEngine patternEngine(utils, 1, 10000000, 2000000); // tau = 0.6667 s

    const AudioFeatures no_beat{};
    AudioFeatures beat{};
    beat.beatFlags = AudioFeatures::BEAT_FUSED;

    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{255, 153, 0, 56}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{203, 255, 0, 12}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{50, 255, 0, 2}));
    ASSERT_EQ(patternEngine.generate_pattern(beat).get_color(0), (pattern_color{0, 255, 102, 255}));

    patternEngine.turnOffReactToBeat();
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{0, 254, 255, 255}));
    patternEngine.turnOnReactToBeat();
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{0, 101, 255, 56}));
    patternEngine.turnOffReactToBeat();
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{51, 0, 255, 255}));
}
