#ifndef _BLUETOOTH_INTERFACE_H_
#define _BLUETOOTH_INTERFACE_H_

#include <BLEUtils.h>
#include <BLEServer.h>

typedef struct t_bluetooth_package {
    bool isOn;
} t_bluetooth_package;

class DeviceStateCallback;

class BluetoothInterface {
private:
    BLEServer *bleServer;
    BLEService *bleService;
    BLECharacteristic *deviceStateCharacteristic;
    BLEAdvertising *bleAdvertising;
    void (*on_package_received) (t_bluetooth_package*);

public:
    BluetoothInterface(void (*on_package_received) (t_bluetooth_package*));

    void initialize();

    // friend class DeviceStateCallback;
};

#endif
