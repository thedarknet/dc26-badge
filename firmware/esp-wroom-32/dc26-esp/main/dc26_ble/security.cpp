#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "security.h"
#include "../lib/ble/BLESecurity.h"
#include "../lib/ble/BLEDevice.h"

const char *CLTSECTAG = "ClientSecurityCallbacks";
const char *SVRSECTAG = "ServerSecurityCallbacks";

uint32_t MySecurity::onPassKeyRequest()
{
	uint32_t passkey = 0;
	if (pBTTask->isActingClient)
	{
		ESP_LOGI(CLTSECTAG, "PassKeyRequest");
		passkey = 123456;
		// TODO: randomize
	}
	else // is Server
	{
		ESP_LOGI(SVRSECTAG, "PassKeyRequest");
		passkey = 123456;
		// TODO: request input
	}
	return passkey;
}

void MySecurity::onPassKeyNotify(uint32_t pass_key)
{
	if (pBTTask->isActingClient)
	{
		ESP_LOGI(CLTSECTAG, "The passkey Notify number:%d", pass_key);
	}
	else
	{
		ESP_LOGI(SVRSECTAG, "The passkey Notify number:%d", pass_key);
	}
	return;
}

bool MySecurity::onConfirmPIN(uint32_t pass_key)
{
	bool retval = false;
	if (pBTTask->isActingClient)
	{
		ESP_LOGI(CLTSECTAG, "The passkey YES/NO number:%d", pass_key);
		vTaskDelay(5000);
		retval = true;
	}
	else
	{
		ESP_LOGI(SVRSECTAG, "onConfirmPin %d", pass_key);
		retval = true;
		// TODO: retval
	}
	return retval;
}

bool MySecurity::onSecurityRequest()
{
	bool retval = false;
	if (pBTTask->isActingClient)
	{
		ESP_LOGI(CLTSECTAG, "Security Request");
		retval = true;
	}
	else
	{
		ESP_LOGI(SVRSECTAG, "On Security Request");
		retval = true;
	}
	return retval;
}

void MySecurity::onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl)
{
	if (pBTTask->isActingClient)
	{
		if(auth_cmpl.success)
		{
			ESP_LOGI(CLTSECTAG, "remote BD_ADDR:");
			esp_log_buffer_hex(CLTSECTAG, auth_cmpl.bd_addr, sizeof(auth_cmpl.bd_addr));
			ESP_LOGI(CLTSECTAG, "address type = %d", auth_cmpl.addr_type);
		}
		ESP_LOGI(CLTSECTAG, "pair status = %s", auth_cmpl.success ? "success" : "fail");
	}
	else
	{
		ESP_LOGI(SVRSECTAG, "Starting BLE work!");
		if(auth_cmpl.success)
		{
			uint16_t length;
			esp_ble_gap_get_whitelist_size(&length);
			ESP_LOGI(SVRSECTAG, "size: %d", length);
		}
	}
}
