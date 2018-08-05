#ifndef MY_SECURITY
#define MY_SECURITY

#include "ble.h"
#include "../lib/ble/BLESecurity.h"
#include "../lib/ble/BLEDevice.h"

class MySecurity : public BLESecurityCallbacks 
{
public:
	BluetoothTask *pBTTask = nullptr;
	bool confirmed = false;
	uint32_t msgInstanceID = 0;
	bool success = false;

	virtual uint32_t onPassKeyRequest();
	virtual void onPassKeyNotify(uint32_t pass_key);
	virtual bool onConfirmPIN(uint32_t pass_key);
	virtual bool onSecurityRequest();
	virtual void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl);
};

#endif // MY_SECURITY
