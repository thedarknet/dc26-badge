/*
 * scan.cpp
 *
 *  Created on: May 30, 2018
 *      Author: cmdc0de
 */



#include "scan.h"
#include "../darknet7.h"
#include "../messaging/stm_to_esp_generated.h"
#include "../messaging/esp_to_stm_generated.h"
#include "../mcu_to_mcu.h"
#include "menu_state.h"
#include "gui_list_processor.h"

using cmdc0de::RGBColor;
using cmdc0de::ErrorType;
using cmdc0de::StateBase;

///////////////////////////////////////////
class NPCInteract: public Darknet7BaseState {
public:
	enum INTERNAL_STATE {NONE, FETCHING_DATA, DISPLAYING_DATA};

private:
	uint32_t RequestID;
	cmdc0de::GUIListData DisplayList;
	cmdc0de::GUIListItemData Items[7];
	char ListBuffer[7][48]; //height then width
	INTERNAL_STATE InternalState;
public:
	NPCInteract() : Darknet7BaseState(), RequestID(0), DisplayList("NPC Interaction:", Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0]))), InternalState(NONE) {

	}
	virtual ~NPCInteract() {}
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::NPCInteractionResponse> *mevt) {
		if(mevt->RequestID==this->RequestID) {
			InternalState = DISPLAYING_DATA;

			MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());

			for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
				Items[i].text = &ListBuffer[i][0];
				Items[i].id = i;
				Items[i].setShouldScroll();
			}
			//for(uint32_t i=0;i<mevt->InnerMsg->APs()->Length() && i<(sizeof(Items) / sizeof(Items[0]));++i) {
			//	sprintf(&ListBuffer[i][0], "%s : %s ", mevt->InnerMsg->APs()->Get(i)->ssid()->c_str(), darknet7::EnumNameWifiMode(mevt->InnerMsg->APs()->Get(i)->authMode()));
			//}

			DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
			DarkNet7::get().getGUI().drawList(&DisplayList);
		}
	}
protected:
	virtual cmdc0de::ErrorType onInit() {

		return ErrorType();
	}

	virtual cmdc0de::StateBase::ReturnStateContext onRun() {
		StateBase *nextState = this;
			switch(InternalState) {
			case FETCHING_DATA:
				if(this->getTimesRunCalledSinceLastReset()>200) {
					nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::NO_DATA_FROM_ESP,2000);
				}
				break;
			case DISPLAYING_DATA:
				if (!GUIListProcessor::process(&DisplayList,(sizeof(Items) / sizeof(Items[0])))) {
					if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
						nextState = DarkNet7::get().getDisplayMenuState();
					} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
						//nextState = &NPCInteraction;
					}
				}
				DarkNet7::get().getGUI().drawList(&DisplayList);
				break;
			case NONE:
				break;
			}
			return ReturnStateContext(nextState);
	}

	virtual cmdc0de::ErrorType onShutdown() {
		return ErrorType();
	}
};
static NPCInteract NPCInteraction;




////////////////////////////////////////////

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
		for(uint32_t i=0;i<mevt->InnerMsg->APs()->Length() && i<(sizeof(Items) / sizeof(Items[0]));++i) {
			sprintf(&ListBuffer[i][0], "%s : %s ", mevt->InnerMsg->APs()->Get(i)->ssid()->c_str(), darknet7::EnumNameWifiMode(mevt->InnerMsg->APs()->Get(i)->authMode()));
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
	auto e = darknet7::CreateSTMToESPRequest(fbb,ESPRequestID,darknet7::STMToESPAny_WiFiScan, r.Union());
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
		if (!GUIListProcessor::process(&DisplayList,(sizeof(Items) / sizeof(Items[0])))) {
			if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
				nextState = DarkNet7::get().getDisplayMenuState();
			} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
				nextState = &NPCInteraction;
			}
		}
		DarkNet7::get().getGUI().drawList(&DisplayList);
		break;
	case NONE:
		break;
	}
	return ReturnStateContext(nextState);
}


ErrorType Scan::onShutdown()
{
	const MSGEvent<darknet7::WiFiScanResults> *mevt=0;
	MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
	return ErrorType();
}



