#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "services.h"
#include "pairing.h"
#include "../lib/ble/BLEDevice.h"

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




void UartCosiCharCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
	std::string rxValue = pCharacteristic->getValue();
	if (rxValue.length() > 0)
	{	
		const char *msgOrig = rxValue.c_str();
		printf("Message get: %s\n", msgOrig);
		/*
		uint32_t len = strlen(msgOrig);
		char * msg = (char*)malloc(len+1);
		memcpy(msg, msgOrig, len);
		msg[len] = '\0';
		xQueueSendFromISR(CallbackQueueHandle, &msg, NULL);
		*/
		flatbuffers::FlatBufferBuilder fbb;
		uint8_t *data = nullptr;
		MCUToMCUTask::Message* m;
		flatbuffers::Offset<darknet7::STMToESPRequest> of;
		flatbuffers::uoffset_t size;

		auto sdata = fbb.CreateString("Poodle");
		auto sendData = darknet7::CreateBLESendDataToDevice(fbb, sdata);
		of = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendDataToDevice,
			sendData.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb, of);
		size = fbb.GetSize();
		data = fbb.GetBufferPointer();
		m = new MCUToMCUTask::Message();
		m->set(size, 0, data);
		xQueueSendFromISR(pBTTask->getQueueHandle(), &m, (TickType_t) 0);
	}
}

void UartServerCallbacks::onConnect(BLEServer* server)
{
	ESP_LOGI(PAIR_SVR_TAG, "connection received");
	isConnected = true;
	pBTTask->isActingServer = true;
}

void UartServerCallbacks::onDisconnect(BLEServer* server)
{
	ESP_LOGI(PAIR_SVR_TAG, "connection dropped");
	isConnected = false;
	pBTTask->isActingServer = false;
}
