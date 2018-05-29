#ifndef DC26_BLUETOOTH_SCANNING
#define DC26_BLUETOOTH_SCANNING

#include "esp_system.h"
#include "esp_log.h"
#include "dc26_ble.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"

class MyScanCallbacks : public BLEAdvertisedDeviceCallbacks {
public:
	BLEAddress *pServerAddress;
	bool server_found = false;
public:
	void onResult(BLEAdvertisedDevice advertisedDevice);
public:
protected:
};

#endif // DC26_BLUETOOTH_SCANNING
