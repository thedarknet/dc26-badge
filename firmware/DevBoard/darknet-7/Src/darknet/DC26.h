/*
 * DC26.h
 *
 *  Created on: Dec 3, 2017
 *      Author: cmdc0de
 */

#ifndef DARKNET_DC26_H_
#define DARKNET_DC26_H_

#include "libstm32/app/app.h"
#include "libstm32/leds/ws2812.h"

class DC26 : public cmdc0de::App {
public:
	DC26();
	virtual ~DC26();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::ErrorType onRun();
private:
	cmdc0de::WS2818 Apa106s;
	//ContactStore MyContacts;
};


namespace cmdc0de {
	class DisplayMessageState;
}

class MenuState;

class StateCollection {
public:
	static cmdc0de::DisplayMessageState *getDisplayMessageState(cmdc0de::StateBase *bm, const char *message, uint16_t timeToDisplay);
	static MenuState *getDisplayMenuState();

};

#endif /* DARKNET_DC26_H_ */
