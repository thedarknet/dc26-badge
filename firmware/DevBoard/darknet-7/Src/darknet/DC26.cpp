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
#include <usart.h>
#include <spi.h>
#include <i2c.h>
#include "libstm32/app/display_message_state.h"

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

cmdc0de::DrawBufferNoBuffer NoBuffer(&Display, &DrawBuffer[0],
		DISPLAY_OPT_WRITE_ROWS);

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
#if 1
	HAL_GPIO_WritePin(SIMPLE_LED1_GPIO_Port, SIMPLE_LED1_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(SIMPLE_LED2_GPIO_Port, SIMPLE_LED2_Pin, GPIO_PIN_SET);
#endif
	GUI gui(&Display);
	gui.drawList(&DrawList);
	Display.swap();

	return et;
}

static const char *RFAILED = "Receive Failed";
static const char *TFAILED = "Transmit Failed";
static const uint16_t ESP_ADDRESS = 1;

ErrorType DC26::onRun() {
	if (HAL_GPIO_ReadPin(MID_BUTTON1_GPIO_Port, MID_BUTTON1_Pin)
			== GPIO_PIN_RESET) {
		static const char *dis = "mid";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (HAL_GPIO_ReadPin(BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin)
			== GPIO_PIN_RESET) {
		static const char *dis = "right";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (HAL_GPIO_ReadPin(BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin)
			== GPIO_PIN_RESET) {
		static const char *dis = "left";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (HAL_GPIO_ReadPin(BUTTON_UP_GPIO_Port, BUTTON_UP_Pin)
			== GPIO_PIN_RESET) {
		static const char *dis = "up";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (HAL_GPIO_ReadPin(BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin)
			== GPIO_PIN_RESET) {
		static const char *dis = "down";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (HAL_GPIO_ReadPin(BUTTON_FIRE1_GPIO_Port, BUTTON_FIRE1_Pin)
			== GPIO_PIN_RESET) {
		static const char *dis = "fire";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	}
	if (HAL_GPIO_ReadPin(MID_BUTTON1_GPIO_Port, MID_BUTTON1_Pin)
			== GPIO_PIN_RESET) {
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		char oBuf[129] = { '\0' };
		char iBuf[129] = { '\0' };
		strcpy(&oBuf[0], "1234567890");
		//uint8_t i = 0;
		//uint8_t o = 1;
		Display.drawString(0, 10, &iBuf[0], cmdc0de::RGBColor::WHITE);
		Display.drawString(0, 20, &oBuf[0], cmdc0de::RGBColor::WHITE);
#if 1
		//HAL_UART_IRQHandler()
		if (HAL_OK
				!= HAL_UART_Transmit(&huart1, (uint8_t *) &oBuf[0], 12, 1000)) {
			Display.drawString(0, 30, TFAILED, cmdc0de::RGBColor::WHITE);
		}
		HAL_Delay(5);
		//HAL_UART_GetState()
		if (HAL_OK
				!= HAL_UART_Receive(&huart1, (uint8_t *) &iBuf[0], 12, 1000)) {
			Display.drawString(0, 30, RFAILED, cmdc0de::RGBColor::WHITE);
		}
#endif
#if 0
		if(HAL_OK==HAL_I2C_IsDeviceReady(&hi2c1,ESP_ADDRESS,10,1000)) {
			if(HAL_OK==HAL_I2C_Master_Transmit(&hi2c1,ESP_ADDRESS,(uint8_t *)&oBuf[0],11,1000)) {
				if(HAL_OK!=HAL_I2C_Master_Receive(&hi2c1,ESP_ADDRESS,(uint8_t*)&iBuf[0],129,1000)) {
					Display.drawString(0,30,RFAILED, cmdc0de::RGBColor::WHITE);
				}
			} else {
				Display.drawString(0,30,TFAILED, cmdc0de::RGBColor::WHITE);
			}
		} else {
			Display.drawString(0,30,DFAILED, cmdc0de::RGBColor::WHITE);
		}
#endif
		Display.drawString(0, 50, &oBuf[0], cmdc0de::RGBColor::WHITE);
		Display.drawString(0, 60, &iBuf[0], cmdc0de::RGBColor::WHITE);
		Display.swap();
		HAL_Delay(2000);
	}

	cmdc0de::StateBase::ReturnStateContext rsc = getCurrentState()->run();
	Display.swap();

	if (rsc.Err.ok()) {
		if (getCurrentState() != rsc.NextMenuToRun) {
			//on state switches reset keyboard and give a 1 second pause on reading from keyboard.
			//KB.reset();
		}
		//if (CurrentState != StateFactory::getGameOfLifeState()
		//		&& (tick > KB.getLastPinSelectedTick())
		//		&& (tick - KB.getLastPinSelectedTick()
		//				> (1000 * 60
		//						* rc.getContactStore().getSettings().getScreenSaverTime()))) {
		//	CurrentState->shutdown();
		//	CurrentState = StateFactory::getGameOfLifeState();
		//} else {
		//	CurrentState = rsc.NextMenuToRun;
		//}
	} else {
		//setCurrentState(StateCollection::getDisplayMessageState(
		//		StateCollection::getDisplayMenuState(), (const char *)"Run State Error....", uint16_t (2000)));
	}

	return ErrorType();
}


cmdc0de::DisplayMessageState DMS;

cmdc0de::DisplayMessageState *StateCollection::getDisplayMessageState(cmdc0de::StateBase *bm, const char *message, uint16_t timeToDisplay) {
	DMS.setMessage(message);
	DMS.setNextState(bm);
	DMS.setTimeInState(timeToDisplay);
	DMS.setDisplay(&Display);
	return &DMS;
}

MenuState *StateCollection::getDisplayMenuState() {
	return 0;
}

