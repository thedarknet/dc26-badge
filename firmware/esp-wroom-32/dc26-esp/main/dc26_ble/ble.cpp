#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// include main dc26 ESP device code for flatbuffers
#include "../mcu_to_mcu.h"
#include "../stm_to_esp_generated.h"

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
	if (!advertising_enabled)
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

void BluetoothTask::toggleAdvertising(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLEAdvertise* advert = m->Msg_as_BLEAdvertise();
	bool state = advert->state();
	// TODO
	if (state)
		this->startAdvertising();
	else
		this->stopAdvertising();
	return;
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

void BluetoothTask::setDeviceName(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetDeviceName* devName = m->Msg_as_BLESetDeviceName();
	// TODO: length limit?
	this->adv_name = devName->name()->str();
	this->refreshAdvertisementData();
}

void BluetoothTask::getInfectionData()
{
	// TODO: return this->adv_manufacturer[bytes]
}

void BluetoothTask::setInfectionData(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetInfectionData* msg = m->Msg_as_BLESetInfectionData();
	uint16_t idata = msg->vectors();
	(void) idata; // TODO: manufacturerData[slice] = idata
	this->refreshAdvertisementData();
}

void BluetoothTask::getCureData()
{
	// TODO: return this->adv_manufacturer[bytes]
}


void BluetoothTask::setCureData(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetCureData* msg = m->Msg_as_BLESetCureData();
	uint16_t cdata = msg->vectors();
	(void) cdata; // TODO: manufacturerData[slice] = idata
	this->refreshAdvertisementData();
}

void BluetoothTask::scanForDevices(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLEScanForDevices* msg = m->Msg_as_BLEScanForDevices();
	uint8_t filter = msg->filter();
	(void) filter;
	// TODO: m->filter()
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

void BluetoothTask::pairWithDevice(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLEPairWithDevice* msg = m->Msg_as_BLEPairWithDevice();
	std::string addr = msg->addr()->str();
	(void) addr;
	// TODO: m->addr()
	// TODO
}

void BluetoothTask::sendPINConfirmation(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESendPINConfirmation* msg = m->Msg_as_BLESendPINConfirmation();
	bool confirm = msg->confirm();
	if (confirm)
	{
		// TODO allow
	}
	else
	{
		// TODO reject
	}
}

void BluetoothTask::getConnectedDevices()
{
	// TODO
}

void BluetoothTask::sendDataToDevice(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESendDataToDevice* msg = m->Msg_as_BLESendDataToDevice();
	std::string addr = msg->addr()->str();
	uint8_t length = msg->length();
	const uint8_t* data = msg->data()->Data();
	(void)addr;
	(void)length;
	(void)data;
	// TODO
}

void BluetoothTask::disconnectFromDevice(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLEDisconnectFromDevice* msg = m->Msg_as_BLEDisconnectFromDevice();
	std::string name = msg->name()->str();
	(void)name;
	// TODO: name->device map, disconnect
}

void BluetoothTask::disconnectFromAll()
{
	// TODO: for all device in name->device map, disconnect
}


/*
	Command Handler:  handle BLE commands sent from the STM device

	Switch on message type, and dispatch to the appropriate function
*/
void BluetoothTask::commandHandler(MCUToMCUTask::Message* msg)
{
	const darknet7::STMToESPRequest* m = msg->asSTMToESP();
	switch (m->Msg_type())
	{
		case darknet7::STMToESPAny_BLEAdvertise:
			this->toggleAdvertising(m);
			break;
		case darknet7::STMToESPAny_BLEGetDeviceName:
			// TODO: return device name
			this->getDeviceName();
			break;
		case darknet7::STMToESPAny_BLESetDeviceName:
			// TODO: update name in advertising data
			break;
		case darknet7::STMToESPAny_BLEGetInfectionData:
			// TODO: return infection data from advertising data
			this->getInfectionData();
			break;
		case darknet7::STMToESPAny_BLESetInfectionData:
			// TODO: set infection data in advertising data
			break;
		case darknet7::STMToESPAny_BLEGetCureData:
			// TODO: return cure data from advertising data
			this->getCureData();
			break;
		case darknet7::STMToESPAny_BLESetCureData:
			// TODO: set cure data in advertising data
			break;
		case darknet7::STMToESPAny_BLEScanForDevices:
			// TODO: this->scanForDevices(msg) // msg has filter info
			// TODO: send results/failure
			break;
		case darknet7::STMToESPAny_BLEPairWithDevice:
			// TODO: add address to pairWithBadge queue to be handled by the
			// badge pairing task
			break;
		case darknet7::STMToESPAny_BLESendPINConfirmation:
			// TODO: set security callbacks confirmation value to true or cancel
			break;
		case darknet7::STMToESPAny_BLEGetConnectedDevices:
			// TODO: serialize name's of connected devices into return message
			break;
		case darknet7::STMToESPAny_BLESendDataToDevice:
			// TODO: this->sendDataToDevice(msg) // msg has addr and data
			break;
		case darknet7::STMToESPAny_BLEDisconnectFromDevice:
			// TODO: this->disconnectFromDevice(msg) // msg has address info
			break;
		case darknet7::STMToESPAny_BLEDisconnectFromAll:
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

	If we haven't done a BLE scan in a while, go ahead and do it.
	This is our primary infection-vector between badges
*/
#define CmdQueueTimeout ((TickType_t) 1000 / portTICK_PERIOD_MS)
void BluetoothTask::run(void * data)
{
	MCUToMCUTask::Message* m = nullptr;
	while (1)
	{
		if (xQueueReceive(STMQueueHandle, &m, CmdQueueTimeout))
		{
			this->commandHandler(m);
		}
		else
		{
			// TODO: check how long since last scan, scan if beyond that time
			// this is so we periodically attempt to get infected
			// scan without filtering anything
		}
		delete m;
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

	STMQueueHandle = xQueueCreateStatic(STM_MSG_QUEUE_SIZE, STM_MSG_ITEM_SIZE,
										fromSTMBuffer, &STMQueue);

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
	refreshAdvertisementData();
	startAdvertising();

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
