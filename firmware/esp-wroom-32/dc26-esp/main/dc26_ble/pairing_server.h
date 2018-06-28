#ifndef DC26_BLUETOOTH_PAIRING
#define DC26_BLUETOOTH_PAIRING

#include "esp_system.h"
#include "esp_log.h"
#include "ble.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"

class BluetoothTask;

class UartRxCharCallbacks : public BLECharacteristicCallbacks {
public:
	QueueHandle_t CallbackQueueHandle = nullptr;
public:
	void onWrite(BLECharacteristic *pCharacteristic);
protected:
};

class UartServerCallbacks : public BLEServerCallbacks {
public:
	BluetoothTask *pBTTask = nullptr;
	bool isConnected;
public:
	void onConnect(BLEServer* server);
	void onDisconnect(BLEServer* server);
protected:
};

#endif // DC26_BLUETOOTH_PAIRING_SERVER
