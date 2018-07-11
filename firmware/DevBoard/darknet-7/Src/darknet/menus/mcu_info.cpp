/*
 * mcu_info.cpp
 *
 *  Created on: May 30, 2018
 *      Author: dcomes
 */

#include "mcu_info.h"
#include "../darknet7.h"
#include "../messaging/stm_to_esp_generated.h"
#include "../messaging/esp_to_stm_generated.h"
#include "../mcu_to_mcu.h"
#include "menu_state.h"

using cmdc0de::RGBColor;
using cmdc0de::ErrorType;
using cmdc0de::StateBase;


MCUInfoState::MCUInfoState() : Darknet7BaseState(),
	BadgeInfoList("MCU Info:", Items, 0, 0, 128, 160, 0, (sizeof(Items) / sizeof(Items[0])))
	, ESPRequestID(0), InternalState(NONE) {

}

MCUInfoState::~MCUInfoState()
{

}

void MCUInfoState::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::ESPSystemInfo> *mevt) {
	if(mevt->RequestID==this->ESPRequestID) {
		InternalState = DISPLAY_DATA;
		sprintf(&ListBuffer[0][0], "ESP Cores: %d", (int)mevt->InnerMsg->cores());
		sprintf(&ListBuffer[1][0], "ESP Features: %d", (int)mevt->InnerMsg->features());
		sprintf(&ListBuffer[2][0], "ESP HeapSize: %lu", mevt->InnerMsg->heapSize());
		sprintf(&ListBuffer[3][0], "ESP idf: %s", mevt->InnerMsg->idf_version()->c_str());
		sprintf(&ListBuffer[4][0], "ESP Model: %ld", mevt->InnerMsg->model());
		sprintf(&ListBuffer[5][0], "ESP Revision: %ld", mevt->InnerMsg->revision());
		MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());

		for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
			Items[i].text = &ListBuffer[i][0];
			Items[i].id = i;
			Items[i].setShouldScroll();
		}

		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		DarkNet7::get().getGUI().drawList(&BadgeInfoList);
	}
}

ErrorType MCUInfoState::onInit() {
	InternalState = FETCHING_DATA;
	flatbuffers::FlatBufferBuilder fbb;
	auto r = darknet7::CreateESPRequest(fbb,darknet7::ESPRequestType::ESPRequestType_SYSTEM_INFO);
	ESPRequestID = DarkNet7::get().nextSeq();
	auto e = darknet7::CreateSTMToESPRequest(fbb,ESPRequestID,darknet7::STMToESPAny_ESPRequest, r.Union());
	darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	const MSGEvent<darknet7::ESPSystemInfo> *si = 0;
	MCUToMCU::get().getBus().addListener(this,si,&MCUToMCU::get());
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);

	DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Fetching data from ESP",RGBColor::BLUE);

	MCUToMCU::get().send(fbb);
	return ErrorType();

}

StateBase::ReturnStateContext MCUInfoState::onRun() {
	StateBase *nextState = this;
	switch(InternalState) {
	case FETCHING_DATA:
		if(this->getTimesRunCalledSinceLastReset()>2000) {
			nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),
					(const char *)"No info from ESP",2000);
		}
		break;
	case DISPLAY_DATA:
		if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
			nextState = DarkNet7::get().getDisplayMenuState();
		}
		break;
	case NONE:
		break;
	}
	return ReturnStateContext(nextState);
}

ErrorType MCUInfoState::onShutdown() {
	return ErrorType();
}




