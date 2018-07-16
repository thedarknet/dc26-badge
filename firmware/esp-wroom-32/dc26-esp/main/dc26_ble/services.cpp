#include "esp_system.h"
#include "esp_log.h"
#include "ble.h"
#include "services.h"

void setupCharacteristics(BLEService* pService)
{
	// Gulotachi feeding
	pService->createCharacteristic(foodUUID,   foodCharProps);
	pService->createCharacteristic(waterUUID,  waterCharProps);
	pService->createCharacteristic(careUUID,   careCharProps);

	// NPC's
	pService->createCharacteristic(ratUUID,    ratCharProps);
	pService->createCharacteristic(catUUID,    catCharProps);
	pService->createCharacteristic(fleaUUID,   fleaCharProps);
	pService->createCharacteristic(pmaskUUID,  pmaskCharProps);

	// Infection
	pService->createCharacteristic(toxUUID,    toxCharProps);
	pService->createCharacteristic(polUUID,    polCharProps);
	pService->createCharacteristic(plaUUID,    plaCharProps);
	pService->createCharacteristic(tetUUID,    tetCharProps);

	// The Gourry Special
	pService->createCharacteristic(boopedUUID, boopCharProps);
	pService->createCharacteristic(doodleUUID, doodleCharProps);
	pService->createCharacteristic(dingleUUID, dingleCharProps);

	// User ID
	pService->createCharacteristic(idUUID,     idCharProps);
	return;
}

void setCharacteristicValue(BLEService* pService, BLEUUID uuid, uint8_t* data, size_t size)
{
	BLECharacteristic* pChar = nullptr;
	pChar = pService->getCharacteristic(uuid);
	if (pChar)
		pChar->setValue(data, size);
	return;
}
