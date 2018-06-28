#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Main BLE library files
#include "../lib/ble/BLE2902.h"
#include "../lib/ble/BLEDevice.h"

//DC26 BLE Files
#include "ble.h"
#include "services.h" // UUIDs for all potential services and characteristics
#include "pairing_server.h"
#include "pairing_client.h"
#include "scanning.h"
#include "./security.h"

#ifdef IS_CLIENT
bool isClient = true;
bool firstSend = true;
#else
bool isClient = false;
bool firstSend = false;
#endif // IS_CLIENT


const char *BluetoothTask::LOGTAG = "BluetoothTask";

// We need a global reference so that we can access the callback message queue
// from a static function
BluetoothTask *pBTTask;

//BLEDevice Device;
UartRxCharCallbacks UartRxCallbacks;
UartServerCallbacks iUartServerCallbacks;
UartClientCallbacks iUartClientCallbacks;
BLE2902 i2902;
BLE2902 j2902;

void BluetoothTask::startB2BAdvertising()
{
	if (!b2b_advertising_enabled)
	{
		ESP_LOGI(LOGTAG, "STARTING BT ADVERTISEMENT");
		pAdvertising->start();
		b2b_advertising_enabled = true;
	}
	else
	{
		ESP_LOGI(LOGTAG, "ADVERTISEMENT ALREADY RUNNING");
	}
}

void BluetoothTask::stopB2BAdvertising()
{
	if (b2b_advertising_enabled)
	{
		ESP_LOGI(LOGTAG, "STOPPING BT ADVERTISEMENT");
		pAdvertising->stop();
		b2b_advertising_enabled = false;
	}
	else
	{
		ESP_LOGI(LOGTAG, "ADVERTISEMENT ALREADY STOPPED");
	}
}


void BluetoothTask::scan(bool active)
{
	ESP_LOGI(LOGTAG, "STARTING SCAN");
	if (scan_started)
	{
		ESP_LOGI(LOGTAG, "SCAN ALREADY IN PROGRESS. ABORTED.");
		return;
	}
	scan_started = true;

	pScan->setActiveScan(active);
	pScan->start(3);

	if (pScanCallbacks->server_found)
	{
		server_found = pScanCallbacks->server_found;
		pServerAddress = pScanCallbacks->pServerAddress;
	}

	scan_started = false;
	ESP_LOGI(LOGTAG, "SCAN COMPLETE");
	return;
}

void BluetoothTask::pair()
{
	ESP_LOGI(LOGTAG, "STARTING PAIRING");
	pPairingClient->connect(*pServerAddress);
	ESP_LOGI(LOGTAG, "PAIRED");
}

void BluetoothTask::unpair()
{
	pPairingClient->disconnect();
}

void BluetoothTask::setB2BAdvData(std::string new_name, std::string new_man_data)
{
	// TODO: Check length < 10 characters
	ESP_LOGI(LOGTAG, "SETTING ADVERTISMENT DATA");
	adv_name = new_name;
	adv_manufacturer = "DN" + new_man_data;

	// Stop advertising, re-set data, return advertising to original state
	bool original_state = b2b_advertising_enabled;
	if (original_state)
	{
		stopB2BAdvertising();
	}

	// Setup data
	BLEAdvertisementData adv_data;
	adv_data.setAppearance(0x26DC);
	adv_data.setFlags(0x6);
	adv_data.setName(adv_name);
	adv_data.setManufacturerData(adv_manufacturer);

	pAdvertising->setAdvertisementData(adv_data);

	// Restart if we were already advertising
	if (original_state)
	{
		startB2BAdvertising();
	}
}

static void clientRxCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
							uint8_t* pData, size_t length, bool isNotify)
{   
	//std::string dstr((const char*)pData, length); 
	//ESP_LOGI("Client Notified of", "%s", dstr.c_str()); 
	char *qbuf = (char *)malloc(length+1);
	memcpy(qbuf, pData, length);
	qbuf[length] = '\x00';

	// FIXME: I almost certainly can't just pass the pointer to the RemoteChar
	// it likely gets free'd the moment this function completes.
	xQueueSendFromISR(pBTTask->CallbackQueueHandle, &qbuf, NULL);
}


#define CbackQTimeout ((TickType_t) 500/portTICK_PERIOD_MS)
unsigned long missed = 0;
void BluetoothTask::do_client_behavior()
{
	char * msg = nullptr;
	if (firstSend)
	{
		iUartClientCallbacks.pTxChar->writeValue("doodlebug");
		firstSend = false;
		return;
	}

	if(xQueueReceive(CallbackQueueHandle, &msg, CbackQTimeout) ==
		pdTRUE)
	{
		ESP_LOGI(LOGTAG, "RX Queue read: %s", msg);
		if (iUartClientCallbacks.connected)
		{
			iUartClientCallbacks.pTxChar->writeValue(msg);
		}
		free(msg);
	}
}

void BluetoothTask::do_server_behavior()
{
	char * msg = nullptr;
	if (iUartServerCallbacks.isConnected)
	{
		if(xQueueReceive(CallbackQueueHandle, &msg, CbackQTimeout) ==
			pdTRUE)
		{
			ESP_LOGI(LOGTAG, "RX Queue read: %s", msg);
			pUartTxCharacteristic->setValue(msg);
			pUartTxCharacteristic->notify();
			free(msg);
		}
	}
}

void BluetoothTask::dispatchCmd(BTCmd *cmd)
{
	switch(*cmd)
	{
	case BT_CMD_START_B2B:
		startB2BAdvertising();
		*cmd = BT_CMD_SET_B2B_ADV_DATA;
		break;
	case BT_CMD_STOP_B2B:
		ESP_LOGI(LOGTAG, "");
		stopB2BAdvertising();
		*cmd = BT_CMD_UNK;
		break;
	case BT_CMD_SET_B2B_ADV_DATA:
		setB2BAdvData("GOURRY!!!", "INFECT");
		*cmd = BT_CMD_ACTIVE_SCAN;
		break;
	case BT_CMD_PASSIVE_SCAN:
		scan(false);
		if (server_found)
			*cmd = BT_CMD_PAIR;
		break;
	case BT_CMD_ACTIVE_SCAN:
		scan(true);
		if (server_found)
			*cmd = BT_CMD_PAIR;
		break;
	case BT_CMD_PAIR:
		isActingClient = true;
		pair();
		iUartClientCallbacks.afterConnect();
		iUartClientCallbacks.pRxChar->registerForNotify(&clientRxCallback);
		*cmd = BT_CMD_UNK;
		break;
	default:
		if (isClient)
			do_client_behavior();
		else
			do_server_behavior();
		break;
	}
}

void BluetoothTask::run(void * data)
{
	// TODO: Get commands from Queue
	ESP_LOGI(LOGTAG, "RUNNING");
	BTCmd cmd;
	if (isClient)
		cmd = BT_CMD_PASSIVE_SCAN;
	else
	{
		cmd = BT_CMD_UNK;
	}
	while (1)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		dispatchCmd(&cmd);
	}
}

bool BluetoothTask::init()
{
	ESP_LOGI(LOGTAG, "INIT");
	
	// Save a pointer to this globally so we can access the queues
	// from a static function
	pBTTask = this;

	// Setup the queue
	CallbackQueueHandle = xQueueCreateStatic(CBACK_MSG_QUEUE_SIZE,
												CBACK_MSG_ITEM_SIZE,
												CallbackBuffer,
												&CallbackQueue);	
	
	BLEDevice::init("DCDN BLE Device");
	
	BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
	pMySecurity = new MySecurity();
	pMySecurity->pBTTask = this;
	BLEDevice::setSecurityCallbacks(pMySecurity);

	pSecurity = new BLESecurity();

	pSecurity->setKeySize();
	pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM);
	pSecurity->setCapability(ESP_IO_CAP_KBDISP);
	pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
	pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

	// Create out Bluetooth Server
	pServer = BLEDevice::createServer();

	// Create the UART Service
	pService = pServer->createService(uartServiceUUID);
	// setup characteristic for the server to send info
	UartRxCallbacks.CallbackQueueHandle = CallbackQueueHandle;
	pUartRxCharacteristic = pService->createCharacteristic(uartCosiUUID, uartCosiCharProps);
	pUartRxCharacteristic->setCallbacks(&UartRxCallbacks);
	pUartRxCharacteristic->addDescriptor(&i2902);

	// setup characteristic for the client to send info
	pUartTxCharacteristic = pService->createCharacteristic(uartCisoUUID, uartCisoCharProps);
	j2902.setNotifications(true);
	pUartTxCharacteristic->addDescriptor(&j2902);

	// set the callbacks and start the service
	iUartServerCallbacks.isConnected = false;
	iUartServerCallbacks.pBTTask = this;
	pServer->setCallbacks(&iUartServerCallbacks);
	pService->start();

	// Setup advertising object
	pAdvertising = pServer->getAdvertising();
	pAdvertising->addServiceUUID(uartServiceUUID);
	setB2BAdvData(adv_name, adv_manufacturer);
	startB2BAdvertising();

	// setup pairing client
	pPairingClient = BLEDevice::createClient();
	iUartClientCallbacks.pBTTask = this;
	pPairingClient->setClientCallbacks(&iUartClientCallbacks);

	// Setup scanning object and callbacks
	pScan = BLEDevice::getScan();
	pScanCallbacks = new MyScanCallbacks();
	pScan->setAdvertisedDeviceCallbacks(pScanCallbacks);

	return true;
}

BluetoothTask::BluetoothTask(const std::string &tName, uint16_t stackSize, uint8_t p)
	: Task(tName, stackSize, p), 
		CallbackQueue(), 
		CallbackQueueHandle(nullptr),
		CallbackBuffer() {
	ESP_LOGI(LOGTAG, "CREATE\n");
}

BluetoothTask::~BluetoothTask() {
	// TODO
}
