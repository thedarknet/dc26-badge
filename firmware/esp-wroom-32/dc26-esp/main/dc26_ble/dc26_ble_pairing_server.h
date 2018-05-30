#ifndef DC26_BLUETOOTH_PAIRING
#define DC26_BLUETOOTH_PAIRING

#include "esp_system.h"
#include "esp_log.h"
#include "dc26_ble.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"

#define PAIR_SERVICE_UUID   "64633236-6463-646e-3230-313870616972"
#define PAIRING_WRITE_UUID  "60000001-0001-0002-0003-000000000004"
#define PAIRING_READ_UUID   "60000002-0001-0002-0003-000000000004"

static BLEUUID uartServiceUUID(PAIR_SERVICE_UUID);
static BLEUUID uartWriteUUID(PAIRING_WRITE_UUID);
static BLEUUID uartReadUUID(PAIRING_READ_UUID);

class UartServerTask;

class UartWriteCharCallbacks : public BLECharacteristicCallbacks {
public:
	void onWrite(BLECharacteristic *pCharacteristic);
protected:
};

class UartServerCallbacks : public BLEServerCallbacks {
public:
	BLECharacteristic *pCharacteristic;
	UartServerTask *pServerTask;
public:
	void onConnect(BLEServer* server);
	void onDisconnect(BLEServer* server);
protected:
};


class UartServerTask : public Task {
public:
	// TODO: Message Queue
public:
	UartServerTask(const std::string &tName, uint16_t stackSize=10000, uint8_t p=3);
	bool init();
public:
	virtual void run(void *data);
	virtual ~UartServerTask();
protected:
	// TODO: Message Queue
};


#endif // DC26_BLUETOOTH_PAIRING_SERVER
