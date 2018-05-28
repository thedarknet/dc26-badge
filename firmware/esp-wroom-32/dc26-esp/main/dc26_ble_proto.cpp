#include <stdio.h>
#include <string.h>
#include "dc26_ble_proto.h"
#include "lib/ble/BLEDevice.h"


#define PAIR_SERVICE_UUID	"64633236-6463-646e-3230-313870616972"
#define PAIRING_CHAR_UUID	"beb5483e-36e1-4688-b7f5-ea07361b26a8"

static BLEUUID pairServiceUUID(PAIR_SERVICE_UUID);
static BLEUUID pairingCharUUID(PAIRING_CHAR_UUID);

const char *BluetoothTask::LOGTAG = "BluetoothTask";

void MyScanCallbacks::onResult(BLEAdvertisedDevice advertisedDevice)
{
	//printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
	if (advertisedDevice.haveAppearance() &&
		advertisedDevice.getAppearance() == 0x26DC)
	{
		// TODO: Report this back to the STM32
		printf("Found DC26 Device: %s \n", advertisedDevice.toString().c_str());
		//if (advertisedDevice.haveServiceUUID() &&
		//	advertisedDevice.getServiceUUID().equals(pairServiceUUID))
		//{
			// Found the device we want, stop scan, save data, setup pairing
			//printf("Found UUID for DC26 Device\n");
			advertisedDevice.getScan()->stop();
			server_found = true;
			pServerAddress = new BLEAddress(advertisedDevice.getAddress());
		//}
	}
}

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
	}
	scan_started = true;

	pScan = pDevice->getScan();
	pScan->setAdvertisedDeviceCallbacks(pScanCallbacks);
	pScan->setActiveScan(active);
	BLEScanResults foundDevices = pScan->start(scan_time);
	printf("Devices found: %d\n", foundDevices.getCount());

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
	ESP_LOGI(LOGTAG, "STARTING PAIRING\n");

	// Connect to the remote BLE Server.
	pClient->connect(*pServerAddress);
	printf("Connected to server\n");

	// Obtain a reference to the service we are after in the remote BLE server.
	BLERemoteService* pRemoteService = pClient->getService(pairServiceUUID);
	if (pRemoteService == nullptr)
	{
		printf("Failed to find our service UUID: %s\n",
				pairServiceUUID.toString().c_str());
		return;
	}
	printf("Found our service\n");


	// Obtain a reference to the characteristic in the service of the remote BLE server.
	pRemoteCharacteristic = pRemoteService->getCharacteristic(pairingCharUUID);
	if (pRemoteCharacteristic == nullptr)
	{
		printf("Failed to find our characteristic UUID: %s\n",
				pairingCharUUID.toString().c_str());
		return;
	}
	printf("Found our characteristic\n");

	// Read the value of the characteristic.
	std::string value = pRemoteCharacteristic->readValue();
	printf("The characteristic value was: %s\n", value.c_str());

	// Disconnect
	pClient->disconnect();
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
	//printf("ADV PAYLOAD: %s\n", adv_data.getPayload().c_str());

	pAdvertising->setAdvertisementData(adv_data);

	// Restart if we were already advertising
	if (original_state)
	{
		startB2BAdvertising();
	}
}

void BluetoothTask::run(void * data)
{
	// TODO: Get commands from Queue
	ESP_LOGI(LOGTAG, "RUNNING");
	BTCmd cmd = BT_CMD_ACTIVE_SCAN;
	while (1)
	{
		vTaskDelay(10000 / portTICK_PERIOD_MS);
		// TODO: Check queue for command
		switch(cmd)
		{
			case BT_CMD_START_B2B:
				startB2BAdvertising();
				cmd = BT_CMD_SET_B2B_ADV_DATA;
				break;
			case BT_CMD_STOP_B2B:
				ESP_LOGI(LOGTAG, "");
				stopB2BAdvertising();
				cmd = BT_CMD_UNK;
				break;
			case BT_CMD_SET_B2B_ADV_DATA:
				setB2BAdvData("GOURRY!!!", "INFECT");
				cmd = BT_CMD_ACTIVE_SCAN;
				break;
			case BT_CMD_PASSIVE_SCAN:
				scan(false);
				cmd = BT_CMD_ACTIVE_SCAN;
				break;
			case BT_CMD_ACTIVE_SCAN:
				scan(true);
				if (server_found)
					cmd = BT_CMD_PAIR;
				break;
			case BT_CMD_PAIR:
				pair();
				cmd = BT_CMD_UNK;
				break;
			default:
				ESP_LOGI(LOGTAG, "INFINITE LOOP");
				break;
		}
	}
}

bool BluetoothTask::init()
{
	ESP_LOGI(LOGTAG, "INIT");
	pDevice = new BLEDevice();
	pDevice->init("DCDN BLE Device");

	// Create out Bluetooth Server
	pServer = pDevice->createServer();

	// Create our DCDN Pairing Service
	pService = pServer->createService(PAIR_SERVICE_UUID);
	pPairingCharacteristic = pService->createCharacteristic(
										 PAIRING_CHAR_UUID,
										 BLECharacteristic::PROPERTY_READ);
	pPairingCharacteristic->setValue("Cryptographic Hash Goes Here");
	pService->start();

	// Create out Client for reaching out for pairing
	pClient = pDevice->createClient();

	// Setup advertising object
	pAdvertising = pServer->getAdvertising();
	setB2BAdvData("DN 1", "EGGPLNT");
	startB2BAdvertising();

	// Setup scanning callbacks
	pScanCallbacks = new MyScanCallbacks();

	return true;
}

BluetoothTask::BluetoothTask(const std::string &tName, uint16_t stackSize, uint8_t p)
	: Task(tName, stackSize, p), InQueue(), InQueueHandle(nullptr) {
	//TODO: , ESPToBTBuffer() {
	ESP_LOGI(LOGTAG, "CREATE\n");
}

BluetoothTask::~BluetoothTask() {
	// TODO
}
