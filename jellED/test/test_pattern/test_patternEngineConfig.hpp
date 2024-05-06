#include "unity.h"
#include "config/patternEngineConfig.h"

void test_set_and_override_pattern_types(void) {
    PatternEngineConfig config;
    pattern_engine_config config_struct{ 
        PatternType::ALTERNATING_COLORS,
        PatternType::COLORED_AMPLITUDE,
        PatternType::ALTERNATING_COLORS,
        PatternType::COLORED_AMPLITUDE,
        4};

    config.update_config(config_struct);

    TEST_ASSERT_EQUAL(4, config.get_beats_per_pattern());
    TEST_ASSERT_EQUAL(PatternType::ALTERNATING_COLORS, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::COLORED_AMPLITUDE, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::ALTERNATING_COLORS, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::COLORED_AMPLITUDE, config.get_next_pattern_type());

    config_struct.pattern1 = PatternType::COLORED_AMPLITUDE;
    config_struct.pattern2 = PatternType::ALTERNATING_COLORS;
    config_struct.pattern3 = PatternType::COLORED_AMPLITUDE;
    config_struct.pattern4 = PatternType::ALTERNATING_COLORS;
    config_struct.beats_per_pattern = 6;

    config.update_config(config_struct);

    TEST_ASSERT_EQUAL(6, config.get_beats_per_pattern());
    TEST_ASSERT_EQUAL(PatternType::COLORED_AMPLITUDE, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::ALTERNATING_COLORS, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::COLORED_AMPLITUDE, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::ALTERNATING_COLORS, config.get_next_pattern_type());
}

void test_loop_around_after_last_pattern(void) {
    PatternEngineConfig config;

    pattern_engine_config config_struct{ 
        PatternType::ALTERNATING_COLORS,
        PatternType::COLORED_AMPLITUDE,
        PatternType::ALTERNATING_COLORS,
        PatternType::COLORED_AMPLITUDE,
        4};

    config.update_config(config_struct);

    TEST_ASSERT_EQUAL(4, config.get_beats_per_pattern());
    TEST_ASSERT_EQUAL(PatternType::ALTERNATING_COLORS, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::COLORED_AMPLITUDE, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::ALTERNATING_COLORS, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::COLORED_AMPLITUDE, config.get_next_pattern_type());

    TEST_ASSERT_EQUAL(PatternType::ALTERNATING_COLORS, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::COLORED_AMPLITUDE, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::ALTERNATING_COLORS, config.get_next_pattern_type());
    TEST_ASSERT_EQUAL(PatternType::COLORED_AMPLITUDE, config.get_next_pattern_type());
}