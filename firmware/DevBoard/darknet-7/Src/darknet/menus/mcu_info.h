/*
 * mcu_info.h
 *
 *  Created on: May 30, 2018
 *      Author: cmdc0de
 */

#ifndef DARKNET_MENUS_MCU_INFO_H_
#define DARKNET_MENUS_MCU_INFO_H_

#include "darknet7_base_state.h"
#include "../darknet7.h"
#include "../mcu_to_mcu.h"

namespace darknet7 {
	class ESPToSTM;
	class ESPSystemInfo;
}

class MCUInfoState: public Darknet7BaseState {
public:
	MCUInfoState();
	virtual ~MCUInfoState();
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::ESPSystemInfo> *);
protected:
	enum INTERNAL_STATE {NONE, FETCHING_DATA, DISPLAY_DATA};
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	cmdc0de::GUIListData BadgeInfoList;
	cmdc0de::GUIListItemData Items[7];
	char ListBuffer[7][48]; //height then width
	uint32_t ESPRequestID;
	INTERNAL_STATE InternalState;
};




#endif /* DARKNET_MENUS_MCU_INFO_H_ */
