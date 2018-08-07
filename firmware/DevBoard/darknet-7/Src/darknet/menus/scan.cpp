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
	enum INTERNAL_STATE {NONE, NPC_LIST_REQUEST, NPC_LIST_DISPLAY, NPC_ERROR,
		NPC_INTERACT_REQUEST, NPC_INTERACT_DISPLAY};

private:
	uint32_t RequestID;
	cmdc0de::GUIListData DisplayList;
	cmdc0de::GUIListItemData Items[8];
	char ListBuffer[8][96]; //height then width
	INTERNAL_STATE InternalState;
	WiFiInfo *InteractInfo;
	uint32_t Timer;
	char NPCName[32];
public:
	NPCInteract() : Darknet7BaseState(), RequestID(0),
		DisplayList(NPCList, Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0])))
		, InternalState(NONE), InteractInfo(0), Timer(0), NPCName() {

	}
	virtual ~NPCInteract() {}
	void setInteraction(WiFiInfo *i) {InteractInfo = i;}
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::NPCInteractionResponse> *mevt) {
		if(mevt->RequestID==this->RequestID) {
			MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
			if(mevt->InnerMsg->wasError()>0) {
				InternalState = NPC_ERROR;
			} else {
				DisplayList.ItemsCount = sizeof(Items)/sizeof(Items[0]);
				for (uint32_t i = 0; i < DisplayList.ItemsCount; i++) {
					Items[i].text = &ListBuffer[i][0];
					Items[i].id = i;
					Items[i].setShouldScroll();
				}
				InternalState = NPC_INTERACT_DISPLAY;
				sprintf(&ListBuffer[0][0], "%s", mevt->InnerMsg->name()->c_str());
				sprintf(&ListBuffer[1][0], "%s", mevt->InnerMsg->description()->c_str());
				sprintf(&ListBuffer[2][0], "Interactions:");
				auto length = mevt->InnerMsg->actions()->Length();
				for(uint32_t i=3;i<3+length && i<((sizeof(Items) / sizeof(Items[0]))-1);++i) {
					sprintf(&ListBuffer[i][0], "%s", mevt->InnerMsg->actions()->GetAsString(i-3)->c_str());
				}
				if(mevt->InnerMsg->response()!=0) {
					sprintf(&ListBuffer[7][0], "R: %s", mevt->InnerMsg->response()->c_str());
				}
				if(mevt->InnerMsg->infections()!=0) {
					DarkNet7::get().getContacts().getSettings().setHealth(mevt->InnerMsg->infections());
				}
				DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
				DarkNet7::get().getGUI().drawList(&DisplayList);
			}
		}
	}
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::NPCList> *mevt) {
		if(mevt->RequestID==this->RequestID) {
			MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
			if(mevt->InnerMsg->wasError()>0) {
				InternalState = NPC_ERROR;
			} else {
				InternalState = NPC_LIST_DISPLAY;
				for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
					Items[i].text = &ListBuffer[i][0];
					Items[i].id = i;
					Items[i].setShouldScroll();
				}
				DisplayList.ItemsCount = mevt->InnerMsg->names()->Length();
				if(DisplayList.ItemsCount > (sizeof(Items) / sizeof(Items[0]))) {
					DisplayList.ItemsCount = sizeof(Items) / sizeof(Items[0]);
				}
				for(uint32_t i=0;i<mevt->InnerMsg->names()->Length() && i<(sizeof(Items) / sizeof(Items[0]));++i) {
					sprintf(&ListBuffer[i][0], "%s", mevt->InnerMsg->names()->GetAsString(i)->c_str());
				}

				DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
				DarkNet7::get().getGUI().drawList(&DisplayList);
			}
		}
	}
protected:
	virtual cmdc0de::ErrorType onInit() {
		InternalState = NPC_LIST_REQUEST;
		memset(&NPCName[0],0,sizeof(NPCName));
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
				if(this->getTimesRunCalledSinceLastReset()>500) {
					nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::NO_DATA_FROM_ESP,2000);
				}
				break;
			case NPC_INTERACT_REQUEST:
				if((HAL_GetTick()-Timer)>8000) {
					nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::NO_DATA_FROM_ESP,2000);
				}
				break;
			case NPC_LIST_DISPLAY:
				if (!GUIListProcessor::process(&DisplayList,DisplayList.ItemsCount)) {
					if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
						nextState = DarkNet7::get().getDisplayMenuState();
					} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
						InternalState = NPC_INTERACT_REQUEST;
						strcpy(&NPCName[0],DisplayList.items[DisplayList.selectedItem].text);
						flatbuffers::FlatBufferBuilder fbb;
						std::vector<uint8_t> bssid;
						for(int kk=0;kk<6;kk++) bssid.push_back(InteractInfo->Bssid[kk]);
						auto r = darknet7::CreateWiFiNPCInteractDirect(fbb,&bssid,(const char *)&InteractInfo->Sid[0],
								(int8_t)1, &NPCName[0],0);
						RequestID = DarkNet7::get().nextSeq();
						auto e = darknet7::CreateSTMToESPRequest(fbb,RequestID,darknet7::STMToESPAny_WiFiNPCInteract, r.Union());
						darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
						const MSGEvent<darknet7::NPCInteractionResponse> *si = 0;
						MCUToMCU::get().getBus().addListener(this,si,&MCUToMCU::get());
						DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);

						DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Sending Message...",RGBColor::BLUE);

						MCUToMCU::get().send(fbb);
						Timer = HAL_GetTick();
					} else {
						DarkNet7::get().getGUI().drawList(&DisplayList);
					}
				} else {
					DarkNet7::get().getGUI().drawList(&DisplayList);
				}
				break;
			case NPC_INTERACT_DISPLAY:
				if (!GUIListProcessor::process(&DisplayList,DisplayList.ItemsCount)) {
					if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
						nextState = DarkNet7::get().getDisplayMenuState();
					} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)
							&& DisplayList.selectedItem>2 && DisplayList.selectedItem<7) { //hacky hacky hacky
						InternalState = NPC_INTERACT_REQUEST;
						flatbuffers::FlatBufferBuilder fbb;
						std::vector<uint8_t> bssid;
						for(int kk=0;kk<6;kk++) bssid.push_back(InteractInfo->Bssid[kk]);
						auto r = darknet7::CreateWiFiNPCInteractDirect(fbb,&bssid,(const char *)&InteractInfo->Sid[0],
							(int8_t)1, &NPCName[0], DisplayList.items[DisplayList.selectedItem].text);
						RequestID = DarkNet7::get().nextSeq();
						auto e = darknet7::CreateSTMToESPRequest(fbb,RequestID,darknet7::STMToESPAny_WiFiNPCInteract, r.Union());
						darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
						const MSGEvent<darknet7::NPCInteractionResponse> *si = 0;
						MCUToMCU::get().getBus().addListener(this,si,&MCUToMCU::get());
						DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);

						DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Sending Message...",RGBColor::BLUE);

						MCUToMCU::get().send(fbb);
						Timer = HAL_GetTick();
					} else {
						DarkNet7::get().getGUI().drawList(&DisplayList);
					}
				} else {
					DarkNet7::get().getGUI().drawList(&DisplayList);
				}
				break;
			case NONE:
				break;
			case NPC_ERROR:
				nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),
						(const char *)"Communication error\nWith NPC",(uint16_t)1500);
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



