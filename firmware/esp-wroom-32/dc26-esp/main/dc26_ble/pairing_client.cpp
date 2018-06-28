#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "services.h"
#include "pairing_client.h"
#include "pairing_server.h"
#include "../lib/ble/BLEDevice.h"

const char *PAIR_CLIENT_TAG = "BTPairingClient";


void UartClientCallbacks::onConnect(BLEClient* client)
{
	ESP_LOGI(PAIR_CLIENT_TAG, "connected to server");
	connected = true;
	pClient = client;
	vTaskDelay(500 / portTICK_PERIOD_MS);
}

void UartClientCallbacks::afterConnect()
{
	// Obtain a reference to the service we are after in the remote BLE server.
	pRemoteService = pClient->getService(uartServiceUUID);
	if (pRemoteService == nullptr) {
		ESP_LOGE(PAIR_CLIENT_TAG, "service finding error");
		return;
	}

	// get the characteristic for the CLIENT to RECEIVE (TX)
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	pRxChar = pRemoteService->getCharacteristic(uartCisoUUID);
	if (pRxChar == nullptr) {
		ESP_LOGE(PAIR_CLIENT_TAG, "first char finding error");
		return;
	}
	
	// get the characteristic for the CLIENT to SEND (RX)
	pTxChar = pRemoteService->getCharacteristic(uartCosiUUID);
	if (pTxChar == nullptr) {
		ESP_LOGE(PAIR_CLIENT_TAG, "second char finding error");
		return;
	}
	
	ESP_LOGI(PAIR_CLIENT_TAG, "Setup Complete");
}

void UartClientCallbacks::onDisconnect(BLEClient* client)
{
	ESP_LOGI(PAIR_CLIENT_TAG, "disconnected");
	pBTTask->isActingClient = false;
	connected = false;
}
