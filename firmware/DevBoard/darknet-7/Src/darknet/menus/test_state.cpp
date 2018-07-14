#include "test_state.h"
#include "../darknet7.h"
#include "menu_state.h"

using cmdc0de::ErrorType;


TestState::TestState() : Darknet7BaseState() {
}

TestState::~TestState() {

}



ErrorType TestState::onInit() {
	return ErrorType();
}

cmdc0de::StateBase::ReturnStateContext TestState::onRun() {
	StateBase *nextState = this;

#if 0
	/*
	if (MyButtons.wereAnyOfTheseButtonsReleased(ButtonInfo::BUTTON_MID)) {
		static const char *dis = "mid";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (MyButtons.wereAnyOfTheseButtonsReleased(ButtonInfo::BUTTON_RIGHT)) {
		static const char *dis = "right";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (MyButtons.wereAnyOfTheseButtonsReleased(ButtonInfo::BUTTON_LEFT)) {
		static const char *dis = "left";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (MyButtons.wereAnyOfTheseButtonsReleased(ButtonInfo::BUTTON_UP)) {
		static const char *dis = "up";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (MyButtons.wereAnyOfTheseButtonsReleased(ButtonInfo::BUTTON_DOWN)) {
		static const char *dis = "down";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	} else if (MyButtons.wereAnyOfTheseButtonsReleased(ButtonInfo::BUTTON_FIRE1)) {
		static const char *dis = "fire";
		Display.fillScreen(cmdc0de::RGBColor::BLACK);
		Display.drawString(0, 20, dis, cmdc0de::RGBColor::WHITE);

	}
	*/
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
#endif
	return cmdc0de::StateBase::ReturnStateContext(nextState);
}

ErrorType TestState::onShutdown() {
	return ErrorType();
}

