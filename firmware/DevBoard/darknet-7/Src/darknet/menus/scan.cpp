/*
 * scan.cpp
 *
 *  Created on: May 30, 2018
 *      Author: cmdc0de
 */



#include "scan.h"
#include "../darknet7.h"
#include "../messaging/stm_to_esp_generated.h"
#include "menu_state.h"

using cmdc0de::RGBColor;
using cmdc0de::ErrorType;
using cmdc0de::StateBase;


Scan::Scan() : Darknet7BaseState(), NPCOnly(false), DisplayList("WiFi:", Items, 0, 0, 160, 128, 0, (sizeof(Items) / sizeof(Items[0]))), ESPRequestID(0), InternalState(NONE)  {

}

Scan::~Scan()
{

}

void Scan::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::WiFiScanResults> *mevt) {
	if(mevt->RequestID==this->ESPRequestID) {
		InternalState = DISPLAY_DATA;

		MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());

		for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
			Items[i].text = &ListBuffer[i][0];
			Items[i].id = i;
			Items[i].setShouldScroll();
		}

		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		DarkNet7::get().getGUI().drawList(&DisplayList);
	}
}

ErrorType Scan::onInit() {
	InternalState = FETCHING_DATA;
	flatbuffers::FlatBufferBuilder fbb;
	darknet7::WiFiScanFilter filter = this->isNPCOnly()?darknet7::WiFiScanFilter_NPC:darknet7::WiFiScanFilter_ALL;

	auto r = darknet7::CreateWiFiScan(fbb,filter);
	ESPRequestID = DarkNet7::get().nextSeq();
	auto e = darknet7::CreateSTMToESPRequest(fbb,ESPRequestID,darknet7::STMToESPAny_ESPRequest, r.Union());
	darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	const MSGEvent<darknet7::WiFiScanResults> *si = 0;
	MCUToMCU::get().getBus().addListener(this,si,&MCUToMCU::get());
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);

	if(isNPCOnly()) {
		DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Scanning for DarkNet NPCs",RGBColor::BLUE);
	} else {
		DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Scanning for APs",RGBColor::BLUE);
	}

	MCUToMCU::get().send(fbb);
	return ErrorType();

}

StateBase::ReturnStateContext Scan::onRun() {
	StateBase *nextState = this;
	switch(InternalState) {
	case FETCHING_DATA:
		if(this->getTimesRunCalledSinceLastReset()>200) {
			nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::NO_DATA_FROM_ESP,2000);
		}
		break;
	case DISPLAY_DATA:
		if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
			nextState = DarkNet7::get().getDisplayMenuState();
		}
		break;
	case NONE:
		break;
	}
	return ReturnStateContext(nextState);
}

ErrorType Scan::onShutdown()
{
	return ErrorType();
}






