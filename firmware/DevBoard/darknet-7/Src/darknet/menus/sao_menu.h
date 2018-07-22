/*
 * sao_menu.h
 *
 *  Created on: Jul 17, 2018
 *      Author: cmdc0de
 */

#ifndef DARKNET_MENUS_SAO_MENU_H_
#define DARKNET_MENUS_SAO_MENU_H_




#include "darknet7_base_state.h"


class SAO: public Darknet7BaseState {
public:
	SAO();
	virtual ~SAO();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:

};




#endif /* DARKNET_MENUS_SAO_MENU_H_ */
