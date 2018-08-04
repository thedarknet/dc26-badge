#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "services.h"
#include "pairing.h"
#include "../lib/ble/BLEDevice.h"

#include "../dc26.h"
#include "../mcu_to_mcu.h"
#include "../stm_to_esp_generated.h"
#include "../esp_to_stm_generated.h"

const char *PAIR_CLIENT_TAG = "BTPairingClient";
const char *PAIR_SVR_TAG = "BTPairingServer";


void UartClientCallbacks::onConnect(BLEClient* client)
{
	ESP_LOGI(PAIR_CLIENT_TAG, "connected to server");
	isConnected = true;
	pClient = client;
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
	isConnected = false;
}

int aliceMessage = 1;
void UartCosiCharCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
	flatbuffers::FlatBufferBuilder fbb;
	flatbuffers::Offset<darknet7::ESPToSTM> of;
	std::string rxValue = pCharacteristic->getValue();
	unsigned int length = rxValue.length();
	const char* pData = rxValue.c_str();
	if (pData[0] == '0')
	{
		// clear mesgbuf2 and start from beginning
		midx2 = 0;
		memset(mesgbuf2, 0, 200);
		memcpy(&mesgbuf2[midx2], &pData[1], length-1);
		midx2 += (length-1);
	}
	else if (pData[0] == '1')
	{
		// continue copying, don't send
		memcpy(&mesgbuf2[midx2], &pData[1], length-1);
		midx2 += (length-1);
	}
	else if (pData[0] == '2')
	{
		memcpy(&mesgbuf2[midx2], &pData[1], length-1);
		midx2 += (length-1);
		// send MessageFromBob STM
		mesgbuf2[midx2] = 0x0;

		printf("Server Received:");
		for (int i = 0; i < midx2; i++)
			printf("%02X", mesgbuf2[i]); // TODO: print stuff
		printf("\n");


		auto sdata = fbb.CreateString(mesgbuf2, midx2);
		auto sendData = darknet7::CreateBLEMessageFromDevice(fbb, sdata);
		of = darknet7::CreateESPToSTM(fbb, 0,
			darknet7::ESPToSTMAny_BLEMessageFromDevice, sendData.Union());
		darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
		getMCUToMCU().send(fbb);
		aliceMessage += 1;
	}
}

void UartServerCallbacks::onConnect(BLEServer* server)
{
	if (!pBTTask->isActingClient)
	{
		ESP_LOGI(PAIR_SVR_TAG, "connection received");
		isConnected = true;
		pBTTask->isActingServer = true;
		aliceMessage = 1;
	}
}

void UartServerCallbacks::onDisconnect(BLEServer* server)
{
	if (this->isConnected)
	{
		ESP_LOGI(PAIR_SVR_TAG, "connection dropped");
		isConnected = false;
	}
	pBTTask->isActingServer = false;
}
