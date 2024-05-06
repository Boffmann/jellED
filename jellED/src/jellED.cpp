#include <Arduino.h>
#include "musicPiece.h"
#include "soundconfig.h"
#include "speaker.h"
#include "INMP441.h"
#include "WS2812.h"
#include "beatdetection.h"
#include "patternEngine.h"
#include "BluetoothInterface.h"
#include "configParser.h"

#define SAMPLEPERIODUS 200

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

constexpr uint8_t LED_PIN = 13;
constexpr uint8_t LED_ENABLE = 14;
constexpr uint16_t NUM_LEDS = 5;

constexpr uint8_t MIC_WS_PIN = 25;
constexpr uint8_t MIC_SD_PIN = 32;
constexpr uint8_t MIC_SCK_PIN = 33;

WS2812 strip = WS2812(LED_PIN, NUM_LEDS);
// /INMP441 mic(MIC_WS_PIN, MIC_SD_PIN, MIC_SCK_PIN);

// BeatDetector detector;
// pattern_engine_config patternEngine_config;
// PatternEngine patternEngine(NUM_LEDS);
// JellEDConfigParser configParser;

long lastMillis = 0;
long loops = 0;

String toString(PatternType p) {
   if (p == PatternType::COLORED_AMPLITUDE) {
      return "COLORED_AMPLITUDE";
   } else {
      return "ALTERNATING_COLORS";
   }
}

void toString(pattern_color c) {
   Serial.println(c.red);
   Serial.println(c.green);
   Serial.println(c.blue);
}

void onBlePackageReceived(std::string &package) {
   Serial.println("On Package Received");
   // configParser.parse_to_config(package);

   // Serial.println("p1");
   // Serial.println(toString(configParser.get_pattern_engine_config().pattern1));
   // Serial.println("p2");
   // Serial.println(toString(configParser.get_pattern_engine_config().pattern2));
   // Serial.println("p3");
   // Serial.println(toString(configParser.get_pattern_engine_config().pattern3));
   // Serial.println("p4");
   // Serial.println(toString(configParser.get_pattern_engine_config().pattern4));
   // Serial.println("bpp");
   // Serial.println(configParser.get_pattern_engine_config().beats_per_pattern);

   // Serial.println("c1");
   // toString(configParser.get_pattern_config().palette_color1);
   // Serial.println("c2");
   // toString(configParser.get_pattern_config().palette_color2);
   // Serial.println("c3");
   // toString(configParser.get_pattern_config().palette_color3);
}

BluetoothInterface bli(onBlePackageReceived);
 
void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   // sbi(ADCSRA, ADPS2);
   //  cbi(ADCSRA, ADPS1);
   //  cbi(ADCSRA, ADPS0);

   Serial.println("ready");
   pinMode(2, OUTPUT);
   pinMode(LED_ENABLE, OUTPUT);
   pinMode(LED_PIN, OUTPUT);
   strip.initialize();
   strip.setBrightness(64);
   digitalWrite(LED_ENABLE, LOW);

   // mic.initialize();

   // patternEngine_config.beats_per_pattern = 4;
   // patternEngine_config.pattern1 = PatternType::ALTERNATING_COLORS;
   // patternEngine_config.pattern2 = PatternType::ALTERNATING_COLORS;
   // patternEngine_config.pattern3 = PatternType::ALTERNATING_COLORS;
   // patternEngine_config.pattern4 = PatternType::ALTERNATING_COLORS;
   // patternEngine.start(patternEngine_config);

   bli.initialize();
}

// 20 - 200hz Single Pole Bandpass IIR Filter
float bassFilter(float sample) {
    static float xv[3] = {0, 0, 0}, yv[3] = {0, 0, 0};
    xv[0] = xv[1];
    xv[1] = xv[2];
    xv[2] = (sample) /
            3.f; // change here to values close to 2, to adapt for stronger or weeker sources of line level audio

    yv[0] = yv[1];
    yv[1] = yv[2];
    yv[2] = (xv[2] - xv[0])
            + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
    return yv[2];
}

// 10hz Single Pole Lowpass IIR Filter
float envelopeFilter(float sample) { //10hz low pass
    static float xv[2] = {0, 0}, yv[2] = {0, 0};
    xv[0] = xv[1];
    xv[1] = sample / 50.f;
    yv[0] = yv[1];
    yv[1] = (xv[0] + xv[1]) + (0.9875119299f * yv[0]);
    return yv[1];
}

// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
float beatFilter(float sample) {
    static float xv[3] = {0, 0, 0}, yv[3] = {0, 0, 0};
    xv[0] = xv[1];
    xv[1] = xv[2];
    xv[2] = sample / 2.7f;
    yv[0] = yv[1];
    yv[1] = yv[2];
    yv[2] = (xv[2] - xv[0])
            + (-0.7169861741f * yv[0]) + (1.4453653501f * yv[1]);
    return yv[2];
}

AudioBuffer audio_buffer;
bool is_beat = false;
int looper = 0;

void loop() {
    for (int i = 0; i < 5; ++i) {
      strip.setColorChannelsFor(i, 255, 0, 0);
    }
    strip.show();

    delay(1000);

    for (int i = 0; i < 5; ++i) {
      strip.setColorChannelsFor(i, 0, 255, 0);
    }
    strip.show();

    delay(1000);
}

/**
void loop() {

   // float mean = .0f;
   // is_beat = (looper++ % 10) == 0;
   // const Pattern& pattern = patternEngine.generate_pattern(is_beat);
   // for (int pattern_index = 0; pattern_index < pattern.get_length(); ++pattern_index) {
   //    const pattern_color &color = pattern.get_color(pattern_index);
      // Serial.print("Red ");
      // Serial.print(color.red);
      // Serial.print(" Green: ");
      // Serial.print(color.green);
      // Serial.print(" Blue: ");
      // Serial.println(color.blue);
   //    strip.setColorChannelsFor(pattern_index, color.red, color.green, color.blue);
   // }
   // strip.show();

   bool buffer_ready = mic.read(&audio_buffer);
   if (buffer_ready) {
      for (size_t sample_index = 0; sample_index < audio_buffer.num_samples; ++sample_index) {
         int16_t audio_value = audio_buffer.buffer[sample_index];
      //    mean += audio_value;
      // mean /= audio_buffer.num_samples;
      // Serial.println(mean);
         is_beat = detector.is_beat(audio_value * 4);
         if (is_beat) {
            Serial.println("is_beat");
         }
         const Pattern& pattern = patternEngine.generate_pattern(is_beat);
         for (int pattern_index = 0; pattern_index < pattern.get_length(); ++pattern_index) {
            const pattern_color &color = pattern.get_color(pattern_index);
            // Serial.print("Red ");
            // Serial.print(color.red);
            // Serial.print(" Green: ");
            // Serial.print(color.green);
            // Serial.print(" Blue: ");
            // Serial.println(color.blue);
            strip.setColorChannelsFor(pattern_index, color.red, color.green, color.blue);
         }
      }
      strip.show();
   }
   
}**/
