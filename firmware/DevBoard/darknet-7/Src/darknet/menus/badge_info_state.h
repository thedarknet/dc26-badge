/*
 * badge_info_state.h
 *
 *  Created on: May 29, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_BADGE_INFO_STATE_H_
#define DARKNET_MENUS_BADGE_INFO_STATE_H_

#include "darknet7_base_state.h"

class ContactStore;

class BadgeInfoState: public Darknet7BaseState {
public:
	BadgeInfoState();
	virtual ~BadgeInfoState();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
	const char *getRegCode(ContactStore &cs);
private:
	cmdc0de::GUIListData BadgeInfoList;
	cmdc0de::GUIListItemData Items[9];
	char ListBuffer[9][32]; //height then width
	char RegCode[18];
};


#endif /* DARKNET_MENUS_BADGE_INFO_STATE_H_ */
