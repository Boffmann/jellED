#include "unity.h"
#include "pattern/alternatingColors.h"

void test_ac_6_leds(void) {
    constexpr int NUM_LEDS = 6;
    AlternatingColors alternatingColors(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{255, 0, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{255, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 255, 0},   3);
    expected_pattern.set_color(pattern_color{255, 0, 0},   4);
    expected_pattern.set_color(pattern_color{0, 255, 0},   5);

    alternatingColors.update_pattern(0);
    // // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "1: red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, alternatingColors.get_color(i).red, str);
        sprintf(str, "1: green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, alternatingColors.get_color(i).green, str);
        sprintf(str, "1: blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, alternatingColors.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{255, 0, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{255, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 255, 0},   3);
    expected_pattern.set_color(pattern_color{255, 0, 0},   4);
    expected_pattern.set_color(pattern_color{0, 255, 0},   5);

    alternatingColors.update_pattern(500000);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "2: red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, alternatingColors.get_color(i).red, str);
        sprintf(str, "2: green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, alternatingColors.get_color(i).green, str);
        sprintf(str, "2: blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, alternatingColors.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{255, 0, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 0, 0},   3);
    expected_pattern.set_color(pattern_color{0, 255, 0},   4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    alternatingColors.update_pattern(100000);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "3: red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, alternatingColors.get_color(i).red, str);
        sprintf(str, "3: green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, alternatingColors.get_color(i).green, str);
        sprintf(str, "3: blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, alternatingColors.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{255, 0, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 0, 0},   3);
    expected_pattern.set_color(pattern_color{0, 255, 0},   4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    alternatingColors.update_pattern(300000);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "4: red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, alternatingColors.get_color(i).red, str);
        sprintf(str, "4: green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, alternatingColors.get_color(i).green, str);
        sprintf(str, "4: blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, alternatingColors.get_color(i).blue, str);
    }
}

void test_ac_6_leds_update_config(void) {
    constexpr int NUM_LEDS = 6;
    AlternatingColors alternatingColors(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{255, 0, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{255, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 255, 0},   3);
    expected_pattern.set_color(pattern_color{255, 0, 0},   4);
    expected_pattern.set_color(pattern_color{0, 255, 0},   5);

    alternatingColors.update_pattern(0);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, alternatingColors.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, alternatingColors.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, alternatingColors.get_color(i).blue, str);
    }

    pattern_config config;
    config.palette_color1 = pattern_color{100, 0, 0};
    config.palette_color2 = pattern_color{0, 100, 0};
    config.palette_color3 = pattern_color{0, 0, 100};

    alternatingColors.update_config(config);

    expected_pattern.set_color(pattern_color{100, 0, 0},   0);
    expected_pattern.set_color(pattern_color{0, 100, 0},   1);
    expected_pattern.set_color(pattern_color{100, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 100, 0},   3);
    expected_pattern.set_color(pattern_color{100, 0, 0},   4);
    expected_pattern.set_color(pattern_color{0, 100, 0},   5);

    alternatingColors.update_pattern(0);
    // TODO This can be better, right?
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, alternatingColors.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, alternatingColors.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, alternatingColors.get_color(i).blue, str);
    }
}
