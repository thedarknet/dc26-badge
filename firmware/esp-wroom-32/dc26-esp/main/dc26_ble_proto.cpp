#include <stdio.h>
#include <string.h>
#include "dc26_ble_proto.h"
#include "lib/ble/BLEDevice.h"

const char *BluetoothTask::LOGTAG = "BluetoothTask";

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
	BTCmd cmd = BT_CMD_START_B2B;
	while (1)
	{
		vTaskDelay(20000 / portTICK_PERIOD_MS);
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
	pServer = pDevice->createServer();
	pAdvertising = pServer->getAdvertising();

	this->setB2BAdvData("DN 1", "EGGPLNT");
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
