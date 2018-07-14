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
#include "security.h"

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
	BLESecurity *pSecurity;
	MySecurity *pMySecurity;
	BLEAdvertising *pAdvertising;
	MyScanCallbacks *pScanCallbacks;
	BLECharacteristic *pUartCisoCharacteristic;
	BLECharacteristic *pUartCosiCharacteristic;
	BLEAdvertisementData adv_data;

	// Advertisement data
	bool advertising_enabled = false;
	std::string adv_name = "DN1";
	std::string adv_manufacturer = "DN2";

	// Callback message queue
	static const int CBACK_MSG_QUEUE_SIZE = 10;
	static const int CBACK_MSG_ITEM_SIZE = sizeof(void *); // TODO: Msg Size?
	StaticQueue_t CallbackQueue;
	QueueHandle_t CallbackQueueHandle = nullptr;
	uint8_t CallbackBuffer[CBACK_MSG_QUEUE_SIZE * CBACK_MSG_ITEM_SIZE];

public: // API
	void startAdvertising(void);
	void stopAdvertising(void);
	void getDeviceName(void);
	void setDeviceName(darknet7::STMToESPRequest* msg);
	void getInfectionData();
	void setInfectionData(darknet7::STMToESPRequest* msg);
	void getCureData();
	void setCureData(darknet7::STMToESPRequest* msg);
	void scanForDevices(darknet7::STMToESPRequest* msg);
	void pairWithDevice(darknet7::STMToESPRequest* msg);
	void sendPINConfirmation(darknet7::STMToESPRequest* msg);
	void getConnectedDevices();
	void sendDataToDevice(darknet7::STMToESPRequest* msg);
	void disconnectFromDevice(darknet7::STMToESPRequest* msg);
	void disconnectFromAll();

public:
	BluetoothTask(const std::string &tName, uint16_t stackSize=10000, uint8_t p=5);
	bool init();
	void commandHandler(MCUToMCUTask::Message* cmd);
	virtual void run(void *data);
	virtual ~BluetoothTask();
protected:
	void refreshAdvertisementData(void);
};


#endif // DC26_BLUETOOTH
