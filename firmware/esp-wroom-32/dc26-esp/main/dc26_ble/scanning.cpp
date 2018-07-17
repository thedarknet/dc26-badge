#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "scanning.h"
#include "../common_generated.h"
#include "../lib/ble/BLEDevice.h"


void MyScanCallbacks::reset(void)
{
	// reset any old settings from prior scans
	this->filter = darknet7::BLEDeviceFilter_NONE;
	//this->infections = 0x0;
	//this->cures = 0x0;
	// TODO: clear results map
	// TODO: deallocate the map memory and reallocate an empty one
}

uint16_t MyScanCallbacks::getInfections(void)
{
	return 0; // TODO this->infections;
}

uint16_t MyScanCallbacks::getCures(void)
{
	return 0; // TODO this->cures;
}

void MyScanCallbacks::getResults(void) // TODO: Return value
{
	return; // TODO this->results;
}

void MyScanCallbacks::setFilter(uint8_t val)
{
	this->filter = val;
}

void MyScanCallbacks::onResult(BLEAdvertisedDevice advertisedDevice)
{
	// filter out DC26 devices only
	if (advertisedDevice.haveAppearance() &&
		advertisedDevice.getAppearance() == 0x26DC) // TODO:  && 
		//advertisedDevice.haveManufacturerData() && 
		//((advertisedDevice.getManufacturerData() & this->filter) != 0))
	{
		// get manufacturer data for infection vectors
		std::string manData = advertisedDevice.getManufacturerData();
		// TODO: parse manData for infection/cure information
		// ... this->infections |= infection_data;
		// ... this->cures |= cure_data;

		// add result to final message
		std::string name = advertisedDevice.getName();
		std::string address = advertisedDevice.getAddress().toString();
		// TODO: build results map
		// TODO: Serialize into ESPToSTM Message?
	}
}
