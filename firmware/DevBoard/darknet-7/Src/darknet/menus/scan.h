/*
 * tamagotchi.h
 *
 *  Created on: May 30, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_SCAN_H_
#define DARKNET_MENUS_SCAN_H_


#include "darknet7_base_state.h"
#include "../mcu_to_mcu.h"


namespace darknet7 {
	class ESPToSTM;
	class ESPSystemInfo;
}


class Scan: public Darknet7BaseState {
public:
	Scan();
	virtual ~Scan();
	void setNPCOnly(bool b) {NPCOnly = b;}
	bool isNPCOnly() {return NPCOnly;}
	enum INTERNAL_STATE {NONE, FETCHING_DATA, DISPLAY_DATA};
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::WiFiScanResults> *mevt);
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	bool NPCOnly;
	cmdc0de::GUIListData DisplayList;
	cmdc0de::GUIListItemData Items[7];
	char ListBuffer[7][48]; //height then width
	uint32_t ESPRequestID;
	INTERNAL_STATE InternalState;
};




#endif /* DARKNET_MENUS_TAMAGOTCHI_H_ */
