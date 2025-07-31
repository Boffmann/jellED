#ifndef _BLUETOOTH_INTERFACE_H_
#define _BLUETOOTH_INTERFACE_H_

#include <BLEUtils.h>
#include <BLEServer.h>

namespace jellED {

class DeviceStateCallback;

class BluetoothInterface {
private:
    BLEServer *bleServer;
    BLEService *bleService;
    BLECharacteristic *deviceStateCharacteristic;
    BLEAdvertising *bleAdvertising;
    void (*on_package_received) (std::string&);

public:
    BluetoothInterface(void (*on_package_received) (std::string&));

    void initialize();

    friend class DeviceStateCallback;
};

} // namespace jellED

#endif
