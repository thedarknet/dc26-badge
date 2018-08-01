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
	// TODO
	return 123456;
}

void MySecurity::onPassKeyNotify(uint32_t pass_key)
{
	ESP_LOGI(SECTAG, "The passkey Notify number:%d", pass_key);
	return;
}


bool MySecurity::onConfirmPIN(uint32_t pass_key)
{
	unsigned int waited = 0;
	DisplayTask::DisplayMsg* dmsg = new DisplayTask::DisplayMsg();
	memset(dmsg->Msg, '0', sizeof(dmsg->Msg));

	sprintf(dmsg->Msg, "%u", pass_key);
	ESP_LOGI(SECTAG, "onConfirmPin: %s", dmsg->Msg);
	xQueueSendFromISR(getDisplayTask().getQueueHandle(), &dmsg, (TickType_t) 0);
	
	if (pBTTask->isActingClient)
		return true;

	this->confirmed = false;

	// TODO: Send to STM, get back confirmation
	flatbuffers::FlatBufferBuilder fbb;
	flatbuffers::Offset<darknet7::ESPToSTM> of;
	auto infect = darknet7::CreateBLESecurityConfirm(fbb);
	of = darknet7::CreateESPToSTM(fbb, 0, darknet7::ESPToSTMAny_BLESecurityConfirm, infect.Union());
	darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
	getMCUToMCU().send(fbb);
	

	// Wait for the confirmation to come through
	while ((this->confirmed == false) && (waited < 15))
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		waited++;
	}
	printf("Client Confirmed? %d\n", this->confirmed);
	return this->confirmed;
}

bool MySecurity::onSecurityRequest()
{
	ESP_LOGI(SECTAG, "On Security Request");
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
		// TODO: Send Generic Response
		darknet7::RESPONSE_SUCCESS res = darknet7::RESPONSE_SUCCESS_True;
		flatbuffers::FlatBufferBuilder fbb;
		flatbuffers::Offset<darknet7::ESPToSTM> of;
		auto infect = darknet7::CreateGenericResponse(fbb, res);
		of = darknet7::CreateESPToSTM(fbb, this->msgInstanceID, 
			darknet7::ESPToSTMAny_GenericResponse, infect.Union());
		darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
		getMCUToMCU().send(fbb);
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
	}
}
