#include "BluetoothInterface.h"
#include <Arduino.h>

constexpr uint8_t LED_PIN = 13;
constexpr uint8_t LED_ENABLE = 14;
constexpr uint16_t NUM_LEDS = 2;

constexpr uint8_t MIC_WS_PIN = 25;
constexpr uint8_t MIC_SD_PIN = 32;
constexpr uint8_t MIC_SCK_PIN = 33;

void onBlePackageReceived(t_bluetooth_package *package) {
   Serial.println("On Package Received");
   Serial.println(package->isOn);
}

BluetoothInterface bli(onBlePackageReceived);
 
void setup() {
   Serial.begin(115200);
   Serial.println(" ");

   Serial.println("ready");
   bli.initialize();
}
 
void loop() {
   delay(200);
}
