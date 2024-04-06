#include "unity.h"
#include <Arduino.h>
#include "test_patternEngine.hpp"
#include "test_coloredAmplitude.hpp"
#include "test_alternatingColors.hpp"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_pe_error_when_not_started);
  RUN_TEST(test_pe_6_leds_on_beat);
  RUN_TEST(test_pe_6_leds_switch_pattern);

  RUN_TEST(test_pca_5_leds_on_beat);
  RUN_TEST(test_pca_5_leds_cycle_update);
  RUN_TEST(test_pca_5_leds_cycle_reset);
  RUN_TEST(test_pca_6_leds_on_beat);
  RUN_TEST(test_pca_6_leds_cycle_update);
  RUN_TEST(test_pca_6_leds_cycle_reset);

  RUN_TEST(test_ac_6_leds);
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