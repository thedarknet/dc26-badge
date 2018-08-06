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

static const char *NPCINTERACTION = "NPC Interaction";
static const char *NPCList = "NPC List";

///////////////////////////////////////////
class NPCInteract: public Darknet7BaseState {
public:
	enum INTERNAL_STATE {NONE, NPC_LIST_REQUEST, NPC_LIST_DISPLAY, NPC_LIST_ERROR,
		NPC_INTERACT_REQUEST, NPC_INTERACT_DISPLAY};

private:
	uint32_t RequestID;
	cmdc0de::GUIListData DisplayList;
	cmdc0de::GUIListItemData Items[7];
	char ListBuffer[7][48]; //height then width
	INTERNAL_STATE InternalState;
	WiFiInfo *InteractInfo;
public:
	NPCInteract() : Darknet7BaseState(), RequestID(0),
		DisplayList(NPCList, Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0])))
		, InternalState(NONE), InteractInfo(0) {

	}
	virtual ~NPCInteract() {}
	void setInteraction(WiFiInfo *i) {InteractInfo = i;}
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::NPCInteractionResponse> *mevt) {
		if(mevt->RequestID==this->RequestID) {
					//InternalState = DISPLAYING_DATA;

					MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
		}
	}
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::NPCList> *mevt) {
		if(mevt->RequestID==this->RequestID) {
			if(mevt->InnerMsg->wasError()>0) {
				InternalState = NPC_LIST_ERROR;
			} else {
				InternalState = NPC_LIST_DISPLAY;

				MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());

				for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
					Items[i].text = &ListBuffer[i][0];
					Items[i].id = i;
					Items[i].setShouldScroll();
				}
				DisplayList.ItemsCount = 0;
				for(uint32_t i=0;i<mevt->InnerMsg->names()->Length() && i<(sizeof(Items) / sizeof(Items[0]));++i) {
					sprintf(&ListBuffer[i][0], "%s", mevt->InnerMsg->names()->GetAsString(i)->c_str());
					DisplayList.ItemsCount++;
				}

				DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
				DarkNet7::get().getGUI().drawList(&DisplayList);
			}
		}
	}
protected:
	virtual cmdc0de::ErrorType onInit() {
		InternalState = NPC_LIST_REQUEST;
		flatbuffers::FlatBufferBuilder fbb;
		std::vector<uint8_t> bssid;
		for(int kk=0;kk<6;kk++) bssid.push_back(InteractInfo->Bssid[kk]);
		auto r = darknet7::CreateWiFiNPCInteractDirect(fbb,&bssid,(const char *)&InteractInfo->Sid[0],
				(int8_t)0);
		RequestID = DarkNet7::get().nextSeq();
		auto e = darknet7::CreateSTMToESPRequest(fbb,RequestID,darknet7::STMToESPAny_WiFiNPCInteract, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		memset(&ListBuffer[0], 0, sizeof(ListBuffer));
		const MSGEvent<darknet7::NPCList> *si = 0;
		MCUToMCU::get().getBus().addListener(this,si,&MCUToMCU::get());
		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);

		DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Getting NPCs at \nthis location",RGBColor::BLUE);

		MCUToMCU::get().send(fbb);
		return ErrorType();
	}

	virtual cmdc0de::StateBase::ReturnStateContext onRun() {
		StateBase *nextState = this;
			switch(InternalState) {
			case NPC_LIST_REQUEST:
				if(this->getTimesRunCalledSinceLastReset()>400) {
					nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::NO_DATA_FROM_ESP,2000);
				}
				break;
			case NPC_LIST_DISPLAY:
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
			default:
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
			sprintf(&ListBuffer[i][0], "%s : %s ", mevt->InnerMsg->APs()->Get(i)->ssid()->c_str(),
					darknet7::EnumNameWifiMode(mevt->InnerMsg->APs()->Get(i)->authMode()));
			memcpy(&Wifis[i].Bssid[0],mevt->InnerMsg->APs()->Get(i)->bssid(),6);
			strcpy(&Wifis[i].Sid[0],mevt->InnerMsg->APs()->Get(i)->ssid()->c_str());
		}

		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		DarkNet7::get().getGUI().drawList(&DisplayList);
	}
}

ErrorType Scan::onInit() {
	InternalState = FETCHING_DATA;
	memset(&Wifis[0],0,sizeof(Wifis));
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
				NPCInteraction.setInteraction(&Wifis[DisplayList.selectedItem]);
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



