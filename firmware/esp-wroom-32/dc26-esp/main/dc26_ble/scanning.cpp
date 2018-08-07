#include <stdio.h>
#include <string.h>
#include "../dc26.h"
#include "ble.h"
#include "scanning.h"
#include <map>
#include "../common_generated.h"
#include "../lib/ble/BLEDevice.h"
#include "../display_handler.h"

void MyScanCallbacks::reset(void)
{
	// reset any old settings from prior scans
	// default to NFECT scan since that's when we'll usually call this
	this->filter = darknet7::BLEDeviceFilter_INFECT;
	this->results.clear();
	this->min_RSSI = 0;
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

unsigned int MyScanCallbacks::getNumberOfResults(void)
{
	return this->results.size();
}

std::map<std::string, std::string> MyScanCallbacks::getResults(void)
{
	return this->results;
}

void MyScanCallbacks::setFilter(uint8_t val)
{
	this->filter = val;
}

static int i = 0;
static char scanMsg[] = "............";
void MyScanCallbacks::onResult(BLEAdvertisedDevice advertisedDevice)
{
	DisplayTask::DisplayMsg* dmsg = new DisplayTask::DisplayMsg();
	memset(dmsg->Msg, '0', sizeof(dmsg->Msg));
	dmsg->y = 40;
	dmsg->clearScreen = false;
	memcpy(dmsg->Msg, scanMsg, i);
	xQueueSendFromISR(getDisplayTask().getQueueHandle(), &dmsg, (TickType_t) 0);
	i %= sizeof(scanMsg);
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

			// if this is an infection only scan, don't record anything
			if (this->filter == darknet7::BLEDeviceFilter_INFECT)
				return;

			int rssi = 0;
			if (advertisedDevice.haveRSSI())
				rssi = advertisedDevice.getRSSI();
			else
				return;

			// TODO: Test the >8 results code
			// FIXME: this code is the worst, for the love of god fix it.
			// add only the 8 higest power devices to final message
			if (this->results.size() == 8 && (rssi > this->min_RSSI))
			{
				std::string smallest;
				for (const auto &p : this->RSSIs)
					smallest = (p.second == this->min_RSSI) ? p.first : smallest;
				// Delete the smallest entry, put in the new entry
				this->results.erase(smallest);
				this->RSSIs.erase(smallest);
				std::string name = advertisedDevice.getName();
				std::string address = advertisedDevice.getAddress().toString();
				this->results[address] = name;
				this->RSSIs[address] = rssi;
				// Find the new smallest entry
				int small_rssi = 0x7FFFFFFF;
				for (const auto &p : this->RSSIs)
					small_rssi = (p.second < small_rssi) ? p.second : small_rssi;
				this->min_RSSI = small_rssi;
			}
			else
			{
				DisplayTask::DisplayMsg* dmsg = new DisplayTask::DisplayMsg();
				memset(dmsg->Msg, '0', sizeof(dmsg->Msg));
				sprintf(dmsg->Msg, "Scanning");
				xQueueSendFromISR(getDisplayTask().getQueueHandle(), &dmsg, (TickType_t) 0);

				std::string name = advertisedDevice.getName();
				std::string address = advertisedDevice.getAddress().toString();
				this->results[address] = name;
			}
		}
	}
}
