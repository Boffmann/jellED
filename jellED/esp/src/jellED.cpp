#include <Arduino.h>
#include <cmath>

#define ONBOARD_LED_PIN 2

constexpr uint8_t LED_PIN = 13;
constexpr uint8_t LED_ENABLE = 14;
constexpr uint16_t NUM_LEDS = 2;

constexpr uint8_t MIC_WS_PIN = 25;
constexpr uint8_t MIC_SD_PIN = 32;
constexpr uint8_t MIC_SCK_PIN = 33;

constexpr uint8_t SPEAKER_OUT_PIN = 26;


long lastMillis = 0;
long loops = 0;


void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   Serial.println("ready");
   Serial.println(esp_get_idf_version());
   pinMode(ONBOARD_LED_PIN, OUTPUT);
}

void loop() {
   digitalWrite(ONBOARD_LED_PIN, HIGH);
   delay(1000);
   digitalWrite(ONBOARD_LED_PIN, LOW);
   delay(1000);
}