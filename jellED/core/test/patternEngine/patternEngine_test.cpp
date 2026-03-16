#include "patternEngine.h"
#include "../utils/TestPlatformUtils.h"

#include <gtest/gtest.h>

using namespace jellED;

class PatternEngineTest : public testing::Test {
};

TEST_F(PatternEngineTest, GenerateBasicRainbowPattern) {
    TestPlatformUtils utils;
    PatternEngine patternEngine(utils, 1, 10000000, 2000000);

    const AudioFeatures no_beat{};
    AudioFeatures beat{};
    beat.beatFlags = AudioFeatures::BEAT_FUSED;

    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{255, 153, 0, 127}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{203, 255, 0, 0}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{50, 255, 0, 0}));
    ASSERT_EQ(patternEngine.generate_pattern(beat).get_color(0), (pattern_color{0, 255, 102, 255}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{0, 254, 255, 127}));
    patternEngine.generate_pattern(no_beat);
    patternEngine.generate_pattern(no_beat);
    patternEngine.generate_pattern(no_beat);
    patternEngine.generate_pattern(no_beat);
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{255, 0, 0, 0})); // Full circle after 10000000 microseconds
    ASSERT_EQ(patternEngine.generate_pattern(beat).get_color(0), (pattern_color{255, 153, 0, 255}));
}

TEST_F(PatternEngineTest, TestToggleReactToBeat) {
    TestPlatformUtils utils;
    PatternEngine patternEngine(utils, 1, 10000000, 2000000);

    const AudioFeatures no_beat{};
    AudioFeatures beat{};
    beat.beatFlags = AudioFeatures::BEAT_FUSED;

    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{255, 153, 0, 127}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{203, 255, 0, 0}));
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{50, 255, 0, 0}));
    ASSERT_EQ(patternEngine.generate_pattern(beat).get_color(0), (pattern_color{0, 255, 102, 255}));

    patternEngine.turnOffReactToBeat();
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{0, 254, 255, 255}));
    patternEngine.turnOnReactToBeat();
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{0, 101, 255, 127}));
    patternEngine.turnOffReactToBeat();
    ASSERT_EQ(patternEngine.generate_pattern(no_beat).get_color(0), (pattern_color{51, 0, 255, 255}));
}
