#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "pairing_server.h"
#include "../lib/ble/BLEDevice.h"

const char *PAIR_SVR_TAG = "BTPairingServer";

void UartRxCharCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
	std::string rxValue = pCharacteristic->getValue();
	if (rxValue.length() > 0)
	{	
		const char *msgOrig = pCharacteristic->getValue().c_str();
		uint32_t len = strlen(msgOrig);
		char * msg = (char*)malloc(len+1);
		memcpy(msg, msgOrig, len);
		msg[len] = '\0';
		xQueueSendFromISR(CallbackQueueHandle, &msg, NULL);
	}
};

void UartServerCallbacks::onConnect(BLEServer* server)
{
	ESP_LOGI(PAIR_SVR_TAG, "connection received");
	isConnected = true;
}

void UartServerCallbacks::onDisconnect(BLEServer* server)
{
	ESP_LOGI(PAIR_SVR_TAG, "connection dropped");
	isConnected = false;
}
