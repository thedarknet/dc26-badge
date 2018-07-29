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

const char *VirtualKeyBoard::STDKBLowerCase = "abcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-";
const char *VirtualKeyBoard::STDKBNames = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-_";
const char *VirtualKeyBoard::STDCAPS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

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

//Remember CurrentPos here means the current position in the input buffer not the position in the keyboard like below.
void VirtualKeyBoard::InputHandleContext::backspace() {
	Buf[CurrentPos] = '\0';
	--CurrentPos;
}

////////

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
			InputContext->addChar(getSelectedChar());
		}
	}
	uint16_t y = 0;
	const char *ptr = VKB;
	uint8_t FontPixelHeight = DarkNet7::get().getDisplay().getFont()->FontHeight;
	uint8_t FontPixelWidth = DarkNet7::get().getDisplay().getFont()->FontWidth;
	uint8_t cursorRow = getCursorY();
	uint8_t curosrColumn = getCursorX();
	for(int i=0;i<SizeOfKeyboard && y < (DarkNet7::DISPLAY_HEIGHT-(y*FontPixelHeight));i+=CharsPerRow, ++y) {
		DarkNet7::get().getDisplay().drawString(XDisplayPos, (YDisplayPos+(y*FontPixelHeight)), ptr, FontColor, BackGround, 1, false, CharsPerRow);
		if(y==cursorRow) {
			if((HAL_GetTick()%1000)<500) {
				DarkNet7::get().getDisplay().drawString(XDisplayPos+(curosrColumn*FontPixelWidth), YDisplayPos+(y*FontPixelHeight), (const char *)"_", CursorColor, BackGround, 1, false);
			}
		}
		ptr = ptr + CharsPerRow;
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





