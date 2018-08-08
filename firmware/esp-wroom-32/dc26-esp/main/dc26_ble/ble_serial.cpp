#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "ble_serial.h"
#include "services.h"

#include "../dc26.h"
#include "../display_handler.h"
#include "../lib/ble/BLEDevice.h"
#include "../lib/ble/BLE2902.h"

SerialCosiCharCallbacks SerialCosiCallbacks;
BLE2902 sCosi2902;
BLE2902 sCiso2902;
BLECharacteristic* pCosiChar;
BLECharacteristic* pCisoChar;

void SerialCosiCharCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
	std::string value = pCharacteristic->getValue();
	if (value.length() > 0)
	{
		const char *msgOrig = value.c_str();
		uint32_t len = strlen(msgOrig);
		if (len > 12)
			len = 12; // Screens not that big

		DisplayTask::DisplayMsg* dmsg = new DisplayTask::DisplayMsg();
		memset(dmsg->Msg, 0, sizeof(dmsg->Msg));
		memcpy(dmsg->Msg, msgOrig, len);
		xQueueSendFromISR(getDisplayTask().getQueueHandle(), &dmsg, (TickType_t) 0);
	}
}

void init_ble_serial(BLEService* pService)
{

	pCisoChar = pService->createCharacteristic(serialCisoUUID, serialCisoCharProps);
	sCiso2902.setNotifications(true);
	pCisoChar->addDescriptor(&sCiso2902);

	pCosiChar = pService->createCharacteristic(serialCosiUUID, serialCosiCharProps);
	pCosiChar->setCallbacks(&SerialCosiCallbacks);
	pCosiChar->addDescriptor(&sCosi2902);

}
