#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <map>
#include <nvs_flash.h>

// include main dc26 ESP device code for flatbuffers
#include "../mcu_to_mcu.h"
#include "../stm_to_esp_generated.h"
#include "../esp_to_stm_generated.h"

// Main BLE library files
#include "../lib/ble/BLE2902.h"
#include "../lib/ble/BLEDevice.h"

//DC26 BLE Files
#include "../dc26.h"
#include "ble.h"
#include "services.h" // UUIDs for all potential services and characteristics
#include "pairing.h"
#include "scanning.h"
#include "ble_serial.h"
#include "./security.h"
#include "../display_handler.h"

const char *BluetoothTask::LOGTAG = "BluetoothTask";
static const char* BT_CFILE_PATH = "bt_name.conf";

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

char mesgbuf[90];
unsigned int midx = 0;
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
							uint8_t* pData, size_t length, bool isNotify)
{
	flatbuffers::FlatBufferBuilder fbb;
	flatbuffers::Offset<darknet7::ESPToSTM> of;
	if (pData[0] == '0')
	{
		// clear mesgbuf and start from beginning
		midx = 0;
		memset(mesgbuf, 0, 90);
		memcpy(&mesgbuf[midx], &pData[1], length-1);
		midx += (length-1);
	}
	else if (pData[0] == '1')
	{
		// continue copying, don't send
		memcpy(&mesgbuf[midx], &pData[1], length-1);
		midx += (length-1);
	}
	else if (pData[0] == '2')
	{
		// finished copying
		memcpy(&mesgbuf[midx], &pData[1], length-1);
		midx += (length-1);

		// send MessageFromBob STM
		auto sdata = fbb.CreateString(mesgbuf, midx);
		auto sendData = darknet7::CreateBLEMessageFromDevice(fbb, sdata);
		of = darknet7::CreateESPToSTM(fbb, 0,
			darknet7::ESPToSTMAny_BLEMessageFromDevice, sendData.Union());
		darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
		getMCUToMCU().send(fbb);
	}
	else if (pData[0] == '3')
	{
		// send Pairing Complete
		auto sendData = darknet7::CreateBLEPairingComplete(fbb);
		of = darknet7::CreateESPToSTM(fbb, 0,
			darknet7::ESPToSTMAny_BLEPairingComplete, sendData.Union());
		darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
		getMCUToMCU().send(fbb);
	}
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
	std::string mandata(this->man_buf, 7);
	BLEAdvertisementData adv_data;
	adv_data.setAppearance(0x26DC);
	adv_data.setFlags(0x6);
	adv_data.setName(adv_name);
	adv_data.setManufacturerData(mandata);
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
	this->man_buf[2] = (char)devtype;
}


void BluetoothTask::setDeviceName(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetDeviceName* devName = m->Msg_as_BLESetDeviceName();
	this->adv_name = devName->name()->str();
	BLEDevice::setDeviceName(this->adv_name);
	if (this->nvs_file_opened)
	{
		nvs_set_str(nvs_fp, "devname", this->adv_name.c_str());
		nvs_commit(nvs_fp);
	}
	// TODO: Set badge name from flash
	this->refreshAdvertisementData();
}

void BluetoothTask::getInfectionData()
{
	uint16_t infections = (man_buf[3] << 8) | (man_buf[4]);
	uint16_t exposures = pScanCallbacks->getExposures();
	uint16_t cures = pScanCallbacks->getCures();

	//printf("exposures: %04x\n", exposures);

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
}

void BluetoothTask::setInfectionData(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetInfectionData* msg = m->Msg_as_BLESetInfectionData();
	uint16_t idata = msg->vectors();
	//printf("infections: %04x\n", idata);
	this->man_buf[3] = (char)((idata >> 8) & 0xFF);
	this->man_buf[4] = (char)(idata & 0xFF);
	this->refreshAdvertisementData();
}

void BluetoothTask::setCureData(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESetCureData* msg = m->Msg_as_BLESetCureData();
	uint16_t cdata = msg->vectors();
	this->man_buf[5] = (char)((cdata >> 8) & 0xFF);
	this->man_buf[6] = (char)(cdata & 0xFF);
	this->refreshAdvertisementData();
}

void BluetoothTask::scanForDevices(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLEScanForDevices* msg = m->Msg_as_BLEScanForDevices();
	uint8_t filter = msg->filter();
	
	pScanCallbacks->setFilter(filter);
	pScan->setActiveScan(true);
	pScan->start(5);
	std::map<std::string, std::string> results = pScanCallbacks->getResults();

	std::vector<flatbuffers::Offset<darknet7::Badge>> badges;
	flatbuffers::FlatBufferBuilder fbb;
	int count = 0;
	for (const auto &p : results)
	{
		if(++count<5) {
			auto addr = fbb.CreateString(p.first);
			auto name = fbb.CreateString(p.second);
			flatbuffers::Offset<darknet7::Badge> badge = darknet7::CreateBadge(fbb, name, addr);
			badges.push_back(badge);
		}
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

	pMySecurity->success = false;
	pClient->connect(remoteAddr);
	pMySecurity->msgInstanceID = m->msgInstanceID();
	
	iUartClientCallbacks.pClient = pClient;
	if (pClient->isConnected())
	{
		iUartClientCallbacks.afterConnect();
		iUartClientCallbacks.pRxChar->registerForNotify(&notifyCallback);
	}

	flatbuffers::FlatBufferBuilder fbb;
	auto con = darknet7::CreateBLEConnected(fbb, true, true);
	flatbuffers::Offset<darknet7::ESPToSTM> of = darknet7::CreateESPToSTM(fbb, 0,
	darknet7::ESPToSTMAny_BLEConnected, con.Union());
	darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
	getMCUToMCU().send(fbb);
	
}

void BluetoothTask::sendPINConfirmation(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESendPINConfirmation* msg = m->Msg_as_BLESendPINConfirmation();
	pMySecurity->confirmed = msg->confirm();
	ESP_LOGI(LOGTAG, "Confirmed Pin");
}

void BluetoothTask::sendDataToDevice(const darknet7::STMToESPRequest* m)
{
	const darknet7::BLESendDataToDevice* msg = m->Msg_as_BLESendDataToDevice();
	std::string buffer = msg->data()->str();
	int len = buffer.length();
	int size = 0;
	int sent = 0;
	const char* buf = buffer.c_str();
	uint8_t temp[23];

	while (len > 0)
	{
		size = (len > 22) ? 22 : len;
		if (sent == 0 && len > 22)
			temp[0] = '0';
		else
			temp[0] = (len > 22) ? '1' : '2';
		memcpy(&temp[1], &buf[sent], size);
		if (iUartClientCallbacks.isConnected)
		{
			//printf("sending dataaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
			if (iUartClientCallbacks.setup)
				iUartClientCallbacks.pTxChar->writeValue(temp, size + 1);
		}
		else if (iUartServerCallbacks.isConnected)
		{
			//printf("sending dataaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
			pUartCisoCharacteristic->setValue(temp, size + 1);
			pUartCisoCharacteristic->notify();
		}
		sent += size;
		len -= size;
	}
}

void BluetoothTask::sendDNPairComplete(const darknet7::STMToESPRequest* m)
{
	if (iUartServerCallbacks.isConnected)
	{
		pUartCisoCharacteristic->setValue("3");
		pUartCisoCharacteristic->notify();
	}
}

void BluetoothTask::disconnect()
{
	if (pClient->isConnected())
		pClient->disconnect();
	isActingClient = false;
	isActingServer = false;
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
		case darknet7::STMToESPAny_BLESendDNPairComplete:
			this->sendDNPairComplete(m);
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
	static int loopsSinceScan = 0;
	MCUToMCUTask::Message* m = nullptr;
	while (1)
	{
		loopsSinceScan += 1;
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
			if (loopsSinceScan >= 180)
			{
				// this is so we periodically attempt to get infected
				// scan without filtering anything
				pScanCallbacks->reset();
				pScan->setActiveScan(true);
				pScan->start(1); // Do a very very short scan
				loopsSinceScan = 0;
				// send the infection data back to the STM
				this->getInfectionData();
			}
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
	// TODO: Get badge name from flash
	esp_err_t err = nvs_open(BT_CFILE_PATH, NVS_READWRITE, &nvs_fp);
	char devName[15] = "\0";
	size_t nameSize = 15;
	if (err == ESP_OK)
	{
		this->nvs_file_opened = true;
		err = nvs_get_str(nvs_fp, "devname", devName, &nameSize);
		if (err == ESP_OK)
		{
			ESP_LOGI("BT Init", "Found Device name %s\n", devName);
			BLEDevice::init(devName);
			std::string devNameString(devName);
			adv_name = devNameString;
		}
		else
		{
			ESP_LOGI("BT Init", "Device name not found.  Default to DNDevice\n");
			BLEDevice::init("DNDevice");
		}
	}
	else
	{
		ESP_LOGI("BT Init", "Failed the open the fail.  Default to DNDevice\n");
		BLEDevice::init("DNDevice");
	}
	BLEDevice::setMTU(43);

	BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
	pMySecurity = new MySecurity();
	pMySecurity->pBTTask = this;
	BLEDevice::setSecurityCallbacks(pMySecurity);

	pSecurity = &iSecurity;
	pSecurity->setKeySize();
	pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM);
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
	
	init_ble_serial(pService);
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
