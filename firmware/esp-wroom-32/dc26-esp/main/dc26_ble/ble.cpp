#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <map>

// include main dc26 ESP device code for flatbuffers
#include "../mcu_to_mcu.h"
#include "../stm_to_esp_generated.h"
#include "../esp_to_stm_generated.h"

// FIXME: remove this when we ship, it's just for building/testing ease
#include "swaphack.h"

// Main BLE library files
#include "../lib/ble/BLE2902.h"
#include "../lib/ble/BLEDevice.h"

//DC26 BLE Files
#include "../dc26.h"
#include "ble.h"
#include "ble_serial.h"
#include "services.h" // UUIDs for all potential services and characteristics
#include "pairing.h"
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
BLESecurity iSecurity;

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
	printf("Client Received: length %d --- %s\n", length, pData);

	/*
	flatbuffers::FlatBufferBuilder fbb;
	uint8_t *bufData = nullptr;
	flatbuffers::Offset<darknet7::STMToESPRequest> of;
	flatbuffers::uoffset_t size;
	MCUToMCUTask::Message* m;
	auto sdata = fbb.CreateString((char *)pData, length);
	auto sendData = darknet7::CreateBLESendDataToDevice(fbb, sdata);
	of = darknet7::CreateSTMToESPRequest(fbb, 0,
		darknet7::STMToESPAny_BLESendDataToDevice, sendData.Union());
	darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb, of);
	size = fbb.GetSize();
	bufData = fbb.GetBufferPointer();
	m = new MCUToMCUTask::Message();
	m->set(size, 0, bufData);
	xQueueSend(pBTTask->getQueueHandle(), &m, (TickType_t) 0);
	*/
	// TODO: actually send it back to the STM
}

static void sendGenericResponse(bool result)
{
	/*
	darknet7::RESPONSE_SUCCESS res = result ? darknet7::RESPONSE_SUCCESS_True : darknet7::RESPONSE_SUCCESS_False;
	flatbuffers::FlatBufferBuilder fbb;
	flatbuffers::Offset<darknet7::ESPToSTM> of;
	auto infect = darknet7::CreateGenericResponse(fbb, res);
	of = darknet7::CreateESPToSTM(fbb, 0, darknet7::ESPToSTMAny_GenericResponse,
		infect.Union());
	darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
	getMCUToMCU().send(fbb);
	*/
	return;
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
	if (state)
		this->startAdvertising();
	else
		this->stopAdvertising();
	sendGenericResponse(true);
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

void BluetoothTask::setDeviceType(uint8_t devtype)
{
	// third byte of the adv_menufacturer device is the device type
	this->adv_manufacturer[2] = (char)devtype;
	sendGenericResponse(true);
}


void BluetoothTask::setDeviceName(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetDeviceName* devName = m->Msg_as_BLESetDeviceName();
	this->adv_name = devName->name()->str();
	this->refreshAdvertisementData();
	sendGenericResponse(true);
}

void BluetoothTask::getInfectionData()
{
	uint16_t infections = (adv_manufacturer[3] << 8) | (adv_manufacturer[4]);
	uint16_t exposures = pScanCallbacks->getExposures();
	uint16_t cures = pScanCallbacks->getCures();
	flatbuffers::FlatBufferBuilder fbb;
	flatbuffers::Offset<darknet7::ESPToSTM> of;
	auto infect = darknet7::CreateBLEInfectionData(fbb, infections, exposures, cures);
	of = darknet7::CreateESPToSTM(fbb, 0, darknet7::ESPToSTMAny_BLEInfectionData,
		infect.Union());
	darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
	getMCUToMCU().send(fbb);
}

void BluetoothTask::setExposureData(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetExposureData* msg = m->Msg_as_BLESetExposureData();
	uint16_t edata = msg->vectors();
	pScanCallbacks->exposures = edata;
	sendGenericResponse(true);
}

void BluetoothTask::setInfectionData(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetInfectionData* msg = m->Msg_as_BLESetInfectionData();
	uint16_t idata = msg->vectors();
	this->adv_manufacturer[3] = (char)((idata >> 8) & 0xFF);
	this->adv_manufacturer[4] = (char)(idata & 0xFF);
	this->refreshAdvertisementData();
	sendGenericResponse(true);
}

void BluetoothTask::setCureData(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetCureData* msg = m->Msg_as_BLESetCureData();
	uint16_t cdata = msg->vectors();
	this->adv_manufacturer[5] = (char)((cdata >> 8) & 0xFF);
	this->adv_manufacturer[6] = (char)(cdata & 0xFF);
	this->refreshAdvertisementData();
	sendGenericResponse(true);
}

void BluetoothTask::scanForDevices(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLEScanForDevices* msg = m->Msg_as_BLEScanForDevices();
	uint8_t filter = msg->filter();
	printf("scanning!\n");

	pScanCallbacks->setFilter(filter);
	pScan->setActiveScan(true);
	pScan->start(5);
	std::map<std::string, std::string> results = pScanCallbacks->getResults();

	std::vector<flatbuffers::Offset<darknet7::Badge>> badges;
	flatbuffers::FlatBufferBuilder fbb;
	for (const auto &p : results)
	{
		printf("Device Found: %s - %s\n", p.first.c_str(), p.second.c_str());
		auto name = fbb.CreateString(p.first);
		auto addr = fbb.CreateString(p.second);
		flatbuffers::Offset<darknet7::Badge> badge = darknet7::CreateBadge(fbb, name, addr);
		badges.push_back(badge);
	}
	auto badgeList = fbb.CreateVector<flatbuffers::Offset<darknet7::Badge>>(badges);
	auto badgesInArea = darknet7::CreateBadgesInArea(fbb, badgeList);
	flatbuffers::Offset<darknet7::ESPToSTM> of = darknet7::CreateESPToSTM(fbb,
		m->msgInstanceID(), darknet7::ESPToSTMAny_BadgesInArea, badgesInArea.Union());
	darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
	getMCUToMCU().send(fbb);
	
	return;
}

void BluetoothTask::pairWithDevice(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLEPairWithDevice* msg = m->Msg_as_BLEPairWithDevice();
	std::string addr = msg->addr()->str();
	BLEAddress remoteAddr = BLEAddress(addr);
	this->isActingClient = true;
	pClient->connect(remoteAddr);

	pMySecurity->msgInstanceID = m->msgInstanceID();	
	if (pClient->isConnected())
	{
		iUartClientCallbacks.afterConnect();
		iUartClientCallbacks.pRxChar->registerForNotify(&notifyCallback);
	}

	sendGenericResponse(pClient->isConnected());
}

void BluetoothTask::sendPINConfirmation(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESendPINConfirmation* msg = m->Msg_as_BLESendPINConfirmation();
	pMySecurity->confirmed = msg->confirm();
	ESP_LOGI(LOGTAG, "Confirmed Pin");
	sendGenericResponse(true);
}

void BluetoothTask::sendDataToDevice(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESendDataToDevice* msg = m->Msg_as_BLESendDataToDevice();
	bool sent = false;
	// TODO: Check if null bytes work
	std::string buffer = msg->data()->str();
	if (iUartClientCallbacks.isConnected)
	{
		printf("Client Sending: %s\n", buffer.c_str());
		iUartClientCallbacks.pTxChar->writeValue(buffer);
		sent = true;
	}
	else if (iUartServerCallbacks.isConnected)
	{
		printf("Server Sending: %s\n", buffer.c_str());
		pUartCisoCharacteristic->setValue(buffer);
		pUartCisoCharacteristic->notify();
		sent = true;
	}
	
	sendGenericResponse(sent);
}

void BluetoothTask::disconnect()
{
	if (pClient->isConnected())
		pClient->disconnect();

	isActingClient = false;
	isActingServer = false;
	sendGenericResponse(true);
}

/*
	Command Handler:  handle BLE commands sent from the STM device

	Switch on message type, and dispatch to the appropriate function
*/
void BluetoothTask::commandHandler(MCUToMCUTask::Message* msg)
{
	auto m = msg->asSTMToESP();
	switch (m->Msg_type())
	{
		case darknet7::STMToESPAny_BLEAdvertise:
			this->toggleAdvertising(m);
			break;
		case darknet7::STMToESPAny_BLESetDeviceName:
			this->setDeviceName(m);
			break;
		case darknet7::STMToESPAny_BLEGetInfectionData:
			this->getInfectionData();
			break;
		case darknet7::STMToESPAny_BLESetExposureData:
			this->setExposureData(m);
			break;
		case darknet7::STMToESPAny_BLESetInfectionData:
			this->setInfectionData(m);
			break;
		case darknet7::STMToESPAny_BLESetCureData:
			this->setCureData(m);
			break;
		case darknet7::STMToESPAny_BLEScanForDevices:
			this->scanForDevices(m);
			break;
		case darknet7::STMToESPAny_BLEPairWithDevice:
			this->pairWithDevice(m);
			break;
		case darknet7::STMToESPAny_BLESendPINConfirmation:
			this->sendPINConfirmation(m);
			break;
		case darknet7::STMToESPAny_BLESendDataToDevice:
			this->sendDataToDevice(m);
			break;
		case darknet7::STMToESPAny_BLEDisconnect:
			this->disconnect();
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
			if (m != nullptr)
			{
				this->commandHandler(m);
				delete m;
			}
		}
		else
		{
			// TODO: check how long since last scan, scan if beyond that time
			// this is so we periodically attempt to get infected
			// scan without filtering anything
			//pScanCallbacks->reset();
			//pScan->setActiveScan(true);
			//pScan->start(1); // Do a very very short scan
		}
	}
}

bool BluetoothTask::init()
{
	ESP_LOGI(LOGTAG, "INIT");

	pBTTask = this;

	isActingClient = false;
	isActingServer = false;

	STMQueueHandle = xQueueCreateStatic(STM_MSG_QUEUE_SIZE, STM_MSG_ITEM_SIZE,
										fromSTMBuffer, &STMQueue);
	BLEDevice::init("DNDevice");
	BLEDevice::setMTU(43);

	BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
	pMySecurity = new MySecurity();
	pMySecurity->pBTTask = this;
	BLEDevice::setSecurityCallbacks(pMySecurity);

	pSecurity = &iSecurity;
	pSecurity->setKeySize();
	pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM);
	//pSecurity->setCapability(ESP_IO_CAP_KBDISP);
	pSecurity->setCapability(ESP_IO_CAP_IO);
	pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
	pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

	// Create out Bluetooth Server
	pServer = BLEDevice::createServer();

	// Create the UART Service
	pService = pServer->createService(uartServiceUUID);
	// setup characteristic for the client to send info
	UartCosiCallbacks.pBTTask = this;
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

	// setup the advertising data
	this->setDeviceType(darknet7::BLEDeviceFilter_BADGE);
	this->refreshAdvertisementData();
	this->startAdvertising();

	// setup pairing client
	pClient = BLEDevice::createClient();
	iUartClientCallbacks.pBTTask = this;
	pClient->setClientCallbacks(&iUartClientCallbacks);

	// Setup scanning object and callbacks
	pScan = BLEDevice::getScan();
	pScanCallbacks = new MyScanCallbacks();
	pScan->setAdvertisedDeviceCallbacks(pScanCallbacks);

	//init_ble_serial(pService);

	return true;
}

BluetoothTask::BluetoothTask(const std::string &tName, uint16_t stackSize, uint8_t p)
	: Task(tName, stackSize, p),
		STMQueue(),
		STMQueueHandle(nullptr),
		fromSTMBuffer() {
	ESP_LOGI(LOGTAG, "CREATE\n");
}

BluetoothTask::~BluetoothTask() {
	// TODO
}
