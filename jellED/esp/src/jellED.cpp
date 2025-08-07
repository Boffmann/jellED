#include <Arduino.h>
#include <algorithm>

#include "EspUart.h"

constexpr uint8_t LED_PIN = 13;
constexpr uint8_t LED_ENABLE = 14;
constexpr uint16_t NUM_LEDS = 2;

constexpr uint8_t MIC_WS_PIN = 25;
constexpr uint8_t MIC_SD_PIN = 32;
constexpr uint8_t MIC_SCK_PIN = 33;

constexpr uint8_t SPEAKER_OUT_PIN = 26;

constexpr uint8_t ESP_UART_TX_PIN = 17;
constexpr uint8_t ESP_UART_RX_PIN = 16;

constexpr uint32_t ESP_UART_BAUD_RATE = 115200;
constexpr bool MODE_SEND = true;

long lastMillis = 0;
long loops = 0;

jellED::SerialConfig espSerialConfig;
jellED::EspUart espUart("Uart Receiver", UART_NUM_2, ESP_UART_TX_PIN, ESP_UART_RX_PIN);

void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   Serial.println("ready");
   Serial.println(esp_get_idf_version());

   if (!espUart.initialize(espSerialConfig, ESP_UART_BAUD_RATE)) {
      Serial.println("Error initializing UART");
   } else {
      Serial.println("Successfully initialized UART");
   }
   espUart.flush();
}

static uint8_t dataToWrite = 0;
void uart_send_int() {
   dataToWrite = dataToWrite + 1;
	if (dataToWrite > 255) {
		dataToWrite = 0;
	}
   if (espUart.send(&dataToWrite, 1) == -1) {
      Serial.println("Failed to write to uart");
   } else {
      Serial.println("Written to uart: " + (String)unsigned(dataToWrite));
   }
   delay(5000);
}

std::string helloToSend = "Hello";
void uart_send_string() {
   if (espUart.send(helloToSend) == -1) {
      Serial.println("Failed to write to uart");
   } else {
      Serial.println("Written to uart: " + (String)helloToSend.c_str());
   }
   delay(5000);
}

int maxLength = 8;
uint8_t receivedBuffer;
void uart_read_int() {
   int available = espUart.available();
   if (available > 0) {
      Serial.println("Available: " + (String)available);
      int received = espUart.receive(&receivedBuffer, std::min(available, maxLength));
       if (received > 0) {
            Serial.println("Received: " + (String)receivedBuffer + " (" + received + " bytes)");
        } else if (received == 0) {
            Serial.println("No data available");
        } else {
            Serial.println("Error receiving message");
        }
      espUart.flush();
   }
}

std::string receivedBuffer_string;
void uart_read_string() {
   int available = espUart.available();
   if (available > 0) {
      Serial.println("Available: " + (String)available);
      int received = espUart.receive(receivedBuffer_string, std::min(available, maxLength));
       if (received > 0) {
            Serial.println("Received: " + (String)receivedBuffer_string.c_str() + " (" + received + " bytes)");
        } else if (received == 0) {
            Serial.println("No data available");
        } else {
            Serial.println("Error receiving message");
        }
      espUart.flush();
   }
}

void loop() {
   if (MODE_SEND) {
		uart_send_string();
   } else {
		uart_read_string();
   }
}