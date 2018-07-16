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
#include "libstm32/display/gui.h"
#include "KeyStore.h"


class MenuState;
class TestState;
class SendMsgState;
class SettingState;
class PairingState;
class AddressState;
class Menu3D;
class GameOfLife;
class CommunicationSettingState;
class BadgeInfoState;
class MCUInfoState;
class Tamagotchi;
class Health;
class Scan;
class VirtualKeyBoard;


class DarkNet7: public cmdc0de::App {
public:
	static const char *sYES;
	static const char *sNO;

	#define START_LANDSCAPE
	#ifdef START_LANDSCAPE
		static const uint32_t DISPLAY_WIDTH = 160;
		static const uint32_t DISPLAY_HEIGHT = 128;
		#define START_ROT DisplayST7735::LANDSCAPE_TOP_LEFT
	#else
		static const uint32_t DISPLAY_WIDTH = 128;
		static const uint32_t DISPLAY_HEIGHT = 160;
		#define START_ROT DisplayST7735::PORTAIT_TOP_LEFT
	#endif
	static const uint16_t BROADCAST_ADDR = 0xFFFF;
public:
	static DarkNet7 &get();
	class ButtonInfo {
	public:
		enum BUTTON {
			BUTTON_LEFT = 0x01
			, BUTTON_RIGHT = 0x02
			, BUTTON_UP = 0x04
			, BUTTON_DOWN = 0x08
			, BUTTON_MID = 0x10
			, BUTTON_FIRE1 = 0x20
			, ANY_KEY = BUTTON_LEFT|BUTTON_RIGHT|BUTTON_UP|BUTTON_DOWN|BUTTON_MID|BUTTON_FIRE1
		};
	public:
		ButtonInfo();
		bool areTheseButtonsDown(const int32_t &b);
		bool isAnyOfTheseButtonDown(const int32_t &b);
		bool isAnyButtonDown();
		bool wereTheseButtonsReleased(const int32_t &b);
		bool wereAnyOfTheseButtonsReleased(const int32_t &b);
		bool wasAnyButtonReleased();
		void reset();
	protected:
		void processButtons();
		friend class DarkNet7;
	private:
		uint8_t ButtonState;
		uint8_t LastButtonState;
	};
public:
	cmdc0de::DisplayMessageState *getDisplayMessageState(cmdc0de::StateBase *bm, const char *message,
			uint16_t timeToDisplay);
	MenuState *getDisplayMenuState();
	TestState *getTestState();
	SendMsgState *getSendMsgState();
	SettingState *getSettingState();
	PairingState *getPairingState();
	AddressState *getAddressBookState();
	Menu3D *get3DState();
	GameOfLife *getGameOfLifeState();
	CommunicationSettingState *getCommunicationSettingState();
	BadgeInfoState * getBadgeInfoState();
	MCUInfoState *getMCUInfoState();
	Tamagotchi *getTamagotchiState();
	Health *getHealthState();
	Scan *getScanState();
public:
	cmdc0de::DisplayST7735 &getDisplay();
	const cmdc0de::DisplayST7735 &getDisplay() const;
	ContactStore &getContacts();
	const ContactStore &getContacts() const;
	cmdc0de::GUI &getGUI();
	const cmdc0de::GUI &getGUI() const;
	ButtonInfo &getButtonInfo();
	const ButtonInfo&getButtonInfo() const;
	uint32_t nextSeq();
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
	cmdc0de::GUI MyGUI;
	ButtonInfo MyButtons;
	uint32_t SequenceNum;
private:
	static DarkNet7 *mSelf;
};

namespace cmdc0de {
class DisplayMessageState;
}

#endif /* DARKNET_DC26_H_ */
