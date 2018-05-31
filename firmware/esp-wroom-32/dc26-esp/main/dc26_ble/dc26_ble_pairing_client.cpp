#include <stdio.h>
#include <string.h>
#include "dc26_ble.h"
#include "dc26_ble_pairing_client.h"
#include "dc26_ble_pairing_server.h"
#include "../lib/ble/BLEDevice.h"

const char *PAIR_CLIENT_TAG = "BTPairingClient";


void UartClientCallbacks::onConnect(BLEClient* client)
{
	ESP_LOGI(PAIR_CLIENT_TAG, "connected to server");
	pClient = client;
}

void UartClientCallbacks::afterConnect()
{
	ESP_LOGI(PAIR_CLIENT_TAG, "connected to server");
	
    // Obtain a reference to the service we are after in the remote BLE server.
	ESP_LOGI(PAIR_CLIENT_TAG, "sleep 500");
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	ESP_LOGI(PAIR_CLIENT_TAG, "get client");
    pRemoteService = pClient->getService(uartServiceUUID);
	ESP_LOGI(PAIR_CLIENT_TAG, "client got");
    if (pRemoteService == nullptr) {
		printf("service finding error\n");
		return;
    }
	ESP_LOGI(PAIR_CLIENT_TAG, "got service");

	// get the characteristic for the CLIENT to RECEIVE (TX)
	vTaskDelay(1800 / portTICK_PERIOD_MS);
    pRxChar = pRemoteService->getCharacteristic(uartTxUUID);
    if (pRxChar == nullptr) {
		printf("first char finding error\n");
		return;
    }
	
	ESP_LOGI(PAIR_CLIENT_TAG, "got characteristic one");
	
	// get the characteristic for the CLIENT to SEND (RX)
	vTaskDelay(1000 / portTICK_PERIOD_MS);
    pTxChar = pRemoteService->getCharacteristic(uartRxUUID);
    if (pTxChar == nullptr) {
		printf("second char finding error\n");
		return;
    }
	ESP_LOGI(PAIR_CLIENT_TAG, "got characteristic two");
	
	vTaskDelay(1000 / portTICK_PERIOD_MS);

	ESP_LOGI(PAIR_CLIENT_TAG, "setup complete");
}

void UartClientCallbacks::onDisconnect(BLEClient* client)
{
	ESP_LOGI(PAIR_CLIENT_TAG, "disconnected");
}
