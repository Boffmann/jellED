#include <Arduino.h>
#include <cmath>

#include "EspUart.h"

constexpr uint8_t LED_PIN = 13;
constexpr uint8_t LED_ENABLE = 14;
constexpr uint16_t NUM_LEDS = 2;

constexpr uint8_t MIC_WS_PIN = 25;
constexpr uint8_t MIC_SD_PIN = 32;
constexpr uint8_t MIC_SCK_PIN = 33;

constexpr uint8_t SPEAKER_OUT_PIN = 26;

constexpr uint8_t ESP_UART_TX_PIN = 17;


long lastMillis = 0;
long loops = 0;

size_t maxLength = 32;
std::string receivedBuffer;

jellED::UartConfig espUartConfig("Uart Receiver", UART_NUM_2);
jellED::EspUart espUart;

void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   Serial.println("ready");
   Serial.println(esp_get_idf_version());

   if (!espUart.initialize(espUartConfig)) {
      Serial.println("Error initializing UART");
   } else {
      Serial.println("Successfully initialized UART");
   }
}

void loop() {
   if (espUart.available()) {
      int received = espUart.receive(receivedBuffer, maxLength);
       if (received > 0) {
            Serial.println("Received: " + (String)receivedBuffer.c_str() + " (" + received + " bytes)");
        } else if (received == 0) {
            Serial.println("No data available");
        } else {
            Serial.println("Error receiving message");
        }
   }
}