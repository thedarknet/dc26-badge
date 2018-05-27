#include <stdio.h>
#include <string.h>
#include "dc26_ble_proto.h"

#include "lib/ble/BLEDevice.h"

BLEServer *g_pServer;
BLEAdvertising *g_pAdvertising;

void dc26_bt_init()
{
    
    BLEDevice::init("DCDN BLE Server");
    g_pServer = BLEDevice::createServer();
    g_pAdvertising = g_pServer->getAdvertising();
    
    BLEAdvertisementData adv_data;
    adv_data.setAppearance(0x26DC);
    adv_data.setFlags(0x6);
    adv_data.setName("GOURRY!!!!");
    adv_data.setManufacturerData("EGGPLANTS");
    
    g_pAdvertising->setAdvertisementData(adv_data);
    g_pServer->startAdvertising();
}

