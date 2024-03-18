#include "unity.h"
#include "patternEngine.h"

constexpr int NUM_LEDS = 6;
constexpr PatternType PATTERN_TYPE = PatternType::COLORED_AMPLITUDE;

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_error_when_not_started(void) {
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

void test_colored_amplitude_on_beat(void) {
    PatternEngine patternEngine(NUM_LEDS);

    Pattern expected_pattern(NUM_LEDS);
    expected_pattern.set_color(pattern_color{0, 255, 0},   0);
    expected_pattern.set_color(pattern_color{0, 255, 0},   1);
    expected_pattern.set_color(pattern_color{0, 255, 0},     2);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 3);
    expected_pattern.set_color(pattern_color{255, 255, 0}, 4);
    expected_pattern.set_color(pattern_color{255, 0, 0},   5);

    patternEngine.start(PatternType::COLORED_AMPLITUDE);
    const Pattern& pattern = patternEngine.generate_pattern(true);
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

// printf("Red expected: %d was: %s at index: %d",
//           expected_pattern.get_color(i).red,
//           pattern.get_color(i).red,
//           i)

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_error_when_not_started);
  RUN_TEST(test_colored_amplitude_on_beat);
  return UNITY_END();
}

/**
  * For Arduino framework
  */
void setup() {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}
void loop() {}