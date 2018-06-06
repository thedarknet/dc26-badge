#include "../lib/ble/BLESecurity.h"
#include "../lib/ble/BLEDevice.h"
#include "ble.h"

class MySecurity : public BLESecurityCallbacks 
{
public:
	BluetoothTask *pBTTask = nullptr;
public:
	virtual uint32_t onPassKeyRequest();
	virtual void onPassKeyNotify(uint32_t pass_key);
	virtual bool onConfirmPIN(uint32_t pass_key);
	virtual bool onSecurityRequest();
	virtual void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl);
};
