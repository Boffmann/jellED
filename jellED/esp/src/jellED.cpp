#include <Arduino.h>
#include <algorithm>

#include "WS2812.h"
#include "esputils.h"
#include "patternEngine.h"
#include "EspUart.h"
#include "uartProtocol.h"

constexpr uint16_t NUM_LEDS              = 10;
constexpr unsigned long PATTERN_DURATION_MICROS = 5000000;
// az-delivery-devkit-v4
// constexpr uint8_t LED_PIN = 13;
// constexpr uint8_t ESP_UART_TX_PIN = 17;
// constexpr uint8_t ESP_UART_RX_PIN = 16;
// esp32-c3-devkitm-1
constexpr uint8_t LED_PIN           = 2;
constexpr uint8_t ESP_UART_TX_PIN   = 21;
constexpr uint8_t ESP_UART_RX_PIN   = 20;
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

// Attempt to read one complete 7-byte packet from the UART buffer.
// Returns true and populates `out` on success; returns false if no valid
// packet is available yet (not enough bytes, bad header, or bad checksum).
static uint8_t rxBuf[jellED::UART_PACKET_SIZE];

bool uart_read_packet(jellED::UartFeatures& out) {
  if (espUart.available() < jellED::UART_PACKET_SIZE) return false;

  int got = espUart.receive(rxBuf, jellED::UART_PACKET_SIZE);
  espUart.flush(); // discard any bytes beyond one packet

  if (got != jellED::UART_PACKET_SIZE) return false;
  return jellED::uart_parse_packet(rxBuf, out);
}

// Persistent audio features — volume data outlives a single loop iteration so
// continuous patterns (breathing glow, etc.) always have current values.
static jellED::AudioFeatures persistentFeatures{};

void loop() {
  jellED::UartFeatures received{};
  if (uart_read_packet(received)) {
    // Full packet received: update all fields including beat flags.
    persistentFeatures.volumeLow    = received.volumeLow;
    persistentFeatures.volumeMid    = received.volumeMid;
    persistentFeatures.volumeHigh   = received.volumeHigh;
    persistentFeatures.beatFlags    = received.beatFlags;
    persistentFeatures.spectralTilt = received.spectralTilt;
  } else {
    // No new packet this frame: keep volume data but clear beat flags.
    // Beat is a momentary event — it should only fire for the one frame
    // in which the packet arrived.
    persistentFeatures.beatFlags = 0;
  }

  const jellED::Pattern& pattern = patternEngine.generate_pattern(persistentFeatures);
  for (int i = 0; i < pattern.get_length(); ++i) {
    const jellED::pattern_color& color = pattern.get_color(i);
    strip.setColorChannelsRGBAFor(i, color.red, color.green, color.blue, color.brightness);
  }
  strip.show();
  delay(1);
}
