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
	BluetoothTask *pBTTask = nullptr;
	QueueHandle_t CallbackQueueHandle = nullptr;
	char mesgbuf2[200];
	unsigned int midx2 = 0;
public:
	void onWrite(BLECharacteristic *pCharacteristic);
protected:
};

class UartServerCallbacks : public BLEServerCallbacks {
public:
	BluetoothTask *pBTTask = nullptr;
	bool isConnected = false;
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
	bool isConnected = false;
	bool setup = false;
public:
	void onConnect(BLEClient* client);
	void afterConnect();
	void onDisconnect(BLEClient* client);
protected:
};


#endif // DC26_CONNECTING
