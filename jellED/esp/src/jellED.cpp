#include <Arduino.h>
#include <algorithm>

#include "WS2812.h"
#include "esputils.h"
#include "patternEngine.h"
#include "EspUart.h"
#include "uartProtocol.h"

constexpr uint16_t NUM_LEDS              = 10;
constexpr unsigned long PATTERN_DURATION_MICROS = 5000000;
constexpr unsigned long BRIGHTNESS_DECAY_MICROS = 1000000;

// Pattern + LED update cadence. 100 Hz is above flicker fusion (~60–90 Hz) and
// matches the UART arrival rate from the Raspi, so we don't render the same
// audio features twice. UART reads still happen every main-loop tick (~1 kHz)
// so beats arriving between pattern updates are captured, not missed.
constexpr uint32_t PATTERN_INTERVAL_MS = 10;
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
jellED::PatternEngine patternEngine(espUtils, NUM_LEDS, PATTERN_DURATION_MICROS, BRIGHTNESS_DECAY_MICROS);

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

bool uart_read_latest(jellED::UartFeatures& out) {
  bool got_any = false;
  while (espUart.available() >= jellED::UART_PACKET_SIZE) {
    int got = espUart.receive(rxBuf, jellED::UART_PACKET_SIZE);
    if (got == jellED::UART_PACKET_SIZE && jellED::uart_parse_packet(rxBuf, out)) {
      got_any = true;
    }
  }
  return got_any;
}

// Persistent audio features — volume data outlives a single loop iteration so
// continuous patterns (breathing glow, etc.) always have current values.
static jellED::AudioFeatures persistentFeatures{};

void loop() {
  // Drain UART every loop tick (~1 kHz) so no packet sits in the buffer
  // longer than ~1 ms. Beat flags are OR-accumulated into persistentFeatures
  // across the whole PATTERN_INTERVAL_MS window — cleared only when a pattern
  // frame actually renders them. This guarantees a beat arriving between
  // pattern updates still shows up in the next frame.
  jellED::UartFeatures received{};
  if (uart_read_latest(received)) {
    persistentFeatures.volumeLow    = received.volumeLow;
    persistentFeatures.volumeMid    = received.volumeMid;
    persistentFeatures.volumeHigh   = received.volumeHigh;
    persistentFeatures.spectralTilt = received.spectralTilt;
    persistentFeatures.beatFlags   |= received.beatFlags;
  }

  // Throttle pattern generation + LED output to PATTERN_INTERVAL_MS.
  static uint32_t last_pattern_ms = 0;
  const uint32_t now = millis();
  if (now - last_pattern_ms >= PATTERN_INTERVAL_MS) {
    last_pattern_ms = now;

    const jellED::Pattern& pattern = patternEngine.generate_pattern(persistentFeatures);
    for (int i = 0; i < pattern.get_length(); ++i) {
      const jellED::pattern_color& color = pattern.get_color(i);
      strip.setColorChannelsRGBAFor(i, color.red, color.green, color.blue, color.brightness);
    }
    strip.show();

    // Beat flags are momentary — consumed by this frame, so clear them.
    persistentFeatures.beatFlags = 0;
  }

  delay(1);
}
