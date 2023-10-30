#include <Arduino.h>
#include "musicPiece.h"
#include "soundconfig.h"
#include "speaker.h"
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
BeatDetector detector;

long lastMillis = 0;
long loops = 0;
 
void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   Serial.println("ready");
   piece.initialize();
   speaker.initialize();
}

void loop() {
   AudioBuffer audio;
   bool buffer_ready = piece.read(&audio);
   for (size_t sample = 0; sample < audio.num_samples;++sample) {
      uint8_t audio_value = audio.buffer[sample];
      speaker.play(audio_value);
      if (detector.is_beat(audio_value)) {
         Serial.print("Beat ");
         Serial.println(++loops);
      }
   }
}
