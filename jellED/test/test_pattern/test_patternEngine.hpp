#include "unity.h"
#include "patternEngine.h"

constexpr int NUM_LEDS = 6;
constexpr PatternType PATTERN_TYPE = PatternType::COLORED_AMPLITUDE;

void test_pe_error_when_not_started(void) {
    PatternEngine patternEngine(NUM_LEDS);
    Pattern error_pattern(NUM_LEDS);
    for (int i = 0; i < NUM_LEDS; ++i) {
        error_pattern.set_color(rainbow_colors[RED_INDEX], i);
    }
    const Pattern& pattern = patternEngine.generate_pattern(true);
    for (int i = 0; i < NUM_LEDS; ++i) {
        TEST_ASSERT_EQUAL(error_pattern.get_color(i).red, pattern.get_color(i).red);
        TEST_ASSERT_EQUAL(error_pattern.get_color(i).green, pattern.get_color(i).green);
        TEST_ASSERT_EQUAL(error_pattern.get_color(i).blue, pattern.get_color(i).blue);
    }
}

void test_pe_6_leds_on_beat(void) {
    PatternEngine patternEngine(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    pattern_engine_config config{ 
        PatternType::COLORED_AMPLITUDE,
        PatternType::COLORED_AMPLITUDE,
        PatternType::COLORED_AMPLITUDE,
        PatternType::COLORED_AMPLITUDE,
        4};

    patternEngine.start(config);
    const Pattern& pattern = patternEngine.generate_pattern(true);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, pattern.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, pattern.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, pattern.get_color(i).blue, str);
    }
}

void test_pe_6_leds_switch_pattern_config(void) {
    PatternEngine patternEngine(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    pattern_engine_config config{ 
        PatternType::COLORED_AMPLITUDE,
        PatternType::COLORED_AMPLITUDE,
        PatternType::COLORED_AMPLITUDE,
        PatternType::COLORED_AMPLITUDE,
        4};

    patternEngine.start(config);
    const Pattern& ca_pattern = patternEngine.generate_pattern(true);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, ca_pattern.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, ca_pattern.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, ca_pattern.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{255, 0, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{255, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 255, 0},   3);
    expected_pattern.set_color(pattern_color{255, 0, 0},   4);
    expected_pattern.set_color(pattern_color{0, 255, 0},   5);

    config.pattern1 = PatternType::ALTERNATING_COLORS;
    config.pattern2 = PatternType::ALTERNATING_COLORS;
    config.pattern3 = PatternType::ALTERNATING_COLORS;
    config.pattern4 = PatternType::ALTERNATING_COLORS;

    patternEngine.update_config(config);
    const Pattern& ac_pattern = patternEngine.generate_pattern(true);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, ac_pattern.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, ac_pattern.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, ac_pattern.get_color(i).blue, str);
    }
}

void test_pe_switch_pattern_after_x_beats(void) {
    PatternEngine patternEngine(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    pattern_engine_config config{ 
        PatternType::COLORED_AMPLITUDE,
        PatternType::ALTERNATING_COLORS,
        PatternType::COLORED_AMPLITUDE,
        PatternType::COLORED_AMPLITUDE,
        2};

    patternEngine.start(config);
    const Pattern& ca_pattern = patternEngine.generate_pattern(true);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, ca_pattern.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, ca_pattern.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, ca_pattern.get_color(i).blue, str);
    }

    const Pattern& ca_pattern2 =  patternEngine.generate_pattern(true);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, ca_pattern2.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, ca_pattern2.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, ca_pattern2.get_color(i).blue, str);
    }

    // After two beats, we should see the AlternaticColors pattern

    expected_pattern.set_color(pattern_color{255, 0, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{255, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 255, 0},   3);
    expected_pattern.set_color(pattern_color{255, 0, 0},   4);
    expected_pattern.set_color(pattern_color{0, 255, 0},   5);

    const Pattern& ac_pattern = patternEngine.generate_pattern(true);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, ac_pattern.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, ac_pattern.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, ac_pattern.get_color(i).blue, str);
    }
}

void test_pe_update_pattern_color_config(void) {
    PatternEngine patternEngine(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    pattern_engine_config pattern_engine_config{ 
        PatternType::COLORED_AMPLITUDE,
        PatternType::ALTERNATING_COLORS,
        PatternType::COLORED_AMPLITUDE,
        PatternType::COLORED_AMPLITUDE,
        2};

    patternEngine.start(pattern_engine_config);
    const Pattern& ca_pattern = patternEngine.generate_pattern(true);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, ca_pattern.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, ca_pattern.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, ca_pattern.get_color(i).blue, str);
    }

    pattern_config pattern_color_config{pattern_color{100, 0, 0}, pattern_color{0, 100, 0}, pattern_color{0, 0, 100}};

    patternEngine.update_pattern_color_config(pattern_color_config);

    expected_pattern.set_color(pattern_color{100, 0, 0},   0);
    expected_pattern.set_color(pattern_color{100, 0, 0},   1);
    expected_pattern.set_color(pattern_color{100, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 100, 0},   3);
    expected_pattern.set_color(pattern_color{0, 100, 0},   4);
    expected_pattern.set_color(pattern_color{0, 0, 100},   5);

    const Pattern& ca_pattern2 = patternEngine.generate_pattern(true);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, ca_pattern.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, ca_pattern.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, ca_pattern.get_color(i).blue, str);
    }

    // After two beats, we should see the AlternaticColors pattern

    expected_pattern.set_color(pattern_color{100, 0, 0},   0);
    expected_pattern.set_color(pattern_color{0, 100, 0},   1);
    expected_pattern.set_color(pattern_color{100, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 100, 0},   3);
    expected_pattern.set_color(pattern_color{100, 0, 0},   4);
    expected_pattern.set_color(pattern_color{0, 100, 0},   5);

    const Pattern& ac_pattern = patternEngine.generate_pattern(true);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, ac_pattern.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, ac_pattern.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, ac_pattern.get_color(i).blue, str);
    }

}
