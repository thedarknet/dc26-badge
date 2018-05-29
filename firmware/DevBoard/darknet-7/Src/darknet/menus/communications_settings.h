/*
 * communications_settings.h
 *
 *  Created on: May 29, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_COMMUNICATIONS_SETTINGS_H_
#define DARKNET_MENUS_COMMUNICATIONS_SETTINGS_H_

#include "darknet7_base_state.h"

class CommunicationSettingState: public Darknet7BaseState {
public:
	CommunicationSettingState();
	virtual ~CommunicationSettingState();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	cmdc0de::GUIListData RadioInfoList;
	cmdc0de::GUIListItemData Items[6];
	char ListBuffer[6][20];
};



#endif /* DARKNET_MENUS_COMMUNICATIONS_SETTINGS_H_ */
