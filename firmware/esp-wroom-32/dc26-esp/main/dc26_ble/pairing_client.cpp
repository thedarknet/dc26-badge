#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "pairing_client.h"
#include "pairing_server.h"
#include "../lib/ble/BLEDevice.h"

const char *PAIR_CLIENT_TAG = "BTPairingClient";


void UartClientCallbacks::onConnect(BLEClient* client)
{
	ESP_LOGI(PAIR_CLIENT_TAG, "connected to server");
	connected = true;
	pClient = client;
}

void UartClientCallbacks::afterConnect()
{
	ESP_LOGI(PAIR_CLIENT_TAG, "connected to server");
	
    // Obtain a reference to the service we are after in the remote BLE server.
	vTaskDelay(150 / portTICK_PERIOD_MS);
    pRemoteService = pClient->getService(uartServiceUUID);
    if (pRemoteService == nullptr) {
		printf("service finding error\n");
		return;
    }

	// get the characteristic for the CLIENT to RECEIVE (TX)
	vTaskDelay(150 / portTICK_PERIOD_MS);
    pRxChar = pRemoteService->getCharacteristic(uartTxUUID);
    if (pRxChar == nullptr) {
		printf("first char finding error\n");
		return;
    }
	
	// get the characteristic for the CLIENT to SEND (RX)
	vTaskDelay(150 / portTICK_PERIOD_MS);
    pTxChar = pRemoteService->getCharacteristic(uartRxUUID);
    if (pTxChar == nullptr) {
		printf("second char finding error\n");
		return;
    }
	
	vTaskDelay(150 / portTICK_PERIOD_MS);
}

void UartClientCallbacks::onDisconnect(BLEClient* client)
{
	ESP_LOGI(PAIR_CLIENT_TAG, "disconnected");
	connected = false;
}
