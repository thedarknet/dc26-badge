#include <stdio.h>
#include <string.h>
#include "dc26_ble.h"
#include "dc26_ble_pairing.h"
#include "../lib/ble/BLEDevice.h"

const char *LOGTAG = "BTPairing";

void MyServerCallbacks::onConnect(BLEServer* server)
{
	ESP_LOGI(LOGTAG, "SERVER CONNECTION RECEIVED");
	// Reset the pairing state
	pPairingServerTask = new PairingServerTask("PairingServerTask");
	pPairingServerTask->pPairingCharacteristic = btTask->pPairingCharacteristic;

	// Save reference to this callbacks objects so we can trade info between 
	// the client and the server objects
	pPairingServerTask->pServerCallbacks = this;
		
	// Start pairing task
	pPairingServerTask->init();
	pPairingServerTask->start();

	connected = true;
}

void MyServerCallbacks::onDisconnect(BLEServer* server)
{
	ESP_LOGI(LOGTAG, "SERVER CONNECTION DROPPED");
	connected = false;

	// Reset the pairing state
	btTask->pPairingCharacteristic->setValue("PAIRING START");
	delete pPairingServerTask;
	pPairingServerTask = nullptr;
}


void MyClientCallbacks::onConnect(BLEClient *client)
{

}

void MyClientCallbacks::afterConnected(BLEClient *client)
{
	ESP_LOGI(LOGTAG, "CLIENT CONNECTION INITIATED");
	pPairingClientTask = new PairingClientTask("PairingClientTask");

	// TODO
	// Save reference to this callbacks object so we can trade info between
	// the client and the server
	pPairingClientTask->pClientCallbacks = this;

	vTaskDelay(500 / portTICK_PERIOD_MS);

	// Get the remote Service
	ESP_LOGI(LOGTAG, "CLIENT find remote service");
 	BLERemoteService* pRemoteService = client->getService(pairServiceUUID);
	if (pRemoteService == nullptr) {
		ESP_LOGI(LOGTAG, "Client failed to find remote service UUID");
		delete pPairingClientTask;
		pPairingClientTask = nullptr;
		return;
	}
	ESP_LOGI(LOGTAG, "Client Found remote service UUID\n");

	vTaskDelay(500 / portTICK_PERIOD_MS);

	// Get the remote characteristic in the service of the remote BLE server.
	ESP_LOGI(LOGTAG, "CLIENT find remote characteristic");
	pRemoteCharacteristic = pRemoteService->getCharacteristic(pairingCharUUID);
	if (pRemoteCharacteristic == nullptr) {
		ESP_LOGI(LOGTAG, "Client Failed to find remote characteristic UUID");
		delete pPairingClientTask;
		pPairingClientTask = nullptr;
		return;
	}
	ESP_LOGI(LOGTAG, "Client found remote characteristic");
	pPairingClientTask->pRemoteCharacteristic = pRemoteCharacteristic;

	ESP_LOGI(LOGTAG, "Client task starting");
	pPairingClientTask->init();
	pPairingClientTask->start();
	connected = true;
}



void MyClientCallbacks::onDisconnect(BLEClient *client)
{
	ESP_LOGI(LOGTAG, "CLIENT CONNECTION DROPPED");
	connected = false;

	// clean up
	if (pPairingClientTask != nullptr)
	{
		delete pPairingClientTask;
		pPairingClientTask = nullptr;
	}
}


/* Server Side Task */
void PairingServerTask::run(void * data)
{
	ESP_LOGI(LOGTAG, "SERVER TASK RUNNING");
//Demo TODO
	std::string msg;
	// wait for client to be connected
	while (!pServerCallbacks->pClientCallbacks->connected)
		delay(500 / portTICK_PERIOD_MS);
	
	ESP_LOGI(LOGTAG, "SERVER TASK SEES CLIENT CONNECTED");

	// Wait for ready message from client
	while (true)
	{
		msg = pPairingCharacteristic->getValue();
		if (msg.c_str()[0] != 0xff)
			delay(500 / portTICK_PERIOD_MS);
		else
			break;
	}
	
	ESP_LOGI(LOGTAG, "SERVER task sees client ready to receive");
	
	// set message to something
	delay(500 / portTICK_PERIOD_MS);
	pPairingCharacteristic->setValue("OKD-D-D-DoodleBug");
	
	ESP_LOGI(LOGTAG, "SERVER task doodlebugging");
	
	// wait for OK to terminate
	while (true)
	{
		msg = pPairingCharacteristic->getValue();
		if (msg.c_str()[0] != 0xff)
			delay(500 / portTICK_PERIOD_MS);
		else
			break;
	}
	
	ESP_LOGI(LOGTAG, "SERVER task sees client completed");

	//terminate	

//Real TODO
	// TODO: Timeout and force disconnect

	// TODO: Wait for notification from client task that target is connected

	// TODO: verify that the connections are between the same target

	// TODO: Push notifications containing the cryptographic key information
		// Stage 1:
			// receive cryptographic info from STM32
			// push our cryptographic info
		// Stage 2:
			// receive result of hashing from STM32
			// push result of hashing target crypto
		// Stage 3:
			// receive success or failing from STM32
			// Result 1: push notification of success
			// Result 2: push notification of failure
		// Stage 4: 
			// disconnect
	// die
}

bool PairingServerTask::init()
{
	pPairingCharacteristic->setValue("PAIRING SERVER START");
	// Setup PairingServer -> STM32 Queue
	// Purge PairingServer -> STM32 Queue
	// Send ready message to STM32
	return true;
}

PairingServerTask::~PairingServerTask()
{
	// TODO?
}

PairingServerTask::PairingServerTask(const std::string &tName,
										uint16_t stackSize,
										uint8_t p)
{
	// TODO?
}




/* Client Side Task */
void PairingClientTask::run(void * data)
{
	ESP_LOGI(LOGTAG, "CLIENT TASK RUNNING");
// Demo TODO
	// wait for server to connect
	while (!pClientCallbacks->pServerCallbacks->connected)
		delay(500 / portTICK_PERIOD_MS);
	
	ESP_LOGI(LOGTAG, "CLIENT task sees server is connected");
	
	// Register for remote characteristic updates
	// Set ready-to-receive state
	delay(500 / portTICK_PERIOD_MS);
	pRemoteCharacteristic->writeValue(0xFF, false);
	
	ESP_LOGI(LOGTAG, "CLIENT wrote value 0xFF");
	std::string msg; // = pRemoteCharacteristic->readValue();
	
	while ((msg = pRemoteCharacteristic->readValue()).c_str()[0] == 0xFF)
		delay(500 / portTICK_PERIOD_MS);
	
	printf("received %s from server\n", msg.c_str());
	
	// disconnect
	delay(500 / portTICK_PERIOD_MS);
	pRemoteCharacteristic->writeValue(0xFF, false);
	ESP_LOGI(LOGTAG, "CLIENT wrote value 0xFF");

// Real TODO
	// TODO: Timeout and force disconnect

	// TODO: wait for notification from server that target is connected

	// TODO: verify that the connections are between the same target

	// TODO
		// Stage 1:
			// Write ready
			// Receive cryptographic info
			// Send to STM32
		// Stage 2:
			// Write ready
			// Receive hashed result
			// Send to STM32
		// Stage 3:
			// Write ready
			// Receive Verification or failure
			// Send to STM32
		// Stage 4:
			// Write ready
			// disconnect
			// send finalize to STM32
	// die
}

bool PairingClientTask::init()
{

	// Register Notify Callback
	// Setup PairingClient -> STM32 queue
	// Purge PairingClient -> STM32 queue
	// Send ready message to STM32
	return true;
}

PairingClientTask::~PairingClientTask()
{
	// TODO?
}

PairingClientTask::PairingClientTask(const std::string &tName,
										uint16_t stackSize,
										uint8_t p)
{
	// TODO?
}
