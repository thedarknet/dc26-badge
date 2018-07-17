/*
 * setting_state.h
 *
 *  Created on: May 29, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_SETTING_STATE_H_
#define DARKNET_MENUS_SETTING_STATE_H_

#include "darknet7_base_state.h"
#include "../KeyStore.h"
#include "../virtual_key_board.h"

class SettingState: public Darknet7BaseState {
public:
	SettingState();
	virtual ~SettingState();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	cmdc0de::GUIListData SettingList;
	cmdc0de::GUIListItemData Items[3];
	char AgentName[ContactStore::AGENT_NAME_LENGTH];
	uint8_t SubState;
	uint8_t MiscCounter;
	VirtualKeyBoard VKB;
	VirtualKeyBoard::InputHandleContext IHC;
};


#endif /* DARKNET_MENUS_SETTING_STATE_H_ */
