#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// FIXME: remove this when we ship, it's just for building/testing ease
#include "swaphack.h"

// Main BLE library files
#include "../lib/ble/BLE2902.h"
#include "../lib/ble/BLEDevice.h"

//DC26 BLE Files
#include "ble.h"
#include "ble_serial.h"
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
UartCosiCharCallbacks UartCosiCallbacks;
UartServerCallbacks iUartServerCallbacks;
UartClientCallbacks iUartClientCallbacks;
BLE2902 i2902;
BLE2902 j2902;


/*
	notifyCallback:  called when the server sends a notify() for the characteristic

	pData will contain the first 20 bytes of data, but the length could be longer
	instead of playing the game of 20 byte chunks or trying to race to read it
	before it is overwritten, what we'll do is pass the pointer to the characteristic
	too the out queue, and do a read().  

	On the server-side, we will not overwrite the current data unless
		1) a read() has occurred, or
		2) an override event occurs (i.e. a disconnection)
*/
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
							uint8_t* pData, size_t length, bool isNotify)
{   
	xQueueSendFromISR(pBTTask->CallbackQueueHandle, &pBLERemoteCharacteristic, NULL);
	// FIXME: send the pointer to the characteristic in the queue
	// then do a read() of the characteristic
	// register an onRead callback which sets the characteristic to be
	// write-able again.
}

void BluetoothTask::startAdvertising()
{
	if (!b2b_advertising_enabled)
	{
		ESP_LOGI(LOGTAG, "STARTING BT ADVERTISEMENT");
		pAdvertising->start();
		advertising_enabled = true;
	}
}

void BluetoothTask::stopAdvertising()
{
	if (advertising_enabled)
	{
		ESP_LOGI(LOGTAG, "STOPPING BT ADVERTISEMENT");
		pAdvertising->stop();
		advertising_enabled = false;
	}
}

void BluetoothTask::refreshAdvertisementData()
{
	// Stop advertising, re-set data, return advertising to original state
	bool original_state = advertising_enabled;
	if (original_state)
	{
		this->stopAdvertising();
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
		this->startAdvertising();
	}
}


void BluetoothTask::getDeviceName()
{
	//TODO: serialize and return this->adv_name
}

void BlueoothTask::setDeviceName(darknet7::STMToESPRequest* msg)
{
	//TODO: set this->adv_name to input
	this->refreshAdvertisementData();
}

void BluetoothTask::getInfectionData()
{
	// TODO
}

void BluetoothTask::setInfectionData(darknet7::STMToESPRequest* msg)
{
	// TODO
}

void BluetoothTask::getCureData()
{
	// TODO
}

void BluetoothTask::setCureData(darknet7::STMToESPRequest* msg)
{
	// TODO
}

void BluetoothTask::scanForDevices(darknet7::STMToESPRequest* msg)
{
	// TODO: confirm msg to BLEScanForDevices object
	// TODO: set scan filter
	// TODO: pScan->setActiveScan(false)
	// TODO: pScan->start(timeout)

	// TODO: infection code
	// TODO: cure code
	// TODO: serialize name:address map into results
	// TODO: send results back to STM
	return;
}

void BluetoothTask::pairWithDevice(darknet7::STMToESPRequest* msg)
{
	// TODO
}

void getConnectedDevices()
{
	// TODO
}

void sendDataToDevice(darknet7::STMToESPRequest* msg)
{
	// TODO
}

void disconnectFromDevice(darknet7::STMToESPRequest* msg)
{
	// TODO
}

void disconnectFromAll()
{
	// TODO
}


/*
	Command Handler:  handle BLE commands sent from the STM device

	Switch on message type, and dispatch to the appropriate function
*/
void BluetoothTask::commandHandler(MCUToMCUTask::Message* msg)
{
	const darknet7::STMToESPRequest* msg = m->asSTMToESP();
	switch (msg->Msg_type())
	{
		case BLEToggleAdvertising:
			// TODO 
			// if (msg->state)
				// this->startAdvertising();
			// else 
				// this->stopAdvertising();
			break;
		case BLEGetDeviceName:
			// TODO: return device name
			break;
		case BLESetDeviceName:
			// TODO: update name in advertising data
			break;
		case BLEGetInfectionData:
			// TODO: return infection data from advertising data
			break;
		case BLESetInfectionData:
			// TODO: set infection data in advertising data
			break;
		case BLEGetCureData:
			// TODO: return cure data from advertising data
			break;
		case BLESetCureData:
			// TODO: set cure data in advertising data
			break;
		case BLEScanForDevices:
			// TODO: this->scanForDevices(msg) // msg has filter info
			// TODO: send results/failure
			break;
		case BLEPairWithDevice:
			// TODO: add address to pairWithBadge queue to be handled by the
			// badge pairing task
			break;
		case BLESendPINConfirmation:
			// TODO: set security callbacks confirmation value to true or cancel
			break;
		case BLEGetConnectedDevices:
			// TODO: serialize name's of connected devices into return message
			break;
		case BLESendDataToDevice:
			// TODO: this->sendDataToDevice(msg) // msg has addr and data
			break;
		case BLEDisconnectFromDevice:
			// TODO: this->disconnectFromDevice(msg) // msg has address info
			break;
		case BLEDisconnectFromAll:
			this->disconnectFromAll();
			break;
		default :
			// send failure
			break;
	}
	// send success
}

/* 
	Main Bluetooth Task:  get messages from STMtoESP Queue and dispatch
*/
#define CmdQueueTimeout ((TickType_t) 1000 / portTICK_PERIOD_MS)
void BluetoothTask::run(void * data)
{
	MCUToMCUTask::Message* m = nullptr;
	while (1)
	{
		if (xQueueReceive(getQueueHandle(), &m, CmdQueueTimeout)
			dispatchCmd(m);
	}
}

bool BluetoothTask::init()
{
	ESP_LOGI(LOGTAG, "INIT");

	// TODO: Setup pairing task with its own queue, for out-of-band command
	// handling by the connection logic
	// this is required to be able to cancel a connection request and to handle
	// pin confirmation input

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
	// setup characteristic for the client to send info
	UartCosiCallbacks.CallbackQueueHandle = CallbackQueueHandle;
	pUartCosiCharacteristic = pService->createCharacteristic(uartCosiUUID, uartCosiCharProps);
	pUartCosiCharacteristic->setCallbacks(&UartCosiCallbacks);
	pUartCosiCharacteristic->addDescriptor(&i2902);

	// setup characteristic for the server to send info
	pUartCisoCharacteristic = pService->createCharacteristic(uartCisoUUID, uartCisoCharProps);
	j2902.setNotifications(true);
	pUartCisoCharacteristic->addDescriptor(&j2902);

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

	init_ble_serial(pService);

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
