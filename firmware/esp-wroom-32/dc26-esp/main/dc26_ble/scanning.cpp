#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "scanning.h"
#include "../lib/ble/BLEDevice.h"


void MyScanCallbacks::onResult(BLEAdvertisedDevice advertisedDevice)
{
	//printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
	if (advertisedDevice.haveAppearance() &&
		advertisedDevice.getAppearance() == 0x26DC)
	{
		// TODO: Report this back to the STM32
		printf("Found DC26 Device: %s \n", advertisedDevice.toString().c_str());
		//if (advertisedDevice.haveServiceUUID() &&
		//	advertisedDevice.getServiceUUID().equals(pairServiceUUID))
		//{
			// Found the device we want, stop scan, save data, setup pairing
			//printf("Found UUID for DC26 Device\n"); 
			server_found = true;
			pServerAddress = new BLEAddress(advertisedDevice.getAddress());
		//}
	}
}
