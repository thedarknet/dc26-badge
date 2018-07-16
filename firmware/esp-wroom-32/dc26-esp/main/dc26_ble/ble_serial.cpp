#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "ble_serial.h"
#include "services.h"
#include "../lib/ble/BLEDevice.h"
#include "../lib/ble/BLE2902.h"

SerialCosiCharCallbacks SerialCosiCallbacks;
BLE2902 sCosi2902;
BLE2902 sCiso2902;
BLECharacteristic* pCisoChar;
BLECharacteristic* pCosiChar;
static const int SERIAL_MSG_QUEUE_SIZE = 10;
static const int SERIAL_MSG_ITEM_SIZE = sizeof(void *); // TODO: Msg size?
StaticQueue_t SerialQueue;
QueueHandle_t SerialQueueHandle = nullptr;
uint8_t SerialBuffer[SERIAL_MSG_QUEUE_SIZE * SERIAL_MSG_ITEM_SIZE];
SerialTask sTask;

void SerialCosiCharCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
	std::string value = pCharacteristic->getValue();
	if (value.length() > 0)
	{
		const char *msgOrig = value.c_str();
		uint32_t len = strlen(msgOrig);
		char * msg = (char *)malloc(len+1);
		memcpy(msg, msgOrig, len);
		msg[len] = '\0';
		xQueueSendFromISR(SerialQueueHandle, &msg, NULL);
	}
}


#define sTimeout ((TickType_t) 500/portTICK_PERIOD_MS)
void SerialTask::run(void* data)
{
	char * msg;
	// check SerialQueue for input and do a thing
	while (true)
	{
		if (xQueueReceive(SerialQueueHandle, &msg, sTimeout))
		{

		}
	}
}

void init_ble_serial(BLEService* pService)
{
	// init the serial queue's
	SerialQueueHandle = xQueueCreateStatic(SERIAL_MSG_QUEUE_SIZE,
											SERIAL_MSG_ITEM_SIZE,
											SerialBuffer,
											&SerialQueue);


	// create serial Ciso/Cosi characteristics
	sCiso2902.setNotifications(true);
	pCisoChar = pService->createCharacteristic(serialCisoUUID, serialCisoCharProps);
	pCisoChar->addDescriptor(&sCiso2902);
	pCisoChar->setCallbacks(&SerialCosiCallbacks);

	// 
	pCosiChar = pService->createCharacteristic(serialCosiUUID, serialCosiCharProps);
	pCosiChar->addDescriptor(&sCosi2902);

	// create write callback

	// create queue monitoring task
	sTask.start();
}
