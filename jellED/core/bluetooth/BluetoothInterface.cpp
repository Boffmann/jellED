#include "BluetoothInterface.h"

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLE2902.h>

#define DEVICE_NAME "jellED"
#define CHARACTERISTIC_NAME "jellED-Write"
#define SERVICE_UUID "665aa768-49c8-11ee-be56-0242ac120002"
#define CHARACTERISTIC_UUID "d0756476-49c8-11ee-be56-0242ac120002"

namespace jellED {

class BleServerCallbacks: public BLEServerCallbacks {

    void onConnect(BLEServer* pServer) {
      Serial.println("Connected to client");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnected to client");
      pServer->startAdvertising();
    };
};

class DeviceStateCallback: public BLECharacteristicCallbacks {
private:
    BluetoothInterface *bluetoothInterface;
public:
    DeviceStateCallback(BluetoothInterface *bluetoothInterface) {
        this->bluetoothInterface = bluetoothInterface;
    }

	void onWrite(BLECharacteristic* characteristic, esp_ble_gatts_cb_param_t* param) {
        Serial.println("Value Write Value");
        std::string readValue = characteristic->getValue();
        // Serial.println(readValue.c_str());
        if (readValue.length() > 0) {
            this->bluetoothInterface->on_package_received(readValue);
        }
    }

    void onWrite(BLECharacteristic * characteristic) {
        Serial.println("Write Value");
        // t_bluetooth_package package;
        // std::string readValue = characteristic->getValue();
        // Serial.println(readValue.c_str());
        // if (readValue.length() > 0) {
        //     bool isOn = readValue[0] == 1;
        //     package.isOn = isOn;
        //     this->bluetoothInterface->on_package_received(&package);
        // }
    }

    
	void onNotify(BLECharacteristic* pCharacteristic) {
        Serial.println("Value Notify Value");
    }
};

BluetoothInterface::BluetoothInterface(void (*on_package_received) (std::string&)) {
    this->on_package_received = on_package_received;
}

void BluetoothInterface::initialize() {
    BLEDevice::init(DEVICE_NAME);
    this->bleServer = BLEDevice::createServer();
    this->bleServer->setCallbacks(new BleServerCallbacks());
    this->bleService = bleServer->createService(SERVICE_UUID);

    this->deviceStateCharacteristic = 
        bleService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);
    deviceStateCharacteristic->addDescriptor(new BLE2902());
    deviceStateCharacteristic->setWriteProperty(true);
    deviceStateCharacteristic->setCallbacks(new DeviceStateCallback(this));

    bleService->start();
    this->bleAdvertising = BLEDevice::getAdvertising();
    this->bleAdvertising->addServiceUUID(SERVICE_UUID);
    this->bleAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
    deviceStateCharacteristic->setValue("Moin");
}

} // namespace jellED
