#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "security.h"
#include "../lib/ble/BLESecurity.h"
#include "../lib/ble/BLEDevice.h"

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
	ESP_LOGI(SECTAG, "onConfirmPin: %d", pass_key);
	// TODO: Send to STM, get back confirmation
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	return true;
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
