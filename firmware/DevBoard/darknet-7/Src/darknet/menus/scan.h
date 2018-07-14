/*
 * tamagotchi.h
 *
 *  Created on: May 30, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_SCAN_H_
#define DARKNET_MENUS_SCAN_H_


#include "darknet7_base_state.h"


class Scan: public Darknet7BaseState {
public:
	Scan();
	virtual ~Scan();
	void setNPCOnly(bool b) {NPCOnly = b;}
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	bool NPCOnly;
};




#endif /* DARKNET_MENUS_TAMAGOTCHI_H_ */
