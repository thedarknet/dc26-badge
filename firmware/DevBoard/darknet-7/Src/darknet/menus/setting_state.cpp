/*
 * setting_state.cpp
 *
 *  Created on: May 29, 2018
 *      Author: dcomes
 */

#include "setting_state.h"

using cmdc0de::ErrorType;
using cmdc0de::StateBase;
using cmdc0de::RGBColor;


SettingState::SettingState() :
		Darknet7BaseState(), SettingList((const char *) "MENU", Items, 0, 0, 128, 160, 0, sizeof(Items) / sizeof(Items[0])), InputPos(
				0), SubState(0) {

	memset(&AgentName[0], 0, sizeof(AgentName));
	Items[0].id = 0;
	Items[0].text = (const char *) "Set Agent Name";
	Items[1].id = 1;
	Items[1].text = (const char *) "Screen Saver Time";
	Items[1].setShouldScroll();
	Items[2].id = 2;
	Items[2].text = (const char *) "Reset Badge Contacts";
	Items[2].setShouldScroll();
}

SettingState::~SettingState() {

}

ErrorType SettingState::onInit() {
	/*
	SubState = 0;
	rc.getDisplay().fillScreen(RGBColor::BLACK);
	rc.getGUI().drawList(&SettingList);
	*/
	return ErrorType();
}

StateBase::ReturnStateContext SettingState::onRun(RunContext &rc) {
	//uint8_t key = rc.getKB().getLastKeyReleased();
	StateBase *nextState = this;
	/*
	switch (SubState) {
		case 0:
			switch (key) {
				case QKeyboard::UP: {
					if (SettingList.selectedItem == 0) {
						SettingList.selectedItem = sizeof(Items) / sizeof(Items[0]) - 1;
					} else {
						SettingList.selectedItem--;
					}
					break;
				}
				case QKeyboard::DOWN: {
					if (SettingList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
						SettingList.selectedItem = 0;
					} else {
						SettingList.selectedItem++;
					}
					break;
				}
				case QKeyboard::BACK: {
					nextState = StateFactory::getMenuState();
				}
					break;
				case QKeyboard::ENTER: {
					SubState = SettingList.selectedItem + 100;
					rc.getDisplay().fillScreen(RGBColor::BLACK);
					switch (SubState) {
						case 100:
							memset(&AgentName[0], 0, sizeof(AgentName));
							getKeyboardContext().init(&AgentName[0], sizeof(AgentName));
							rc.getDisplay().drawString(0, 10, (const char*) "Current agent name:");
							if (*rc.getContactStore().getSettings().getAgentName() == '\0') {
								rc.getDisplay().drawString(0, 20, (const char *) "NOT SET");
							} else {
								rc.getDisplay().drawString(0, 20, rc.getContactStore().getSettings().getAgentName());
							}
							rc.getDisplay().drawString(0, 30, (const char*) "Set agent name:");
							break;
						case 101:
							rc.getDisplay().drawString(0, 10, (const char*) "Time until badge\ngoes to sleep:",
									RGBColor::WHITE, RGBColor::BLACK, 1, true);
							InputPos = rc.getContactStore().getSettings().getScreenSaverTime();
							break;
						case 102:
							rc.getKB().reset();
							rc.getDisplay().drawString(0, 10, (const char*) "ERASE ALL\nCONTACTS?");
							rc.getDisplay().drawString(0, 30, (const char*) "Touch 1 to Cancel");
							rc.getDisplay().drawString(0, 34, (const char*) "Touch Hook to do it");
							break;
					}
				}
					break;
				default:
					break;
			}
			break;

		case 100:
			rc.getKB().updateContext(getKeyboardContext());
			if (rc.getKB().getLastKeyReleased() == QKeyboard::ENTER && AgentName[0] != '\0' && AgentName[0] != ' '
					&& AgentName[0] != '_') {
				AgentName[ContactStore::AGENT_NAME_LENGTH - 1] = '\0';
				getKeyboardContext().finalize();
				//done
				if (rc.getContactStore().getSettings().setAgentname(&AgentName[0])) {
					nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Save Successful",
							2000);
				} else {
					nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Save FAILED!",
							4000);
				}
			} else {
				rc.getDisplay().drawString(0, 40, &AgentName[0]);
			}
			break;
		case 101:
			if (rc.getKB().getLastKeyReleased() == QKeyboard::ENTER) {
				if (rc.getContactStore().getSettings().setScreenSaverTime(InputPos)) {
					nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Setting saved",
							2000);
				} else {
					nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Save FAILED!",
							4000);
				}
			} else if (rc.getKB().getLastKeyReleased() != QKeyboard::NO_PIN_SELECTED) {
				InputPos = rc.getKB().getLastKeyReleased() + 1;
				if (InputPos > 8) {
					InputPos = 8;
				} else if (InputPos < 1) {
					InputPos = 1;
				}
			}
			sprintf(&AgentName[0], "%d Minutes", InputPos);
			rc.getDisplay().drawString(0, 40, &AgentName[0]);
			break;
		case 102:
			if (rc.getKB().getLastKeyReleased() == QKeyboard::BACK) {
				nextState = StateFactory::getMenuState();
			} else if (rc.getKB().getLastKeyReleased() == QKeyboard::ENTER) {
				rc.getContactStore().resetToFactory();
				StateFactory::getAddressBookState()->resetSelection();
				nextState = StateFactory::getMenuState();
			}
			break;
	}
	if (SubState < 100 && rc.getKB().wasKeyReleased()) {
		rc.getGUI().drawList(&SettingList);
	}
	*/
	return ReturnStateContext(nextState);
}

ErrorType SettingState::onShutdown()
{
	InputPos = 0;
	memset(&AgentName[0], 0, sizeof(AgentName));
	return ErrorType();
}
