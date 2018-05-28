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
#include "libstm32/display/display_device.h"
#include "libstm32/app/display_message_state.h"
#include "KeyStore.h"


class MenuState;

class DarkNet7 : public cmdc0de::App {
public:
	static DarkNet7 &get();
public:
	cmdc0de::DisplayMessageState *getDisplayMessageState(cmdc0de::StateBase *bm, const char *message, uint16_t timeToDisplay);
	MenuState *getDisplayMenuState();
	cmdc0de::DisplayST7735 &getDisplay();
	const cmdc0de::DisplayST7735 &getDisplay() const;
	ContactStore &getContacts();
	const ContactStore &getContacts() const;
	virtual ~DarkNet7();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::ErrorType onRun();
private:
	DarkNet7();
private:
	cmdc0de::WS2818 Apa106s;
	ContactStore MyContacts;
	cmdc0de::DisplayST7735 Display;
	cmdc0de::DrawBuffer2D16BitColor16BitPerPixel1Buffer DisplayBuffer;
	cmdc0de::DisplayMessageState DMS;
private:
	static DarkNet7 *mSelf;
};


namespace cmdc0de {
	class DisplayMessageState;
}

#endif /* DARKNET_DC26_H_ */
