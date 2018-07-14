#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "scanning.h"
#include "../lib/ble/BLEDevice.h"


void reset(void)
{
	// reset any old settings from prior scans
	this->filter = DEVICE_FILTER_NONE;
	this->infections = 0x0;
	this->cures = 0x0;
	// TODO: clear results map
	// TODO: deallocate the map memory and reallocate an empty one
}

uint16_t getInfections(void)
{
	return this->infections;
}

uint16_t getCures(void)
{
	return this->cures;
}

void getResults(void) // TODO: Return value
{
	return this->results;
}

void MyScanCallbacks::setFilter(uint8_t val)
{
	this->filter = val;
}

void MyScanCallbacks::onResult(BLEAdvertisedDevice advertisedDevice)
{
	// filter out DC26 devices only
	if (advertisedDevice.haveAppearance() &&
		advertisedDevice.getAppearance() == 0x26DC && 
		advertisedDevice.haveManufacturerData() && 
		((advertiedDevice.getManufacturerData() & this->filter) != 0))
	{
		// get manufacturer data for infection vectors
		std::string manData = advertisedDevice.getManufacturerData();
		// TODO: parse manData for infection/cure information
		// ... this->infections |= infection_data;
		// ... this->cures |= cure_data;

		// add result to final message
		std::string name = advertisedDevice.getName()
		std::string address = advertisedDevice.getAddress()
		// TODO: build results map
		// TODO: Serialize into ESPToSTM Message?
	}
}
