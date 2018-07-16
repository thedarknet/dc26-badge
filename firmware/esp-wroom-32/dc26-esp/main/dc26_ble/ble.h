#ifndef DC26_BLUETOOTH
#define DC26_BLUETOOTH

#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../lib/Task.h"
#include "../lib/ble/BLEDevice.h"
#include "../mcu_to_mcu.h"
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
	std::string adv_manufacturer = "DN\0\0\0\0"; //DN - Infections - Cures

	// Security stuff -- FIXME: remove actingClient/Server
	bool isActingClient = false;
	bool isActingServer = false;

	// Callback message queue
	static const int CBACK_MSG_QUEUE_SIZE = 10;
	static const int CBACK_MSG_ITEM_SIZE = sizeof(void *); // TODO: Msg Size?
	StaticQueue_t CallbackQueue;
	QueueHandle_t CallbackQueueHandle = nullptr;
	uint8_t CallbackBuffer[CBACK_MSG_QUEUE_SIZE * CBACK_MSG_ITEM_SIZE];

	static const int STM_MSG_QUEUE_SIZE = 5;
	static const int STM_MSG_ITEM_SIZE = sizeof(MCUToMCUTask::Message *);
	StaticQueue_t STMQueue;
	QueueHandle_t STMQueueHandle = nullptr;
	uint8_t fromSTMBuffer[STM_MSG_QUEUE_SIZE*STM_MSG_ITEM_SIZE];
	

public: // API
	void startAdvertising(void);
	void stopAdvertising(void);
	void toggleAdvertising(const darknet7::STMToESPRequest* m);
	void getDeviceName(void);
	void setDeviceName(const darknet7::STMToESPRequest* m);
	void getInfectionData();
	void setInfectionData(const darknet7::STMToESPRequest* m);
	void getCureData();
	void setCureData(const darknet7::STMToESPRequest* m);
	void scanForDevices(const darknet7::STMToESPRequest* m);
	void pairWithDevice(const darknet7::STMToESPRequest* m);
	void sendPINConfirmation(const darknet7::STMToESPRequest* m);
	void getConnectedDevices();
	void sendDataToDevice(const darknet7::STMToESPRequest* m);
	void disconnectFromDevice(const darknet7::STMToESPRequest* m);
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
