/*
 * mcu_info.h
 *
 *  Created on: May 30, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_MCU_INFO_H_
#define DARKNET_MENUS_MCU_INFO_H_

#include "darknet7_base_state.h"


class MCUInfoState: public Darknet7BaseState {
public:
	MCUInfoState();
	virtual ~MCUInfoState();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	cmdc0de::GUIListData BadgeInfoList;
	cmdc0de::GUIListItemData Items[9];
	char ListBuffer[9][64]; //height then width
};




#endif /* DARKNET_MENUS_MCU_INFO_H_ */
