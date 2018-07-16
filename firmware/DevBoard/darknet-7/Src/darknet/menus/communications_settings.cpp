/*
 * communications_settings.cpp
 *
 *  Created on: May 29, 2018
 *      Author: dcomes
 */


#include "communications_settings.h"
#include "../darknet7.h"
#include "../messaging/stm_to_esp_generated.h"
#include "../messaging/esp_to_stm_generated.h"
#include "gui_list_processor.h"

using cmdc0de::ErrorType;
using cmdc0de::RGBColor;
using cmdc0de::StateBase;

CommunicationSettingState::CommunicationSettingState() : Darknet7BaseState()
	, CommSettingList("Comm Info:", Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0]))), Items(), ListBuffer(), InternalState(NONE), ESPRequestID(0) {

}

CommunicationSettingState::~CommunicationSettingState()
{

}
void CommunicationSettingState::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::CommunicationStatusResponse> *mevt) {
	if(mevt->RequestID==this->ESPRequestID) {
		for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
			Items[i].text = &ListBuffer[i][0];
		}
		sprintf(&ListBuffer[0][0], "Wifi Status: %s", EnumNameWiFiStatus(mevt->InnerMsg->WifiStatus()));
		sprintf(&ListBuffer[1][0], "BLE Advertise: %s", mevt->InnerMsg->BLEAdvertise()?DarkNet7::sYES:DarkNet7::sNO);
		sprintf(&ListBuffer[2][0], "BLE DeviceName: %s", mevt->InnerMsg->BLEDevideName()->c_str());
		sprintf(&ListBuffer[3][0], "BLE Discoverable: %s", mevt->InnerMsg->BLEDiscoverable()?DarkNet7::sYES:DarkNet7::sNO);
		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		DarkNet7::get().getGUI().drawList(&CommSettingList);
		MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
		InternalState = DISPLAY_DATA;
	}
}

ErrorType CommunicationSettingState::onInit() {
	InternalState = FETCHING_DATA;
	flatbuffers::FlatBufferBuilder fbb;
	auto r = darknet7::CreateCommunicationStatusRequest(fbb);
	ESPRequestID = DarkNet7::get().nextSeq();
	auto e = darknet7::CreateSTMToESPRequest(fbb,ESPRequestID,darknet7::STMToESPAny_CommunicationStatusRequest, r.Union());
	darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	const MSGEvent<darknet7::CommunicationStatusResponse> *si = 0;
	MCUToMCU::get().getBus().addListener(this,si,&MCUToMCU::get());
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);

	DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Fetching data from ESP",RGBColor::BLUE);

	MCUToMCU::get().send(fbb);

	//turn on AP on / off
	// below see bluetoothtask class
	//BLE discoverable off/on
	//BLE set/get device name
	//

	return ErrorType();
}

StateBase::ReturnStateContext CommunicationSettingState::onRun() {

	StateBase *nextState = this;
	if (!GUIListProcessor::process(&CommSettingList,(sizeof(Items) / sizeof(Items[0])))) {
		if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1 | DarkNet7::ButtonInfo::BUTTON_MID)) {
			switch (CommSettingList.selectedItem) {
				case 0:
					break;
				case 1:
					break;
				case 2:
	//				nextState = DarkNet7::get().getAddressBookState();
					break;
				case 3:
					break;
				case 4:
					break;
				}
		}
	}
	return ReturnStateContext(nextState);
}

ErrorType CommunicationSettingState::onShutdown()
{
	return ErrorType();
}


