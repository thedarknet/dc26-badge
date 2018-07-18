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


class BLESetDeviceNameMenu: public Darknet7BaseState {
private:
	VirtualKeyBoard VKB;
	char NewDeviceName[13];
	VirtualKeyBoard::InputHandleContext IHC;
	const char *CurrentDeviceName;
	//uint32_t RequestID;
public:
	void setCurrentNamePtr(const char *p) {CurrentDeviceName = p;}
	BLESetDeviceNameMenu() : Darknet7BaseState(), VKB(), NewDeviceName(), IHC(&NewDeviceName[0],sizeof(NewDeviceName)),
			CurrentDeviceName(0) { //, RequestID(0) {

	}
	virtual ~BLESetDeviceNameMenu() {}
protected:
	virtual cmdc0de::ErrorType onInit() {
		memset(&NewDeviceName[0],0,sizeof(NewDeviceName));
		VKB.init(VirtualKeyBoard::STDKBLowerCase,&IHC,5,DarkNet7::DISPLAY_WIDTH-5,60,RGBColor::WHITE, RGBColor::BLACK,
				RGBColor::BLUE,'_');
		return ErrorType();
	}

	virtual cmdc0de::StateBase::ReturnStateContext onRun() {
		StateBase *nextState = this;
			DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);

			VKB.process();

			DarkNet7::get().getDisplay().drawString(0,10,(const char *)"Current Name: ");
			DarkNet7::get().getDisplay().drawString(0,20,CurrentDeviceName);
			DarkNet7::get().getDisplay().drawString(0,40,(const char *)"Mid button finishes");
			if(DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
				flatbuffers::FlatBufferBuilder fbb;
				auto r = darknet7::CreateBLESetDeviceNameDirect(fbb,&NewDeviceName[0]);
				//RequestID = DarkNet7::get().nextSeq();
				auto z = darknet7::CreateSTMToESPRequest(fbb,DarkNet7::get().nextSeq(),darknet7::STMToESPAny_BLESetDeviceName,r.Union());
				darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,z);
				//send BLE set device name
				//switch state to communication setting which will send a request for communication settings
				//
				MCUToMCU::get().send(fbb);
				//const MSGEvent<darknet7::CommunicationStatusResponse> *si = 0;
				//MCUToMCU::get().getBus().addListener(this,si,&MCUToMCU::get());
				nextState = DarkNet7::get().getCommunicationSettingState();
			}
		return ReturnStateContext(nextState);
	}

	virtual cmdc0de::ErrorType onShutdown() {
		return ErrorType();
	}
};
static BLESetDeviceNameMenu BLESetName_Menu;

class BLEBoolMenu: public Darknet7BaseState {
public:
	enum TYPE {AD};
private:
	cmdc0de::GUIListData BLEList;
	cmdc0de::GUIListItemData Items[2];
	char ListBuffer[2][6];
	bool YesOrNo;
	TYPE Type;
public:
	BLEBoolMenu(const char *name, TYPE t) : Darknet7BaseState(),
		BLEList(name, Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0])))
		, YesOrNo(false), Type(t) {
		strcpy(&ListBuffer[0][0],"Yes");
		strcpy(&ListBuffer[1][0],"No");
	}
	virtual ~BLEBoolMenu() {}
	void setValue(bool b) {YesOrNo = b;}
protected:
	virtual cmdc0de::ErrorType onInit() {
		BLEList.items[0].id = 0;
		BLEList.items[0].text = &ListBuffer[0][0];
		BLEList.items[1].id = 1;
		BLEList.items[1].text = &ListBuffer[1][0];
		if(YesOrNo) {
			BLEList.selectedItem = 0;
		} else {
			BLEList.selectedItem = 1;
		}
		return ErrorType();
	}

	virtual cmdc0de::StateBase::ReturnStateContext onRun() {
		StateBase *nextState = this;
		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		DarkNet7::get().getGUI().drawList(&BLEList);

		if(DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
			flatbuffers::FlatBufferBuilder fbb;
			if(Type==AD) {
				auto r = darknet7::CreateBLEAdvertise(fbb,BLEList.selectedItem);
				auto z = darknet7::CreateSTMToESPRequest(fbb,DarkNet7::get().nextSeq(),darknet7::STMToESPAny_BLEAdvertise,r.Union());
				darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,z);
			}
			MCUToMCU::get().send(fbb);
			nextState = DarkNet7::get().getCommunicationSettingState();
		}
		return ReturnStateContext(nextState);
	}

	virtual cmdc0de::ErrorType onShutdown() {
		return ErrorType();
	}
};
static const char *Advertise = "BLE Advertise";
static BLEBoolMenu BLEAdvertise_Menu(Advertise, BLEBoolMenu::AD);


class WiFi: public Darknet7BaseState {
private:
	VirtualKeyBoard VKB;
	char SSID[17];
	char Password[32];
	darknet7::WifiMode SecurityType;
	VirtualKeyBoard::InputHandleContext IHC;
	cmdc0de::GUIListData WifiSettingList;
	cmdc0de::GUIListItemData Items[4];
	char ListBuffer[4][33];
	uint16_t WorkingItem;
	static const uint16_t NO_WORKING_TIME = 0xFFFF;
public:
	//WiFi() : Darknet7BaseState(), VKB(), SSID(), Password(), SecurityType(darknet7::WifiMode_OPEN), IHC(&SSID[0],sizeof(SSID)) { //, ListBuffer() {
	WiFi() : Darknet7BaseState(), VKB(), SSID(), Password(), SecurityType(darknet7::WifiMode_OPEN), IHC(0,0)
		, WifiSettingList("WiFi Settings:", Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0])))
		, ListBuffer(), WorkingItem(NO_WORKING_TIME) {
	}
	virtual ~WiFi() {}
protected:
	virtual cmdc0de::ErrorType onInit() {
		memset(&SSID[0],0,sizeof(SSID));
		memset(&Password[0],0,sizeof(Password));
		memset(&ListBuffer[0],0,sizeof(ListBuffer));
		for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
			Items[i].id = i;
			Items[i].text = &ListBuffer[i][0];
			Items[i].setShouldScroll();
		}
		return ErrorType();
	}

	virtual cmdc0de::StateBase::ReturnStateContext onRun() {
		StateBase *nextState = this;
		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		sprintf(&ListBuffer[0][0],"AP Type: %s", darknet7::EnumNameWifiMode(SecurityType));
		if(SecurityType!=darknet7::WifiMode_OPEN ) {
			sprintf(&ListBuffer[1][0],"SSID: %s", &SSID[0]);
			sprintf(&ListBuffer[2][0],"Password: %s", &Password[0]);
		}
		strcpy(&ListBuffer[3][0],"Submit");

		if(WorkingItem==NO_WORKING_TIME) {
			if (!GUIListProcessor::process(&WifiSettingList,(sizeof(Items) / sizeof(Items[0])))) {
				if(DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
					WorkingItem = WifiSettingList.selectedItem;
					switch(WorkingItem) {
					case 1:
						IHC.set(&SSID[0],sizeof(SSID));
						VKB.init(VirtualKeyBoard::STDKBLowerCase,&IHC,5,DarkNet7::DISPLAY_WIDTH-5,60,RGBColor::WHITE,	RGBColor::BLACK, RGBColor::BLUE,'_');
						break;
					case 2:
						IHC.set(&Password[0],sizeof(Password));
						VKB.init(VirtualKeyBoard::STDKBLowerCase,&IHC,5,DarkNet7::DISPLAY_WIDTH-5,60,RGBColor::WHITE,	RGBColor::BLACK, RGBColor::BLUE,'_');
						break;
					case 3:
					{
						flatbuffers::FlatBufferBuilder fbb;
						auto r = darknet7::CreateSetupAPDirect(fbb,&SSID[0],&Password[0],SecurityType);
						auto z = darknet7::CreateSTMToESPRequest(fbb,DarkNet7::get().nextSeq(),darknet7::STMToESPAny_BLESetDeviceName,r.Union());
						darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,z);
						MCUToMCU::get().send(fbb);
					}
						break;
					}
				} else if ( DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
					nextState = DarkNet7::get().getCommunicationSettingState();
				}
			}
		} else {
			switch(WorkingItem) {
			case 0:
			{
				if(DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_UP)) {
					uint32_t s = (uint32_t)(SecurityType);
					++s;
					s = s%darknet7::WifiMode_MAX;
					SecurityType = (darknet7::WifiMode)s;
				} else if (DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_DOWN)) {
					if(SecurityType==darknet7::WifiMode_MIN) {
						SecurityType = darknet7::WifiMode_MAX;
					} else {
						uint32_t s = (uint32_t)(SecurityType);
						--s;
						SecurityType = (darknet7::WifiMode)s;
					}
				} else if (DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID
																					| DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
					WorkingItem = NO_WORKING_TIME;
				}
			}
			break;
			case 1:
				VKB.process();
				break;
			case 2:
				VKB.process();
				break;
			}
		}
		return ReturnStateContext(nextState);
	}

	virtual cmdc0de::ErrorType onShutdown() {
		return ErrorType();
	}
};
static WiFi WiFiMenu;


CommunicationSettingState::CommunicationSettingState() : Darknet7BaseState()
	, CommSettingList("Comm Info:", Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0])))
	, Items(), ListBuffer(), CurrentDeviceName(), ESPRequestID(0), InternalState(NONE) {

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
		strcpy(&CurrentDeviceName[0],mevt->InnerMsg->BLEDevideName()->c_str());
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

	return ErrorType();
}

StateBase::ReturnStateContext CommunicationSettingState::onRun() {
	StateBase *nextState = this;
	if (!GUIListProcessor::process(&CommSettingList,(sizeof(Items) / sizeof(Items[0])))) {
		if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1 | DarkNet7::ButtonInfo::BUTTON_MID)) {
			DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
			switch(CommSettingList.selectedItem) {
			case 0:
				nextState = &WiFiMenu;
				break;
			case 1:
				nextState = &BLEAdvertise_Menu;
				break;
			case 2:
				BLESetName_Menu.setCurrentNamePtr(&CurrentDeviceName[0]);
				nextState = &BLESetName_Menu;
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


