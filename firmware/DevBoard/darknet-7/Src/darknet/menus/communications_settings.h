/*
 * communications_settings.h
 *
 *  Created on: May 29, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_COMMUNICATIONS_SETTINGS_H_
#define DARKNET_MENUS_COMMUNICATIONS_SETTINGS_H_

#include "darknet7_base_state.h"
#include "../mcu_to_mcu.h"
#include "../virtual_key_board.h"


namespace darknet7 {
	class ESPToSTM;
	class CommunicationStatusResponse;
}


class CommunicationSettingState: public Darknet7BaseState {
public:
	CommunicationSettingState();
	virtual ~CommunicationSettingState();
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::CommunicationStatusResponse> *mevt);
protected:
	enum INTERNAL_STATE { NONE, FETCHING_DATA, DISPLAY_DATA };
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	cmdc0de::GUIListData CommSettingList;
	cmdc0de::GUIListItemData Items[3];
	char ListBuffer[3][24];
	char CurrentDeviceName[13];
	uint32_t ESPRequestID;
	INTERNAL_STATE InternalState;
	darknet7::WiFiStatus CurrentWifiStatus;
};



#endif /* DARKNET_MENUS_COMMUNICATIONS_SETTINGS_H_ */
