#ifndef DC26_BLUETOOTH_SERIAL
#define DC26_BLUETOOTH_SERIAL

#include "esp_system.h"
#include "esp_log.h"
#include "ble.h"
#include "../lib/ble/BLE2902.h"
#include "../lib/ble/BLEDevice.h"
#include "../lib/ble/BLECharacteristic.h"

class SerialCosiCharCallbacks : public BLECharacteristicCallbacks {
public:
public:
	void onWrite(BLECharacteristic *pCharacteristic);
protected:
};

void init_ble_serial(BLEService* pService);

#endif // DC26_BLUETOOTH_SERIAL
