#include <Arduino.h>
#include <algorithm>

#include "WS2812.h"
#include "esputils.h"
#include "patternEngine.h"
#include "EspUart.h"
#include "uartProtocol.h"

constexpr uint16_t NUM_LEDS = 10;
constexpr unsigned long PATTERN_DURATION_MICROS = 5000000;
// az-delivery-devkit-v4
// constexpr uint8_t LED_PIN = 13;
// constexpr uint8_t ESP_UART_TX_PIN = 17;
// constexpr uint8_t ESP_UART_RX_PIN = 16;
// esp32-c3-devkitm-1
constexpr uint8_t LED_PIN = 2;
constexpr uint8_t ESP_UART_TX_PIN = 21;
constexpr uint8_t ESP_UART_RX_PIN = 20;
constexpr uint32_t ESP_UART_BAUD_RATE = 115200;

jellED::WS2812 strip = jellED::WS2812(LED_PIN, NUM_LEDS);
jellED::EspPlatformUtils espUtils;
jellED::PatternEngine patternEngine =
    jellED::PatternEngine(espUtils, NUM_LEDS, PATTERN_DURATION_MICROS);

jellED::SerialConfig espSerialConfig;
jellED::EspUart espUart("Uart Receiver", UART_NUM_0, ESP_UART_TX_PIN, ESP_UART_RX_PIN);

void setup() {
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
      int received = espUart.receive(&receivedBuffer, std::min(available, maxLength));
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
  // Removed delay(10) - was adding up to 10ms latency to beat detection
  // The strip.show() already takes time for WS2812 protocol (~30µs per LED)
  // If CPU usage is too high, use delay(1) instead
  delay(1);
}
