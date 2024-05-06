#include <Arduino.h>
#include "musicPiece.h"
#include "soundconfig.h"
#include "speaker.h"
#include "INMP441.h"
#include "beatdetection.h"
#include "patternEngine.h"
#include "BluetoothInterface.h"
#include "config/configParser.h"

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
JellEDConfigParser configParser;

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
   configParser.parse_to_config(package);

   Serial.println("p1");
   Serial.println(toString(configParser.get_pattern_engine_config().pattern1));
   Serial.println("p2");
   Serial.println(toString(configParser.get_pattern_engine_config().pattern2));
   Serial.println("p3");
   Serial.println(toString(configParser.get_pattern_engine_config().pattern3));
   Serial.println("p4");
   Serial.println(toString(configParser.get_pattern_engine_config().pattern4));
   Serial.println("bpp");
   Serial.println(configParser.get_pattern_engine_config().beats_per_pattern);

   Serial.println("c1");
   toString(configParser.get_pattern_config().palette_color1);
   Serial.println("c2");
   toString(configParser.get_pattern_config().palette_color2);
   Serial.println("c3");
   toString(configParser.get_pattern_config().palette_color3);
}

BluetoothInterface bli(onBlePackageReceived);
 
void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   Serial.println("ready");
   pinMode(2, OUTPUT);
   //piece.initialize();
   //mic.initialize();
   //speaker.initialize();
   //patternEngine.start(PatternType::COLORED_AMPLITUDE);
   bli.initialize();
}

uint8_t convert8Bit(int16_t in) {
   float intermediate = 32768.0f + (float) in;
   return (uint8_t) (intermediate * 0.00389105058);
}

uint16_t convertUnsigned(int16_t in) {
   return (uint16_t) 32768 + in;
}

void loop() {
   delay(200);
}
