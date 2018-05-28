#ifndef DC26_BLUETOOTH
#define DC26_BLUETOOTH

#include "esp_system.h"
#include "esp_log.h"
#include "lib/Task.h"
#include "lib/ble/BLEDevice.h"

enum BTCmd
{
	BT_CMD_START_B2B,
	BT_CMD_STOP_B2B,
	BT_CMD_SET_B2B_ADV_DATA,
	BT_CMD_PASSIVE_SCAN,
	BT_CMD_ACTIVE_SCAN,
	BT_CMD_UNK,
};

class MyScanCallbacks : public BLEAdvertisedDeviceCallbacks {
public:
	void onResult(BLEAdvertisedDevice advertisedDevice);
public:
public:
protected:
};


class BluetoothTask : public Task {
public:
	static const int ESP_TO_BT_MSG_QUEUE_SIZE = 10;
	//static const int ESP_TO_BT_MSG_ITEM_SIZE = sizeof(darknet7::ESPToBTRequest*);
	static const char *LOGTAG;

	BLEDevice *pDevice;
	BLEServer *pServer;
	BLEScan *pScan;
	BLEAdvertising *pAdvertising;
	MyScanCallbacks *pScanCallbacks;
	BLEAdvertisementData adv_data;

	// Advertisement data
	bool b2b_advertising_enabled = false;
	std::string adv_name = "DN 12345";
	std::string adv_manufacturer = "DNEGGPLANT";

	// Scanning related data
	uint32_t scan_time = 30;
	bool scan_started = false;
	bool cancel_scan = false;

public:
	BluetoothTask(const std::string &tName, uint16_t stackSize=10000, uint8_t p=5);
	bool init();

	// DC26 Badge-life protocol advertising
	void setB2BAdvData(std::string new_name, std::string new_man_data);
	void startB2BAdvertising();
	void stopB2BAdvertising();

	// Scanning for advertising device
	void scan(bool active);

	// Serial badge-to-??? comms
public:
	virtual void run(void *data);
	virtual ~BluetoothTask();
protected:
	StaticQueue_t InQueue;
	QueueHandle_t InQueueHandle = nullptr;
	//uint6_t ESPToBTBuffer[ESP_TO_BT_MSG_QUEUE_SIZE*ESP_TO_BT_MSG_ITEM_SIZE]
};


#endif // DC26_BLUETOOTH
