/*
 * DC26.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: dcomes
 */

#include "DC26.h"
#include "libstm32/display/display_device.h"
#include "libstm32/display/gui.h"
#include "libstm32/display/fonts.h"
#include <main.h>
#include "libstm32/config.h"

using cmdc0de::ErrorType;
using cmdc0de::DisplayST7735;
using cmdc0de::DrawBufferNoBuffer;
using cmdc0de::GUIListData;
using cmdc0de::GUIListItemData;
using cmdc0de::GUI;

static const uint32_t DISPLAY_WIDTH = 128;
static const uint32_t DISPLAY_HEIGHT = 160;
static const uint32_t DISPLAY_OPT_WRITE_ROWS = 2;
DisplayST7735 Display(DISPLAY_WIDTH, DISPLAY_HEIGHT, DisplayST7735::PORTAIT);
uint16_t DrawBuffer[DISPLAY_WIDTH * DISPLAY_OPT_WRITE_ROWS]; //120 wide, 10 pixels high, 2 bytes per pixel (uint16_t)
uint8_t DrawBufferRangeChange[DISPLAY_HEIGHT / DISPLAY_OPT_WRITE_ROWS + 1];

cmdc0de::DrawBufferNoBuffer NoBuffer(&Display, &DrawBuffer[0], DISPLAY_OPT_WRITE_ROWS);



DC26::DC26() {
	// TODO Auto-generated constructor stub

}

DC26::~DC26() {
	// TODO Auto-generated destructor stub
}

ErrorType DC26::onInit() {
	ErrorType et;

	GUIListItemData items[4];
	GUIListData DrawList((const char *) "Self Check", items, uint8_t(0),
			uint8_t(0), uint8_t(128), uint8_t(64), uint8_t(0), uint8_t(0));
	//DO SELF CHECK
	if ((et = Display.init(DisplayST7735::FORMAT_16_BIT,
			DisplayST7735::ROW_COLUMN_ORDER, &Font_6x10, &NoBuffer)).ok()) {
		HAL_Delay(1000);
		items[0].set(0, "OLED_INIT");
		DrawList.ItemsCount++;
	}
	GUI gui(&Display);
	gui.drawList(&DrawList);
	Display.swap();

	return et;
}

ErrorType DC26::onRun() {
	return ErrorType();
}
