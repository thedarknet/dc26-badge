#ifndef DC26_BLUETOOTH
#define DC26_BLUETOOTH

#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"
#include "pairing_server.h"
#include "pairing_client.h"
#include "scanning.h"

enum BTCmd
{
	BT_CMD_START_B2B,
	BT_CMD_STOP_B2B,
	BT_CMD_SET_B2B_ADV_DATA,
	BT_CMD_PASSIVE_SCAN,
	BT_CMD_ACTIVE_SCAN,
	BT_CMD_PAIR,
	BT_CMD_ECHO,
	BT_CMD_UNK,
};

class BluetoothTask : public Task {
public:
	static const char *LOGTAG;

	BLEDevice *pDevice;
	BLEServer *pServer;
	BLEClient *pPairingClient;
	BLEService *pService;
	BLEScan *pScan;
	BLEAdvertising *pAdvertising;
	MyScanCallbacks *pScanCallbacks;
	BLECharacteristic *pUartTxCharacteristic;
	BLECharacteristic *pUartRxCharacteristic;
	BLEAdvertisementData adv_data;

	// Advertisement data
	bool b2b_advertising_enabled = false;
	std::string adv_name = "DN1";
	std::string adv_manufacturer = "DN2";

	// Scanning related data
	uint32_t scan_time = 30;
	bool scan_started = false;
	bool cancel_scan = false;

	// Client Connecting Data
	bool server_found;
	BLEAddress *pServerAddress;
	BLERemoteCharacteristic *pRemoteCharacteristic;

	// Callback message queue
	static const int CBACK_MSG_QUEUE_SIZE = 10;
	static const int CBACK_MSG_ITEM_SIZE = sizeof(void *); // TODO: Msg Size?
	StaticQueue_t CallbackQueue;
	QueueHandle_t CallbackQueueHandle = nullptr;
	uint8_t CallbackBuffer[CBACK_MSG_QUEUE_SIZE * CBACK_MSG_ITEM_SIZE];

public:
	BluetoothTask(const std::string &tName, uint16_t stackSize=10000, uint8_t p=5);
	bool init();

	// DC26 Badge-life protocol advertising
	void setB2BAdvData(std::string new_name, std::string new_man_data);
	void startB2BAdvertising();
	void stopB2BAdvertising();

	// Scanning for advertising device
	void scan(bool active);

	// Simple pairing demo
	void pair();
	void unpair();

	// Serial badge-to-??? comms
public:
	void dispatchCmd(BTCmd *cmd);
	virtual void run(void *data);
	virtual ~BluetoothTask();
protected:
};


#endif // DC26_BLUETOOTH
