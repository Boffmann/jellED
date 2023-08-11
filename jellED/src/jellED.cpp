#include "INMP441.h"
#include <Arduino.h>

constexpr uint8_t LED_PIN = 13;
constexpr uint8_t LED_ENABLE = 14;
constexpr uint16_t NUM_LEDS = 2;

constexpr uint8_t MIC_WS_PIN = 25;
constexpr uint8_t MIC_SD_PIN = 32;
constexpr uint8_t MIC_SCK_PIN = 33;

INMP441 mic(MIC_WS_PIN, MIC_SD_PIN, MIC_SCK_PIN);
 
void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   mic.initialize();
   
   Serial.println("ready");
}
 
void loop() {
   int rangelimit = 2000;
   Serial.print(rangelimit*-1);
   Serial.print(" ");
   Serial.print(rangelimit);
   Serial.print(" ");

   size_t bytesIn;
   bool buffer_ready;
   int16_t* buffer = mic.read(&bytesIn, &buffer_ready);
   if (buffer_ready) {
      int16_t samples_read = bytesIn / 8;
      if (samples_read > 0) {
         float mean = 0;
         for (int16_t i = 0; i < samples_read; ++i) {
            mean += buffer[i];
         }

         mean /= samples_read;
         Serial.println(mean);
      }
   }
}