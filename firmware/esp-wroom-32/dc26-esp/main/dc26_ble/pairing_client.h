#ifndef DC26_BLUETOOTH_PAIRING_CLIENT
#define DC26_BLUETOOTH_PAIRING_CLIENT

#include "esp_system.h"
#include "esp_log.h"
#include "ble.h"
#include "pairing_server.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"

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


#endif // DC26_BLUETOOTH_PAIRING_CLIENT
