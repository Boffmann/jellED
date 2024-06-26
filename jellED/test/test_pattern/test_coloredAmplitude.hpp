#include "unity.h"
#include "pattern/coloredAmplitude.h"

void test_pca_5_leds_on_beat(void) {
    constexpr int NUM_LEDS = 5;
    ColoredAmplitude coloredAmplitude(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{255, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 0, 0}, 4);

    coloredAmplitude.update_pattern(0);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }
}

void test_pca_5_leds_cycle_update(void) {
    constexpr int NUM_LEDS = 5;
    ColoredAmplitude coloredAmplitude(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{255, 255, 0},   2);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 3);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 4);

    coloredAmplitude.update_pattern(250000);
    // // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 3);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 4);

    coloredAmplitude.update_pattern(375000);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{0, 0, 0},   0);
    expected_pattern.set_color(pattern_color{0, 0, 0},   1);
    expected_pattern.set_color(pattern_color{0, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 3);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 4);

    coloredAmplitude.update_pattern(500000);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }
}

void test_pca_5_leds_cycle_reset(void) {
    constexpr int NUM_LEDS = 5;
    ColoredAmplitude coloredAmplitude(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{255, 255, 0},   2);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 3);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 4);

    coloredAmplitude.update_pattern(250000);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{255, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 0, 0}, 4);

    coloredAmplitude.update_pattern(0);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }
}

// Even number of leds

void test_pca_6_leds_on_beat(void) {
    constexpr int NUM_LEDS = 6;
    ColoredAmplitude coloredAmplitude(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    coloredAmplitude.update_pattern(0);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }
}

void test_pca_6_leds_cycle_update(void) {
    constexpr int NUM_LEDS = 6;
    ColoredAmplitude coloredAmplitude(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 3);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 4);
    expected_pattern.set_color(pattern_color{0, 0, 0},   5);

    coloredAmplitude.update_pattern(250000);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 3);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 4);
    expected_pattern.set_color(pattern_color{0, 0, 0},   5);

    coloredAmplitude.update_pattern(375000);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{0, 0, 0},   0);
    expected_pattern.set_color(pattern_color{0, 0, 0},   1);
    expected_pattern.set_color(pattern_color{0, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 3);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 4);
    expected_pattern.set_color(pattern_color{0, 0, 0},   5);

    coloredAmplitude.update_pattern(500000);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }
}

void test_pca_6_leds_cycle_reset(void) {
    constexpr int NUM_LEDS = 6;
    ColoredAmplitude coloredAmplitude(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 3);
    expected_pattern.set_color(pattern_color{0, 0, 0}, 4);
    expected_pattern.set_color(pattern_color{0, 0, 0},   5);

    coloredAmplitude.update_pattern(250000);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }

    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    coloredAmplitude.update_pattern(0);
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }
}

void test_pca_6_leds_update_config(void) {
    constexpr int NUM_LEDS = 6;
    ColoredAmplitude coloredAmplitude(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},   2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    coloredAmplitude.update_pattern(0);
    // TODO This can be better, right?
    char str[15];
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }

    pattern_config config;
    config.palette_color1 = pattern_color{100, 0, 0};
    config.palette_color2 = pattern_color{0, 100, 0};
    config.palette_color3 = pattern_color{0, 0, 100};

    coloredAmplitude.update_config(config);

    expected_pattern.set_color(pattern_color{100, 0, 0},   0);
    expected_pattern.set_color(pattern_color{100, 0, 0},   1);
    expected_pattern.set_color(pattern_color{100, 0, 0},   2);
    expected_pattern.set_color(pattern_color{0, 100, 0},   3);
    expected_pattern.set_color(pattern_color{0, 100, 0}, 4);
    expected_pattern.set_color(pattern_color{0, 0, 100}, 5);

    coloredAmplitude.update_pattern(0);
    // TODO This can be better, right?
    for (int i = 0; i < NUM_LEDS; ++i) {
        sprintf(str, "red i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).red, coloredAmplitude.get_color(i).red, str);
        sprintf(str, "green i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).green, coloredAmplitude.get_color(i).green, str);
        sprintf(str, "blue i: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(expected_pattern.get_color(i).blue, coloredAmplitude.get_color(i).blue, str);
    }
}
