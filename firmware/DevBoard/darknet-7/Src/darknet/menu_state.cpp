
#include "menu_state.h"
#include "libstm32/display/display_device.h"

using cmdc0de::ErrorType;
using cmdc0de::StateBase;

MenuState::MenuState(cmdc0de::DisplayDevice *d) :
		StateBase(), MenuList("Main Menu", Items, 0, 0, d->getWidth(), d->getHeight(), 0, (sizeof(Items) / sizeof(Items[0]))),
				Display(d)
{
}

MenuState::~MenuState() {

}

const char *HasMessage = "DCDN Net Msgs *";
const char *NoHasMessage = "DCDN Net Msgs";

ErrorType MenuState::onInit() {
	Items[0].id = 0;
	//if (getContactStore().getSettings().isNameSet()) {
		Items[0].text = (const char *) "Settings";
	//} else {
	//	Items[0].text = (const char *) "Settings *";
	//}
	/*

	Items[1].id = 1;
	Items[1].text = (const char *) "IR Pair";
	Items[2].id = 2;
	Items[2].text = (const char *) "Address Book";
	Items[3].id = 3;
	if (((MessageState *) StateFactory::getMessageState())->hasNewMessage()) {
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
	Items[7].text = (const char *) "Radio Info";
	Items[8].id = 8;
	Items[8].text = (const char *) "KeyBoard Test";
	Items[9].id = 9;
	Items[9].text = (const char *) "Quest Dialing";
	Items[10].id = 10;
	Items[10].text = (const char *) "Gateway";
	rc.getDisplay().fillScreen(RGBColor::BLACK);
	rc.getGUI().drawList(&this->MenuList);
	*/
	return ErrorType();
}

cmdc0de::StateBase::ReturnStateContext MenuState::onRun() {
	StateBase *nextState = this;
	/*
	StateBase *nextState = this;
	uint8_t key = rc.getKB().getLastKeyReleased();

	switch (key) {
		case QKeyboard::UP: {
			if (MenuList.selectedItem == 0) {
				MenuList.selectedItem = sizeof(Items) / sizeof(Items[0]) - 1;
			} else {
				MenuList.selectedItem--;
			}
			break;
		}
		case QKeyboard::DOWN: {
			if (MenuList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
				MenuList.selectedItem = 0;
			} else {
				MenuList.selectedItem++;
			}
			break;
		}
		case QKeyboard::BACK: {
			MenuList.selectedItem = 0;
		}
			break;
		case QKeyboard::ENTER: {
			switch (MenuList.selectedItem) {
				case 0:
					nextState = StateFactory::getSettingState();
					break;
				case 1:
					if (rc.getContactStore().getSettings().getAgentName()[0] != '\0') {
						nextState = StateFactory::getIRPairingState();
					} else {
						nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(),
								(const char *) "You must set your agent name first", 3000);
					}
					break;
				case 2:
					nextState = StateFactory::getAddressBookState();
					break;
				case 3:
					nextState = StateFactory::getMessageState();
					break;
				case 4:
					nextState = StateFactory::get3DState();
					break;
				case 5:
					nextState = StateFactory::getGameOfLifeState();
					break;
				case 6:
					nextState = StateFactory::getBadgeInfoState();
					break;
				case 7:
					nextState = StateFactory::getRadioInfoState();
					break;
				case 8:
					nextState = StateFactory::getKeyBoardTest();
					break;
				case 9:
					rc.getKB().setDialerMode(true);
					nextState = StateFactory::getKeyBoardTest();
					break;
				case 10:
					nextState = StateFactory::getGateway();
					break;
			}
		}
			break;
	}
	if (rc.getKB().wasKeyReleased() && key != 9) {
		rc.getGUI().drawList(&this->MenuList);
	}
	*/
	return ReturnStateContext(nextState);
}

ErrorType MenuState::onShutdown() {
	//MenuList.selectedItem = 0;
	return ErrorType();
}
