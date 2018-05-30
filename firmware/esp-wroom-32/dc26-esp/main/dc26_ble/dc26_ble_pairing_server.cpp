#include <stdio.h>
#include <string.h>
#include "dc26_ble.h"
#include "dc26_ble_pairing_server.h"
#include "../lib/ble/BLEDevice.h"

const char *PAIR_SVR_TAG = "BTPairingServer";

void UartWriteCharCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
	std::string rxValue = pCharacteristic->getValue();
	if (rxValue.length() > 0)
		printf("Received Value: %s\n", rxValue.c_str());
};

void UartServerCallbacks::onConnect(BLEServer* server)
{
	ESP_LOGI(PAIR_SVR_TAG, "SERVER CONNECTION RECEIVED");
	//pServerTask = new UartServerTask("BtUartServer");
	//pServerTask->init();
	//pServerTask->start(pCharacteristic);
}

void UartServerCallbacks::onDisconnect(BLEServer* server)
{
	ESP_LOGI(PAIR_SVR_TAG, "SERVER CONNECTION DROPPED");
}


bool UartServerTask::init()
{
	// TODO
	return true;
}

void UartServerTask::run(void *data)
{
	BLECharacteristic *pCharacteristic = (BLECharacteristic *)data;
	printf("task!\n");
	while (1)
	{
		printf("sending a nom\n");
		pCharacteristic->setValue("nom");
		pCharacteristic->notify();
		delay(10000);
	}
}

UartServerTask::UartServerTask(const std::string &tName, uint16_t stackSize, uint8_t p)
	: Task(tName,stackSize,p) // TODO: Queue Stuff
{
	// TODO
}

UartServerTask::~UartServerTask()
{
	// TODO
}
