#include "../lib/ble/BLESecurity.h"
#include "../lib/ble/BLEDevice.h"

const char *SECTAG = "SecurityCallbacks";

class MySecurity : public BLESecurityCallbacks 
{
public:
	virtual uint32_t onPassKeyRequest()
	{
		ESP_LOGI(SECTAG, "PassKeyRequest");
		return 123456;
	}

	virtual void onPassKeyNotify(uint32_t pass_key)
	{
		ESP_LOGI(SECTAG, "The passkey Notify number:%d", pass_key);
	}
	
	virtual bool onConfirmPIN(uint32_t pass_key)
	{
		ESP_LOGI(SECTAG, "The passkey YES/NO number:%d", pass_key);
		vTaskDelay(5000);
		return true;
	}

	virtual bool onSecurityRequest()
	{
		ESP_LOGI(SECTAG, "SecurityRequest");
		return true;
	}

	virtual void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
	{
		ESP_LOGI(SECTAG, "Starting BLE work!");
	}
};
