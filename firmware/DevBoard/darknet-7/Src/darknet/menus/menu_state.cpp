#include "menu_state.h"
#include "../libstm32/display/display_device.h"
#include "../darknet7.h"
#include "test_state.h"
#include "setting_state.h"
#include "pairing_state.h"
#include "AddressState.h"
#include "GameOfLife.h"
#include "menu3d.h"
#include "communications_settings.h"
#include "badge_info_state.h"
#include "mcu_info.h"
#include "tamagotchi.h"
#include "health.h"
#include "scan.h"
#include "gui_list_processor.h"

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
	Items[3].text = (const char *) "3D";
	Items[4].id = 4;
	Items[4].text = (const char *) "Screen Saver";
	Items[5].id = 5;
	Items[5].text = (const char *) "STM Info";
	Items[6].id = 6;
	Items[6].text = (const char *) "ESP Info";
	Items[7].id = 7;
	Items[7].text = (const char *) "Tamagotchi";
	Items[8].id = 8;
	Items[8].text = (const char *) "Communications Settings";
	Items[9].id = 9;
	Items[9].text = (const char *) "Health";
	Items[10].id = 10;
	Items[10].text = (const char *) "Scan for NPCs";
	Items[11].id = 11;
	Items[11].text = (const char *) "Test Badge";
	Items[12].id = 12;
	Items[12].text = (const char *) "Scan: Shitty Addon Badge";
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
	DarkNet7::get().getGUI().drawList(&this->MenuList);
	return ErrorType();
}

cmdc0de::StateBase::ReturnStateContext MenuState::onRun() {
	StateBase *nextState = this;
	if (!GUIListProcessor::process(&MenuList,(sizeof(Items) / sizeof(Items[0])))) {
		if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1 | DarkNet7::ButtonInfo::BUTTON_MID)) {
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
			case 4:
				nextState = DarkNet7::get().getGameOfLifeState();
				break;
			case 5:
				nextState = DarkNet7::get().getBadgeInfoState();
				break;
			case 6:
				nextState = DarkNet7::get().getMCUInfoState();
				break;
			case 7:
				nextState = DarkNet7::get().getTamagotchiState();
				break;
			case 8:
				nextState = DarkNet7::get().getCommunicationSettingState();
				break;
			case 9:
				nextState = DarkNet7::get().getHealthState();
				break;
			case 10:
				DarkNet7::get().getScanState()->setNPCOnly(true);
				nextState = DarkNet7::get().getScanState();
				break;
			case 11:
				nextState = DarkNet7::get().getTestState();
				break;

			}
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
