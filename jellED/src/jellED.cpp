#include "WS2812.h"

constexpr uint8_t LED_PIN = 13;
constexpr uint8_t LED_ENABLE = 14;
constexpr uint16_t NUM_LEDS = 2;

WS2812 strip = WS2812(LED_PIN, NUM_LEDS);
 
void setup() {
   pinMode(LED_ENABLE, OUTPUT);
   strip.initialize();
   strip.setBrightness(128);
   digitalWrite(LED_ENABLE, LOW);
}
 
void loop() {
   for (int i = 0; i < NUM_LEDS; ++i) {
      strip.setColorChannelsFor(i, 255, 0, 0);
   }
   strip.show();
   delay(1000);

   for (int i = 0; i < NUM_LEDS; ++i) {
      strip.setColorChannelsFor(i, 255, 0, 255);
   }
   
   strip.show();
   delay(1000);
}