/*
 * virtual_key_board.h
 *
 *  Created on: Jul 14, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_VIRTUAL_KEY_BOARD_H_
#define DARKNET_MENUS_VIRTUAL_KEY_BOARD_H_

#include "libstm32/display/display_device.h"
#include "libstm32/observer/event_bus.h"

class VirtualKeyBoard {
public:
	class InputHandleContext {
	public:
		InputHandleContext(char *b, uint8_t s) : Buf(b), Size(s), CurrentPos(0) {}
		void addChar(char b);
		void backspace();
	private:
		char *Buf;
		uint8_t Size;
		uint8_t CurrentPos;
	};
public:
	typedef cmdc0de::EventBus<1,2,2,3> VKB_EVENT_BUS_TYPE;
	static const char *STDKB; //= "abcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-";
public:
	VirtualKeyBoard();
	void init(const char *vkb, InputHandleContext *ic, int16_t xdisplayPos, int16_t xEndDisplay, int16_t yDisplayPos, const cmdc0de::RGBColor &fontColor,
			const cmdc0de::RGBColor &backgroundColor, const cmdc0de::RGBColor &cursorColor, const char cursorChar = '_');
	void process();
	uint8_t getCursorX();
	uint8_t getCursorY();
	uint8_t getVKBIndex();
	char getSelectedChar();
private:
	const char *VKB;
	uint16_t SizeOfKeyboard;
	int16_t XDisplayPos;
	int16_t XEndDisplayPos;
	int16_t YDisplayPos;
	cmdc0de::RGBColor FontColor;
	cmdc0de::RGBColor BackGround;
	cmdc0de::RGBColor CursorColor;
	char CursorChar;
	int16_t CursorPos;
	int16_t CharsPerRow;
	InputHandleContext *InputContext;
};



#endif /* DARKNET_MENUS_VIRTUAL_KEY_BOARD_H_ */
