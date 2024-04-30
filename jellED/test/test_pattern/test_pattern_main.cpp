#include "unity.h"
#include <Arduino.h>
#include "test_patternEngine.hpp"
#include "test_coloredAmplitude.hpp"
#include "test_alternatingColors.hpp"
#include "test_patternEngineConfig.hpp"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_set_and_override_pattern_types);
  RUN_TEST(test_loop_around_after_last_pattern);

  RUN_TEST(test_pe_error_when_not_started);
  RUN_TEST(test_pe_6_leds_on_beat);
  RUN_TEST(test_pe_6_leds_switch_pattern_config);
  RUN_TEST(test_pe_switch_pattern_after_x_beats);
  RUN_TEST(test_pe_update_pattern_color_config);

  RUN_TEST(test_pca_5_leds_on_beat);
  RUN_TEST(test_pca_5_leds_cycle_update);
  RUN_TEST(test_pca_5_leds_cycle_reset);
  RUN_TEST(test_pca_6_leds_on_beat);
  RUN_TEST(test_pca_6_leds_cycle_update);
  RUN_TEST(test_pca_6_leds_cycle_reset);
  RUN_TEST(test_pca_6_leds_update_config);

  RUN_TEST(test_ac_6_leds);
  RUN_TEST(test_ac_6_leds_update_config);
  return UNITY_END();
}

/**
  * For Arduino framework
  */
void setup() {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  Serial.begin(115200);
  delay(2000);
  Serial.println("ready");

  runUnityTests();
}
void loop() {}