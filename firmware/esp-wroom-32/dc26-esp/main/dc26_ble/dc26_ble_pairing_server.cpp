#include <stdio.h>
#include <string.h>
#include "dc26_ble.h"
#include "dc26_ble_pairing_server.h"
#include "../lib/ble/BLEDevice.h"

const char *PAIR_SVR_TAG = "BTPairingServer";

void UartRxCharCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
	std::string rxValue = pCharacteristic->getValue();
	if (rxValue.length() > 0)
		ESP_LOGI(PAIR_SVR_TAG, "Received Value - %s", rxValue.c_str());
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
