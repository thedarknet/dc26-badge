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

//TODO Can't swap between landscape and portait
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

static const uint32_t DISPLAY_OPT_WRITE_ROWS = DISPLAY_HEIGHT;
DisplayST7735 Display(DISPLAY_WIDTH, DISPLAY_HEIGHT, START_ROT);
uint16_t DrawBuffer[DISPLAY_WIDTH * DISPLAY_OPT_WRITE_ROWS]; //120 wide, 10 pixels high, 2 bytes per pixel (uint16_t)

cmdc0de::DrawBufferNoBuffer NoBuffer(&Display, &DrawBuffer[0],DISPLAY_OPT_WRITE_ROWS);

cmdc0de::DrawBuffer2D16BitColor16BitPerPixel1Buffer DisplayBuffer(static_cast<uint8_t>(DISPLAY_WIDTH),static_cast<uint8_t>(DISPLAY_HEIGHT),
		&DrawBuffer[0],&Display);

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
			uint8_t(0), uint8_t(DISPLAY_WIDTH), uint8_t(DISPLAY_HEIGHT/2), uint8_t(0), uint8_t(0));
	//DO SELF CHECK
	if ((et = Display.init(DisplayST7735::FORMAT_16_BIT, &Font_6x10, &DisplayBuffer)).ok()) {
			//DisplayST7735::MADCTL_MY, &Font_6x10, &DisplayBuffer)).ok()) {
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
	Display.fillScreen(cmdc0de::RGBColor::BLACK);
	Display.swap();
	Display.fillRec(30,10,80,40,cmdc0de::RGBColor(0,255,0));
	Display.swap();
#if DEBUG_WHY_CANT_CHANGE_ROTATION
	//Display.setRotation(cmdc0de::DisplayDevice::LANDSCAPE_TOP_LEFT,true);
	Display.fillScreen(cmdc0de::RGBColor::BLACK);
	Display.swap();
	gui.drawList(&DrawList);
	Display.swap();
	Display.fillRec(30,10,80,40,cmdc0de::RGBColor(0,255,0));
	Display.swap();
#endif

	uint16_t r = 0, g= 0, b = 0;
	uint16_t y = 0;
	while(y<Display.getHeight()) {
		for(uint32_t i=0;i<Display.getWidth();i++) {
			Display.drawPixel(i,y,cmdc0de::RGBColor(r,g,b));
			if(r==0xFF && g==0xFF && b==0xFF) {
				r=g=b=0;
			} else if (r==0xFF && g==0xFF) {
				++b;
			} else if (r==0xFF) {
				++g;
			} else {
				++r;
			}
		}
		++y;
	}
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

