#include <Arduino.h>
#include <algorithm>

#include "WS2812.h"
#include "esputils.h"
#include "patternEngine.h"
#include "EspUart.h"
#include "uartProtocol.h"

constexpr uint8_t LED_PIN = 13;
constexpr uint16_t NUM_LEDS = 5;
constexpr unsigned long PATTERN_DURATION_MICROS = 5000000;
constexpr uint8_t ESP_UART_TX_PIN = 17;
constexpr uint8_t ESP_UART_RX_PIN = 16;
constexpr uint32_t ESP_UART_BAUD_RATE = 115200;

jellED::WS2812 strip = jellED::WS2812(LED_PIN, NUM_LEDS);
jellED::EspPlatformUtils espUtils;
jellED::PatternEngine patternEngine =
    jellED::PatternEngine(espUtils, NUM_LEDS, PATTERN_DURATION_MICROS);

jellED::SerialConfig espSerialConfig;
jellED::EspUart espUart("Uart Receiver", UART_NUM_2, ESP_UART_TX_PIN, ESP_UART_RX_PIN);

void setup() {
  Serial.begin(115200);
   Serial.println(" ");
  strip.initialize();
  strip.setBrightness(255);
  espUart.initialize(espSerialConfig, ESP_UART_BAUD_RATE);
  espUart.flush();
}

int maxLength = 8;
uint8_t receivedBuffer;
uint8_t uart_read_int() {
   int available = espUart.available();
   if (available > 0) {
      // Serial.println("Available: " + (String)available);
      int received = espUart.receive(&receivedBuffer, std::min(available, maxLength));
       if (received > 0) {
            Serial.println("Received: " + (String)receivedBuffer + " (" + received + " bytes)");
        } else if (received == 0) {
            Serial.println("No data available");
        } else {
            Serial.println("Error receiving message");
        }
      espUart.flush();
      if (received > 0) {
        return receivedBuffer;
      }
   }
   return 0;
}

uint8_t uartReceived = 0;
bool is_beat = false;
void loop() {
  uartReceived = uart_read_int();
  if (uartReceived == jellED::UART_BEAT_DETECTED) {
    is_beat = true;
  }
  
  const jellED::Pattern &pattern = patternEngine.generate_pattern(is_beat);
  for (int pattern_index = 0; pattern_index < pattern.get_length();
       ++pattern_index) {
    const jellED::pattern_color &color = pattern.get_color(pattern_index);
    strip.setColorChannelsRGBAFor(pattern_index, color.red, color.green,
                                  color.blue, color.brightness);
  }
  strip.show();
  is_beat = false;
  delay(10);
}
