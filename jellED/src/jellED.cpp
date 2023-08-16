#include "musicPiece.h"
#include <Arduino.h>

constexpr uint8_t LED_PIN = 13;
constexpr uint8_t LED_ENABLE = 14;
constexpr uint16_t NUM_LEDS = 2;

constexpr uint8_t MIC_WS_PIN = 25;
constexpr uint8_t MIC_SD_PIN = 32;
constexpr uint8_t MIC_SCK_PIN = 33;
MusicPiece piece;
 
void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   Serial.println("ready");
   MusicPiece piece;
}
 
void loop() {
   int rangelimit = 20000;
   Serial.print(rangelimit*-1);
   Serial.print(" ");
   Serial.print(rangelimit);
   Serial.print(" ");
   AudioBuffer buffer;
   bool buffer_ready = piece.read(&buffer);
   if (buffer_ready) {
      if (buffer.num_samples > 0) {
         float mean = 0;
         for (int16_t i = 0; i < buffer.num_samples; ++i) {
            mean += buffer.buffer[i];
         }

         mean /= buffer.num_samples;
         Serial.println(mean);
      }
   }
}
