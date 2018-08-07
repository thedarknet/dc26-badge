#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "security.h"
#include "../lib/ble/BLESecurity.h"
#include "../lib/ble/BLEDevice.h"
#include "../lib/ssd1306.h"

#include "../dc26.h"
#include "../mcu_to_mcu.h"
#include "../esp_to_stm_generated.h"
#include "../stm_to_esp_generated.h"
#include "../display_handler.h"

const char *SECTAG = "SecurityCallbacks";

uint32_t MySecurity::onPassKeyRequest()
{
	ESP_LOGI(SECTAG, "PassKeyRequest ********************");
	this->success = false;
	// TODO
	return 123456;
}

void MySecurity::onPassKeyNotify(uint32_t pass_key)
{
	ESP_LOGI(SECTAG, "The passkey Notify number:%d", pass_key);
	this->success = false;
	return;
}


bool MySecurity::onConfirmPIN(uint32_t pass_key)
{
	unsigned int waited = 0;
	this->success = false;
	DisplayTask::DisplayMsg* dmsg = new DisplayTask::DisplayMsg();
	memset(dmsg->Msg, '0', sizeof(dmsg->Msg));
	dmsg->y = 40;
	dmsg->clearScreen = false;
	sprintf(dmsg->Msg, "PIN: %u", pass_key);
	ESP_LOGI(SECTAG, "onConfirmPin: %s", dmsg->Msg);
	xQueueSendFromISR(getDisplayTask().getQueueHandle(), &dmsg, (TickType_t) 0);

	// Client side doesn't like being interrupted like this, just have server confirm
	if (pBTTask->isActingClient)
		return true;
	else
		pBTTask->pScan->stop(); // potential race conditiong O_O

	// Send to STM, get back confirmation
	flatbuffers::FlatBufferBuilder fbb;
	flatbuffers::Offset<darknet7::ESPToSTM> of;
	auto c = darknet7::CreateBLESecurityConfirm(fbb);
	of = darknet7::CreateESPToSTM(fbb, 0, darknet7::ESPToSTMAny_BLESecurityConfirm, c.Union());
	darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
	getMCUToMCU().send(fbb);

	this->confirmed = false;
	// Wait for the confirmation to come through for about 15 seconds
	while ((this->confirmed == false) && (waited < 15))
	{
		ESP_LOGI(SECTAG, "waiting: %d\n", this->confirmed);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		waited++;
	}

	ESP_LOGI(SECTAG, "finished: %d\n", this->confirmed);
	return this->confirmed;
}

bool MySecurity::onSecurityRequest()
{
	ESP_LOGI(SECTAG, "On Security Request");
	this->success = false;
	return true;
}

void MySecurity::onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl)
{
	if (pBTTask->isActingClient)
	{
		if(auth_cmpl.success)
		{
			ESP_LOGI(SECTAG, "remote BD_ADDR:");
			esp_log_buffer_hex(SECTAG, auth_cmpl.bd_addr, sizeof(auth_cmpl.bd_addr));
			ESP_LOGI(SECTAG, "address type = %d", auth_cmpl.addr_type);
		}
		ESP_LOGI(SECTAG, "pair status = %s", auth_cmpl.success ? "success" : "fail");
	}
	else
	{
		ESP_LOGI(SECTAG, "Starting BLE work!");
		if(auth_cmpl.success)
		{
			uint16_t length;
			esp_ble_gap_get_whitelist_size(&length);
			ESP_LOGI(SECTAG, "size: %d", length);
		}
		flatbuffers::FlatBufferBuilder fbb;
		auto con = darknet7::CreateBLEConnected(fbb, auth_cmpl.success, pBTTask->isActingClient);
		flatbuffers::Offset<darknet7::ESPToSTM> of = darknet7::CreateESPToSTM(fbb, 0, 
			darknet7::ESPToSTMAny_BLEConnected, con.Union());
		darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
		getMCUToMCU().send(fbb);
	}

	this->success = true;
	vTaskDelay(3000 / portTICK_PERIOD_MS);
}
