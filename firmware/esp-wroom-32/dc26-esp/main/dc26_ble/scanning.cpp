#include <stdio.h>
#include <string.h>
#include "ble.h"
#include "scanning.h"
#include <map>
#include "../common_generated.h"
#include "../lib/ble/BLEDevice.h"


void MyScanCallbacks::reset(void)
{
	// reset any old settings from prior scans
	// default to the INFECT scan since that's when we'll usually be calling this
	this->filter = darknet7::BLEDeviceFilter_INFECT;
	this->results.clear();
}

uint16_t MyScanCallbacks::getExposures(void)
{
	uint16_t retval = this->exposures;
	this->exposures = 0x0000;
	return retval;
}

uint16_t MyScanCallbacks::getCures(void)
{
	uint16_t retval = this->cures;
	this->cures = 0x0000;
	return retval;
}

std::map<std::string, std::string> MyScanCallbacks::getResults(void)
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
		advertisedDevice.getAppearance() == 0x26DC)
	{
		// filter out only the requested devices (Badge or NPC)
		if (this->filter == darknet7::BLEDeviceFilter_ALL ||
			this->filter == darknet7::BLEDeviceFilter_INFECT ||
			(advertisedDevice.haveManufacturerData() && 
			((advertisedDevice.getManufacturerData()[2] == this->filter) != 0)))
		{
			// get manufacturer data for infection vectors
			std::string manData = advertisedDevice.getManufacturerData();
			this->exposures |= ((manData[3] << 8) | manData[4]);
			this->cures |= ((manData[5] << 8) | manData[6]);

			// add result to final message
			// if this is an infection only scan, don't record anything
			if (this->filter != darknet7::BLEDeviceFilter_INFECT)
			{ 
				std::string name = advertisedDevice.getName();
				std::string address = advertisedDevice.getAddress().toString();
				this->results[address] = name;
			}
		}
	}
}
