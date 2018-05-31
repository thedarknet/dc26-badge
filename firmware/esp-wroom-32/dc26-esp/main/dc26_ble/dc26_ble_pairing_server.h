#ifndef DC26_BLUETOOTH_PAIRING
#define DC26_BLUETOOTH_PAIRING

#include "esp_system.h"
#include "esp_log.h"
#include "dc26_ble.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"

#define PAIR_SERVICE_UUID   "64633236-6463-646e-3230-313870616972"
#define PAIRING_TX_UUID  "60000001-0001-0002-0003-000000000004"
#define PAIRING_RX_UUID   "60000002-0001-0002-0003-000000000004"

static BLEUUID uartServiceUUID(PAIR_SERVICE_UUID);
static BLEUUID uartTxUUID(PAIRING_TX_UUID);
static BLEUUID uartRxUUID(PAIRING_RX_UUID);

class UartRxCharCallbacks : public BLECharacteristicCallbacks {
public:
	void onWrite(BLECharacteristic *pCharacteristic);
protected:
};

class UartServerCallbacks : public BLEServerCallbacks {
public:
	bool isConnected;
public:
	void onConnect(BLEServer* server);
	void onDisconnect(BLEServer* server);
protected:
};

#endif // DC26_BLUETOOTH_PAIRING_SERVER
