/*
 * virtual_key_board.cpp
 *
 *  Created on: Jul 14, 2018
 *      Author: cmdc0de
 */

#include "virtual_key_board.h"
#include "darknet7.h"
#include <string>

using cmdc0de::RGBColor;

const char *VirtualKeyBoard::STDKB = "abcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-";

void VirtualKeyBoard::InputHandleContext::addChar(char b) {
	if(Buf!=0) {
		if(CurrentPos<(Size-1)) {
			Buf[CurrentPos] = b;
			++CurrentPos;
		} else {
			//do nothing
		}
	}
}

void VirtualKeyBoard::InputHandleContext::backspace() {
	Buf[CurrentPos] = '\0';
	--CurrentPos;
}

VirtualKeyBoard::VirtualKeyBoard(): VKB(0), SizeOfKeyboard(0), XDisplayPos(0), XEndDisplayPos(DarkNet7::DISPLAY_WIDTH), YDisplayPos(0),
	FontColor(RGBColor::WHITE), BackGround(RGBColor::BLACK), CursorColor(RGBColor::BLUE), CursorChar('_'), CursorPos(0), CharsPerRow(0), InputContext(0) {


}

void VirtualKeyBoard::init(const char *vkb, InputHandleContext *ic, int16_t xdisplayPos, int16_t xEndDisplay, int16_t yDisplayPos, const cmdc0de::RGBColor &fontColor,
		const cmdc0de::RGBColor &backgroundColor, const cmdc0de::RGBColor &cursorColor, char cursorChar) {
	VKB = vkb;
	SizeOfKeyboard = strlen(VKB);
	XDisplayPos = xdisplayPos;
	XEndDisplayPos = xEndDisplay;
	YDisplayPos = yDisplayPos;
	FontColor = fontColor;
	BackGround = backgroundColor;
	CursorColor = cursorColor;
	CursorChar = cursorChar;
	CursorPos = 0;
	uint8_t FontPixelWidth = DarkNet7::get().getDisplay().getFont()->FontWidth;
	CharsPerRow = (XEndDisplayPos-XDisplayPos)/FontPixelWidth;
	InputContext = ic;
}



void VirtualKeyBoard::process() {
	if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_LEFT)) {
		if(CursorPos>0)	--CursorPos;
	} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_RIGHT)) {
		if(CursorPos<SizeOfKeyboard) CursorPos++;
		else CursorPos=0;
	} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_UP)) {
		if(CursorPos>=CharsPerRow) CursorPos-=CharsPerRow;
	} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_DOWN)) {
		CursorPos+=CharsPerRow;
		if(CursorPos>SizeOfKeyboard) CursorPos = SizeOfKeyboard-1;
	} else if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
		if(InputContext) {
			InputContext->addChar(VKB[CursorPos]);
		}
	}
}


uint8_t VirtualKeyBoard::getCursorX() {
	return CursorPos%CharsPerRow;
}


uint8_t VirtualKeyBoard::getCursorY() {
	return CursorPos/CharsPerRow;

}

uint8_t VirtualKeyBoard::getVKBIndex() {
	return CursorPos;
}


char VirtualKeyBoard::getSelectedChar() {
	return VKB[CursorPos];
}





