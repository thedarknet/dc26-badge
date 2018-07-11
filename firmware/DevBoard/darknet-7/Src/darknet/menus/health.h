/*
 * tamagotchi.h
 *
 *  Created on: May 30, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_HEALTH_H_
#define DARKNET_MENUS_HEALTH_H_


#include "darknet7_base_state.h"


class Health: public Darknet7BaseState {
public:
	Health();
	virtual ~Health();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:

};




#endif /* DARKNET_MENUS_TAMAGOTCHI_H_ */
