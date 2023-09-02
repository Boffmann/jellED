#include "BluetoothInterface.h"

#include<Arduino.h>

#include <BLEDevice.h>

#define DEVICE_NAME "jellED"
#define SERVICE_UUID "665aa768-49c8-11ee-be56-0242ac120002"
#define CHARACTERISTIC_UUID "d0756476-49c8-11ee-be56-0242ac120002"

class DeviceStateCallback: public BLECharacteristicCallbacks {
private:
    BluetoothInterface *bluetoothInterface;
public:

    DeviceStateCallback(BluetoothInterface *bluetoothInterface) {
        this->bluetoothInterface = bluetoothInterface;
    }

    void onWrite(BLECharacteristic * characteristic) {
        t_bluetooth_package package;
        std::string readValue = characteristic->getValue();
        if (readValue.length() > 0) {
            bool isOn = readValue[0] == 1;
            package.isOn = isOn;
            this->bluetoothInterface->on_package_received(&package);
        }
    }
};

BluetoothInterface::BluetoothInterface(void (*on_package_received) (t_bluetooth_package*)) {
    this->on_package_received = on_package_received;
}

void BluetoothInterface::initialize() {
    BLEDevice::init(DEVICE_NAME);
    this->bleServer = BLEDevice::createServer();
    this->bleService = bleServer->createService(SERVICE_UUID);

    this->deviceStateCharacteristic = 
        bleService->createCharacteristic(CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_WRITE);
    deviceStateCharacteristic->setCallbacks(new DeviceStateCallback(this));

    bleService->start();
    this->bleAdvertising = BLEDevice::getAdvertising();
    this->bleAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();
}