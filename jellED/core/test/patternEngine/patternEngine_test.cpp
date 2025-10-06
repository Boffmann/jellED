#include "patternEngine.h"
#include "../utils/TestPlatformUtils.h"

#include <gtest/gtest.h>

using namespace jellED;

class PatternEngineTest : public testing::Test {
};

TEST_F(PatternEngineTest, GenerateBasicRainbowPattern) {
    TestPlatformUtils utils;
    PatternEngine patternEngine(utils, 1, 10000000, 2000000);
    const Pattern& generated_pattern = patternEngine.generate_pattern(false);
    ASSERT_EQ(generated_pattern.get_length(), 1);
    ASSERT_EQ(generated_pattern.get_color(0), (pattern_color{255, 153, 0, 127}));
    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{203, 255, 0, 0}));
    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{50, 255, 0, 0}));
    ASSERT_EQ(patternEngine.generate_pattern(true).get_color(0), (pattern_color{0, 255, 102, 255}));
    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{0, 254, 255, 127}));
    patternEngine.generate_pattern(false);
    patternEngine.generate_pattern(false);
    patternEngine.generate_pattern(false);
    patternEngine.generate_pattern(false);
    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{255, 0, 0, 0})); // Full circle after 10000 microseconds
    ASSERT_EQ(patternEngine.generate_pattern(true).get_color(0), (pattern_color{255, 153, 0, 255}));
}

TEST_F(PatternEngineTest, TestToggleReactToBeat) {
    TestPlatformUtils utils;
    PatternEngine patternEngine(utils, 1, 10000000, 2000000);

    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{255, 153, 0, 127}));
    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{203, 255, 0, 0}));
    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{50, 255, 0, 0}));
    ASSERT_EQ(patternEngine.generate_pattern(true).get_color(0), (pattern_color{0, 255, 102, 255}));

    patternEngine.turnOffReactToBeat();
    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{0, 254, 255, 255}));
    patternEngine.turnOnReactToBeat();
    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{0, 101, 255, 127}));
    patternEngine.turnOffReactToBeat();
    ASSERT_EQ(patternEngine.generate_pattern(false).get_color(0), (pattern_color{51, 0, 255, 255}));
}