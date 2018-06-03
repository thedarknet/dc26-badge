#include "MessageState.h"
#include "../darknet7.h"
#include "menu_state.h"

using cmdc0de::ErrorType;


MessageState::RadioMessage::RadioMessage() :
		Msg(), FromUID(0), Rssi(0) {
	memset(&Msg[0], 0, sizeof(Msg));
}

MessageState::MessageState() : Darknet7BaseState()
		, RMsgs(), InternalState(MESSAGE_LIST), MsgList("Radio Msgs", Items, 0, 0, 128, 64, 0,
				(sizeof(Items) / sizeof(Items[0]))), CurrentPos(0), NewMessage(0) {
	memset(&RMsgs[0], 0, sizeof(RMsgs));
}

MessageState::~MessageState() {

}

uint32_t min(uint32_t one, uint32_t two) {
	if (one < two)
		return one;
	return two;
}

void MessageState::addMsg(const char *msg, uint16_t msgSize, uint16_t uid, uint8_t rssi) {
	memset(&RMsgs[CurrentPos].Msg[0], 0, sizeof(RMsgs[CurrentPos].Msg));
	memcpy(&RMsgs[CurrentPos].Msg[0], msg, min(msgSize, sizeof(RMsgs[CurrentPos].Msg)-1));
	RMsgs[CurrentPos].Rssi = rssi;
	RMsgs[CurrentPos].FromUID = uid;
	CurrentPos++;
	CurrentPos = CurrentPos % ((sizeof(RMsgs) / sizeof(RMsgs[0])));
	NewMessage = true;
}

ErrorType MessageState::onInit() {
	InternalState = MESSAGE_LIST;
	//look at the newest message (the one just before cur pos bc currentpos is inc'ed after adding a message
	uint8_t v = CurrentPos == 0 ? MAX_R_MSGS - 1 : CurrentPos - 1;
	for (uint16_t i = 0; i < MAX_R_MSGS; i++) {
		Items[i].id = RMsgs[v].FromUID;
		ContactStore::Contact c;
		if (RMsgs[v].FromUID == DarkNet7::BROADCAST_ADDR) {
			Items[i].text = "Broadcast Msg";
		} else if (DarkNet7::get().getContacts().findContactByID(RMsgs[v].FromUID, c)) {
			Items[i].text = c.getAgentName();
			Items[i].setShouldScroll();
		} else {
			Items[i].text = "";
		}
		v = v == 0 ? (MAX_R_MSGS - 1) : v - 1;
	}
	DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
	DarkNet7::get().getGUI().drawList(&MsgList);
	return ErrorType();
}

static char MsgDisplayBuffer[62] = { '\0' };
static char FromBuffer[20] = { '\0' };

cmdc0de::StateBase::ReturnStateContext MessageState::onRun() {
	StateBase *nextState = this;

	if (InternalState == MESSAGE_LIST) {
		NewMessage = false;
		if(DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_UP)) {
			if (MsgList.selectedItem == 0) {
				MsgList.selectedItem = sizeof(Items) / sizeof(Items[0]) - 1;
			} else {
				MsgList.selectedItem--;
			}
		} else if (DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_DOWN)) {
			if (MsgList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
				MsgList.selectedItem = 0;
			} else {
				MsgList.selectedItem++;
			}
		} else if (DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_LEFT)) {
			nextState = DarkNet7::get().getDisplayMenuState();
		} else if (DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
			if (Items[MsgList.selectedItem].id != 0) {
				MsgDisplayBuffer[0] = '\0';
				for (uint16_t i = 0; i < (sizeof(RMsgs) / sizeof(RMsgs[0])); i++) {
					if (RMsgs[i].FromUID == Items[MsgList.selectedItem].id) {
						strncpy(&MsgDisplayBuffer[0], RMsgs[i].Msg, sizeof(RMsgs[i].Msg));
					}
				}
				if (MsgDisplayBuffer[0] == '\0') {
					sprintf(&MsgDisplayBuffer[0], "Message from %s is gone only 8 stored in memory.",
							Items[MsgList.selectedItem].text);
					nextState = DarkNet7::get().getDisplayMessageState(this,&MsgDisplayBuffer[0], 5000);
				} else {
					InternalState = DETAIL;
					sprintf(&FromBuffer[0], "F: %s", Items[MsgList.selectedItem].text);
				}
			}
		}
		DarkNet7::get().getGUI().drawList(&MsgList);
	} else {
		//find message in array:
		DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
		DarkNet7::get().getDisplay().drawString(0, 10, &FromBuffer[0]);
		DarkNet7::get().getDisplay().drawString(0, 20, &MsgDisplayBuffer[0]);
		if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_LEFT|DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
			onInit();
			InternalState = MESSAGE_LIST;
		}
	}
	return cmdc0de::StateBase::ReturnStateContext(nextState);
}

ErrorType MessageState::onShutdown() {
	return ErrorType();
}

