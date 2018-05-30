/*
 * darknet7_base_state.h
 *
 *  Created on: May 29, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_DARKNET7_BASE_STATE_H_
#define DARKNET_MENUS_DARKNET7_BASE_STATE_H_

#include "../libstm32/app/state_base.h"
#include "../messaging/stm_to_esp_generated.h"
#include "../libstm32/display/gui.h"


class Darknet7BaseState: public cmdc0de::StateBase {
public:
	Darknet7BaseState() {}
	virtual ~Darknet7BaseState() {}
};



#endif /* DARKNET_MENUS_DARKNET7_BASE_STATE_H_ */
