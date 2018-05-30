#include <stdio.h>
#include <string.h>
#include "dc26_ble.h"
#include "dc26_ble_pairing_client.h"
#include "dc26_ble_pairing_server.h"
#include "../lib/ble/BLEDevice.h"

const char *PAIR_CLIENT_TAG = "BTPairingClient";


static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
							uint8_t* pData, size_t length, bool isNotify)
{
	printf("Notified of value: %s\n", pBLERemoteCharacteristic->readValue().c_str());
}

void UartClientCallbacks::onConnect(BLEClient* client)
{
	ESP_LOGI(PAIR_CLIENT_TAG, "CLIENT CONNECTED");
	pClientTask = new UartClientTask("BtUartClient");
	pClientTask->init();
	pClientTask->start(client);
}

void UartClientCallbacks::onDisconnect(BLEClient* client)
{
	ESP_LOGI(PAIR_CLIENT_TAG, "CLIENT DISCONNECTED");
	pClientTask->stop();
	delete pClientTask;
}


bool UartClientTask::init()
{
	// TODO
	return true;
}

void UartClientTask::run(void *data)
{
	BLEClient *pClient;
	BLERemoteService* pRemoteService;
	BLERemoteCharacteristic *pRemoteCharacteristic;
	BLERemoteCharacteristic *pRemoteCharacteristic2;
	
	printf("task!\n");
	pClient = (BLEClient *)data;

    // Obtain a reference to the service we are after in the remote BLE server.
	delay(500 / portTICK_PERIOD_MS);
    pRemoteService = pClient->getService(uartServiceUUID);
    if (pRemoteService == nullptr) {
		printf("service finding error\n");
		return;
    }

    // Obtain a reference to the characteristic in the service of the remote BLE server.
	delay(500 / portTICK_PERIOD_MS);
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uartWriteUUID);
    if (pRemoteCharacteristic == nullptr) {
		printf("first char finding error\n");
		return;
    }
	
	// Obtain reference to characteristic we can receive from 
	delay(500 / portTICK_PERIOD_MS);
    pRemoteCharacteristic2 = pRemoteService->getCharacteristic(uartReadUUID);
    if (pRemoteCharacteristic2 == nullptr) {
		printf("second char finding error\n");
		return;
    }
	
	delay(500 / portTICK_PERIOD_MS);
	// register to receive notifications	
	pRemoteCharacteristic2->registerForNotify(notifyCallback);
	
	while (1)
	{
		// Send a message
		delay(10000);
		printf("sending doodlebug\n");
		pRemoteCharacteristic->writeValue("doodlebug");
	}
	// TODO
	// data is the target address to connect to 

	// connect to the address

	// timeouts

	// wait for STM32 message with crypto message 1
	// write to uartWriteUUID in loop until fully sent
}

UartClientTask::UartClientTask(const std::string &tName, uint16_t stackSize, uint8_t p)
	: Task(tName,stackSize,p) // TODO: Queue Stuff
{
	// TODO
}

UartClientTask::~UartClientTask()
{
	// TODO
}
