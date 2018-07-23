#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "security.h"
#include "../lib/ble/BLESecurity.h"
#include "../lib/ble/BLEDevice.h"
#include "../lib/ssd1306.h"

// TODO: remove these includes
#include "../mcu_to_mcu.h"
#include "../stm_to_esp_generated.h"


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
	char pass_str[11];
	memset(&pass_str, '0', 11);
	sprintf(pass_str, "%u", pass_key);

	ESP_LOGI(SECTAG, "onConfirmPin: %s", pass_str);
	SSD1306_Puts(pass_str, &Font_7x10, 1);
	// TODO: Send to STM, get back confirmation
	if (pBTTask->isActingClient)
	{
		pBTTask->isActingClient = false;
		return true;
	}

	this->confirmed = false;

	// FIXME: remove this message when moving to development badge
	if (!this->sent)
	{
		printf("sending PIN confirmation message\n");
		flatbuffers::FlatBufferBuilder fbb;
		uint8_t *data = nullptr;
		MCUToMCUTask::Message* m;
		flatbuffers::Offset<darknet7::STMToESPRequest> of;
		flatbuffers::uoffset_t size;

		auto confirm = darknet7::CreateBLESendPINConfirmation(fbb, true);
		of = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendPINConfirmation,
			confirm.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb, of);
		 size = fbb.GetSize();
		data = fbb.GetBufferPointer();
		m = new MCUToMCUTask::Message();
		m->set(size, 0, data);
		xQueueSend(pBTTask->getQueueHandle(), &m, (TickType_t) 0);
		this->sent = true;
	}

	// TODO: display the number (pass_key) on the screen and send a message to
	// the STM to confirm the number

	// Wait for the confirmation to come through
	while ((this->confirmed == false) && (waited < 10))
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		waited++;
	}
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
