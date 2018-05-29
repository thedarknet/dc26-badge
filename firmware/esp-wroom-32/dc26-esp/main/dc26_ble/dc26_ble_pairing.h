#ifndef DC26_BLUETOOTH_PAIRING
#define DC26_BLUETOOTH_PAIRING

#include "esp_system.h"
#include "esp_log.h"
#include "dc26_ble.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"

#define PAIR_SERVICE_UUID   "64633236-6463-646e-3230-313870616972"
#define PAIRING_CHAR_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define PAIRING_DESC_UUID   "b000b135-0001-0002-0003-000000000004"
static BLEUUID pairServiceUUID(PAIR_SERVICE_UUID);
static BLEUUID pairingCharUUID(PAIRING_CHAR_UUID);
static BLEUUID pairingDescUUID(PAIRING_DESC_UUID);

class BluetoothTask;
class PairingServerTask;
class PairingClientTask;
class MyServerCallbacks;
class MyClientCallbacks;

class MyServerCallbacks : public BLEServerCallbacks {
public:
	BluetoothTask *btTask;
	PairingServerTask *pPairingServerTask;
	MyClientCallbacks *pClientCallbacks;
	bool connected = false;
public:
	void onConnect(BLEServer* server);
	void onDisconnect(BLEServer* server);
public:
protected:
};

class MyClientCallbacks : public BLEClientCallbacks {
public:
	BluetoothTask *btTask;
	PairingClientTask *pPairingClientTask;
	MyServerCallbacks *pServerCallbacks;
	BLERemoteCharacteristic *pRemoteCharacteristic;
	bool connected = false;
public:
	void onConnect(BLEClient *pClient);
	void afterConnected(BLEClient *pClient);
	void onDisconnect(BLEClient *pClient);
};


class PairingServerTask : public Task {
public:
	BLEAddress *pClientAddress = nullptr;
	BLEAddress *pServerAddress = nullptr;
	BLECharacteristic *pPairingCharacteristic = nullptr;
	MyServerCallbacks *pServerCallbacks = nullptr;
public:
	PairingServerTask(const std::string &tName, uint16_t stackSize=10000, uint8_t p=5);
	bool init();
	virtual void run(void *data);
	virtual ~PairingServerTask();
};

class PairingClientTask : public Task {
public:
	BLEAddress *pClientAddress = nullptr;
	BLEAddress *pServerAddress = nullptr;
	BLERemoteCharacteristic *pRemoteCharacteristic;
	MyClientCallbacks *pClientCallbacks = nullptr;
public:
	PairingClientTask(const std::string &tName, uint16_t stackSize=10000, uint8_t p=5);
	bool init();
	virtual void run(void *data);
	virtual ~PairingClientTask();
};

#endif // DC26_BLUETOOTH_PAIRING_SERVER
