#include <Arduino.h>
#include "musicPiece.h"
#include "soundconfig.h"
#include "speaker.h"
#include "INMP441.h"
#include "beatdetection.h"
#include "patternEngine.h"

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
PatternEngine patternEngine(NUM_LEDS);

long lastMillis = 0;
long loops = 0;
 
void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   Serial.println("ready");
   pinMode(2, OUTPUT);
   piece.initialize();
   //mic.initialize();
   speaker.initialize();
   patternEngine.start(PatternType::COLORED_AMPLITUDE);
}

uint8_t convert8Bit(int16_t in) {
   float intermediate = 32768.0f + (float) in;
   return (uint8_t) (intermediate * 0.00389105058);
}

uint16_t convertUnsigned(int16_t in) {
   return (uint16_t) 32768 + in;
}

void loop() {
   AudioBuffer audio;

   bool buffer_ready = piece.read(&audio);
   for (size_t sample = 0; sample < audio.num_samples;++sample) {
      int16_t audio_value = audio.buffer[sample];
      speaker.play(convert8Bit(audio_value));
      const Pattern& pattern = patternEngine.generate_pattern(detector.is_beat(audio_value));
      for (int i = 0; i < pattern.get_length(); ++i) {
         const pattern_color& color = pattern.get_color(i);
         Serial.print("Red ");
         Serial.print(color.red);
         Serial.print(" Green: ");
         Serial.print(color.green);
         Serial.print(" Blue: ");
         Serial.println(color.blue);
      }
   }
}
