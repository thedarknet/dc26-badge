#ifndef DC26_BLUETOOTH_PAIRING_CLIENT
#define DC26_BLUETOOTH_PAIRING_CLIENT

#include "esp_system.h"
#include "esp_log.h"
#include "dc26_ble.h"
#include "dc26_ble_pairing_server.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"

class UartClientCallbacks : public BLEClientCallbacks {
public:
	BLEClient *pClient;
	BLERemoteService *pRemoteService;
	BLERemoteCharacteristic *pTxChar;
	BLERemoteCharacteristic *pRxChar;
public:
	void onConnect(BLEClient* client);
	void afterConnect();
	void onDisconnect(BLEClient* client);
protected:
};


#endif // DC26_BLUETOOTH_PAIRING_CLIENT
