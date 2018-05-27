#include <stdio.h>
#include <string.h>
#include "dc26_ble_proto.h"
#include "lib/ble/BLEDevice.h"

BLEServer *g_pServer;
BLEAdvertising *g_pAdvertising;
bool g_b2b_advertising_enabled = false;

std::string g_adv_name = "";
std::string g_adv_manufacturer = "";

static void startB2BAdvertising()
{
    if (!g_b2b_advertising_enabled)
    {
        g_pAdvertising->start();
        printf("DC26 Badge-to-Badge advertising started\n");
        g_b2b_advertising_enabled = true;
    }
    else
    {
        printf("DC26 Badge-to-Badget advertising already started");
    }
}

static void stopB2BAdvertising()
{
    if (g_b2b_advertising_enabled)
    {
        g_pAdvertising->stop();
        printf("DC26 Badge-to-Badge advertising stopped\n");
        g_b2b_advertising_enabled = false;
    }
    else
    {
        printf("DC26 Badge-to-Badget advertising already started");
    }
}

static void setB2BAdvData(std::string new_name, std::string new_man_data)
{
    // TODO: Check length < 10 characters
    g_adv_name = new_name;
    g_adv_manufacturer = "DN" + new_man_data;
    
    // Stop advertising, re-set data, return advertising to original state
    bool original_state = g_b2b_advertising_enabled;
    if (original_state)
    {
        stopB2BAdvertising();
    }
    
    // Setup data
    BLEAdvertisementData adv_data;
    adv_data.setAppearance(0x26DC);
    adv_data.setFlags(0x6);
    adv_data.setName(g_adv_name);
    adv_data.setManufacturerData(g_adv_manufacturer);
    g_pAdvertising->setAdvertisementData(adv_data);

    // Restart if we were already advertising
    if (original_state)
    {
        startB2BAdvertising();
    }
}

static void btCmdTask(void *)
{
    BTCmd cmd = BT_CMD_START_B2B;
    while (1)
    {
        vTaskDelay(20000 / portTICK_PERIOD_MS);
        // TODO: Check queue for command
        switch(cmd)
        {
            case BT_CMD_START_B2B:
                startB2BAdvertising();
                cmd = BT_CMD_SET_B2B_ADV_DATA;
                break;
            case BT_CMD_STOP_B2B:
                stopB2BAdvertising();
                cmd = BT_CMD_UNK;
                break;
            case BT_CMD_SET_B2B_ADV_DATA:
                setB2BAdvData("GOURRY!!!!", "INFECT!");
                cmd = BT_CMD_UNK;
                break;
            default:
                printf("EGGPLANT: waiting for an updated name now: %d\n", cmd);
                break;
        }
    }
}

void dc26_bt_init() 
{
    BLEDevice::init("DCDN BLE Server");
    g_pServer = BLEDevice::createServer();
    g_pAdvertising = g_pServer->getAdvertising();
    
    setB2BAdvData("DN 12345", "EGGPLANT");

    // Create the comms task from STM->Main_App->BT
    xTaskCreate(btCmdTask, "bt_cmd_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    //BLEAdvertisementData adv_data;
    //adv_data.setAppearance(0x26DC);
    //adv_data.setFlags(0x6);
    //adv_data.setName("GOURRY!!!!");
    //adv_data.setManufacturerData("EGGPLANTS");
    
    //g_pAdvertising->setAdvertisementData(adv_data);
    //g_pServer->startAdvertising();
}

