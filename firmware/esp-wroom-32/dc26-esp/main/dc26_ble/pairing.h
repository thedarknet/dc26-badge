#ifndef DC26_BLE_PAIRING
#define DC26_BLE_PAIRING

#include "esp_system.h"
#include "esp_log.h"
#include "ble.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"

class BluetoothTask;

class UartCosiCharCallbacks : public BLECharacteristicCallbacks {
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


class UartClientCallbacks : public BLEClientCallbacks {
public:
	BluetoothTask *pBTTask;
	BLEClient *pClient;
	BLERemoteService *pRemoteService;
	BLERemoteCharacteristic *pTxChar;
	BLERemoteCharacteristic *pRxChar;
	bool connected = false;
public:
	void onConnect(BLEClient* client);
	void afterConnect();
	void onDisconnect(BLEClient* client);
protected:
};


#endif // DC26_CONNECTING
