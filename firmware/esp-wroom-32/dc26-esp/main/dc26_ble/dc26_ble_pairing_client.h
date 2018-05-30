#ifndef DC26_BLUETOOTH_PAIRING_CLIENT
#define DC26_BLUETOOTH_PAIRING_CLIENT

#include "esp_system.h"
#include "esp_log.h"
#include "dc26_ble.h"
#include "dc26_ble_pairing_server.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"



class UartClientTask : public Task {
public:
	// TODO: Message Queue
public:
	UartClientTask(const std::string &tName, uint16_t stackSize=10000, uint8_t p=3);
	bool init();
public:
	virtual void run(void *data);
	virtual ~UartClientTask();
protected:
	// TODO: Message Queue
};


class UartClientCallbacks : public BLEClientCallbacks {
public:
	UartClientTask *pClientTask;
public:
	void onConnect(BLEClient* client);
	void onDisconnect(BLEClient* client);
protected:
};


#endif // DC26_BLUETOOTH_PAIRING_CLIENT
