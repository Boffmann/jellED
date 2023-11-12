#include <Arduino.h>
#include "musicPiece.h"
#include "soundconfig.h"
#include "speaker.h"
#include "INMP441.h"
#include "beatdetection.h"

constexpr uint8_t LED_PIN = 13;
constexpr uint8_t LED_ENABLE = 14;
constexpr uint16_t NUM_LEDS = 2;

constexpr uint8_t MIC_WS_PIN = 25;
constexpr uint8_t MIC_SD_PIN = 32;
constexpr uint8_t MIC_SCK_PIN = 33;

constexpr uint8_t SPEAKER_OUT_PIN = 26;

MusicPiece piece;
Speaker speaker(SAMPLE_RATE);
//INMP441 mic(MIC_WS_PIN, MIC_SD_PIN, MIC_SCK_PIN);
BeatDetector detector;

long lastMillis = 0;
long loops = 0;
 
void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   Serial.println("ready");
   piece.initialize();
   //mic.initialize();
   speaker.initialize();
}

uint8_t convert(int16_t in) {
   float intermediate = 32768.0f + (float) in;
   return (uint8_t) (intermediate * 0.00389105058);
}

void loop() {

   AudioBuffer audio;
   /*if (mic.read(&audio)) {
      if (audio.num_samples > 0) {
         //float mean = 0.f;
         for (size_t i = 0; i < audio.num_samples; i++) {
            //mean += audio.buffer[i];
            if (detector.is_beat(audio.buffer[i])) {
               Serial.print("Beat ");
               Serial.println(++loops);
            }
         }
         //Serial.println(mean / audio.num_samples);
      }
   }*/

   bool buffer_ready = piece.read(&audio);
   for (size_t sample = 0; sample < audio.num_samples;++sample) {
      int16_t audio_value = audio.buffer[sample];
      speaker.play(convert(audio_value));
      if (detector.is_beat(audio_value)) {
         Serial.print("Beat ");
         Serial.println(++loops);
      }
   }
}
