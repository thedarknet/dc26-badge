#include "menu_state.h"
#include "../libstm32/display/display_device.h"
#include "../darknet7.h"
#include "MessageState.h"
#include "setting_state.h"
#include "pairing_state.h"
#include "AddressState.h"
#include "GameOfLife.h"
#include "menu3d.h"
#include "communications_settings.h"
#include "badge_info_state.h"
#include "mcu_info.h"
#include "tamagotchi.h"

using cmdc0de::ErrorType;
using cmdc0de::StateBase;
using cmdc0de::RGBColor;

MenuState::MenuState() :
		Darknet7BaseState(), MenuList("Main Menu", Items, 0, 0, DarkNet7::get().getDisplay().getWidth(),
				DarkNet7::get().getDisplay().getHeight()
				, 0, (sizeof(Items) / sizeof(Items[0])))
{
}

MenuState::~MenuState() {

}

const char *HasMessage = "DarkNet Msgs *";
const char *NoHasMessage = "DarkNet Msgs";

ErrorType MenuState::onInit() {
	Items[0].id = 0;
	if (DarkNet7::get().getContacts().getSettings().isNameSet()) {
		Items[0].text = (const char *) "Settings";
	} else {
		Items[0].text = (const char *) "Settings *";
	}
	Items[1].id = 1;
	Items[1].text = (const char *) "Badge Pair";
	Items[2].id = 2;
	Items[2].text = (const char *) "Address Book";
	Items[3].id = 3;
	if (DarkNet7::get().getMessageState()->hasNewMsg()) {
		Items[3].text = HasMessage;
	} else {
		Items[3].text = NoHasMessage;
	}
	Items[4].id = 4;
	Items[4].text = (const char *) "3D";
	Items[5].id = 5;
	Items[5].text = (const char *) "Screen Saver";
	Items[6].id = 6;
	Items[6].text = (const char *) "Badge Info";
	Items[7].id = 7;
	Items[7].text = (const char *) "MCU Info";
	Items[8].id = 8;
	Items[8].text = (const char *) "Tamagotchi";
	Items[9].id = 9;
	Items[9].text = (const char *) "Communications Settings";
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
	DarkNet7::get().getGUI().drawList(&this->MenuList);
	return ErrorType();
}

cmdc0de::StateBase::ReturnStateContext MenuState::onRun() {
	StateBase *nextState = this;
	if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_UP)) {
		if (MenuList.selectedItem == 0) {
			MenuList.selectedItem = sizeof(Items) / sizeof(Items[0]) - 1;
		} else {
			MenuList.selectedItem--;
		}
	} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_DOWN)) {
		if (MenuList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
			MenuList.selectedItem = 0;
		} else {
			MenuList.selectedItem++;
		}
	} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_LEFT)) {
		MenuList.selectedItem = 0;
	} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
		switch (MenuList.selectedItem) {
			case 0:
				nextState = DarkNet7::get().getSettingState();
				break;
			case 1:
				if (DarkNet7::get().getContacts().getSettings().getAgentName()[0] != '\0') {
					nextState = DarkNet7::get().getPairingState();
				} else {
					nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),
							(const char *) "You must set your agent name first", 3000);
				}
				break;
			case 2:
				nextState = DarkNet7::get().getAddressBookState();
				break;
			case 3:
				nextState = DarkNet7::get().get3DState();
				break;
			case 5:
				nextState = DarkNet7::get().getGameOfLifeState();
				break;
			case 6:
				nextState = DarkNet7::get().getBadgeInfoState();
				break;
			case 7:
				nextState = DarkNet7::get().getMCUInfoState();
				break;
			case 8:
				nextState = DarkNet7::get().getTamagotchiState();
				break;
			case 9:
				nextState = DarkNet7::get().getCommunicationSettingState();
				break;

		}
	}
	if (DarkNet7::get().getButtonInfo().wasAnyButtonReleased()) {
		DarkNet7::get().getGUI().drawList(&this->MenuList);
	}

	return StateBase::ReturnStateContext(nextState);
}

ErrorType MenuState::onShutdown() {
	//MenuList.selectedItem = 0;
	return ErrorType();
}
