#include "test_state.h"
#include "../darknet7.h"
#include "menu_state.h"

using cmdc0de::ErrorType;
using cmdc0de::RGBColor;


TestState::TestState() : Darknet7BaseState(), ButtonList("Button Info:", Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0]))), TimesFireHasBeenHeld(0) {

}

 TestState::~TestState() {

}


ErrorType TestState::onInit() {
	TimesFireHasBeenHeld=0;
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	sprintf(&ListBuffer[6][0], "Exit hold fire1");

	for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
		Items[i].text = &ListBuffer[i][0];
		Items[i].id = i;
		Items[i].setShouldScroll();
	}
	return ErrorType();
}

cmdc0de::StateBase::ReturnStateContext TestState::onRun() {
	StateBase *nextState = this;

	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
	sprintf(&ListBuffer[0][0], "   UP: %s", DarkNet7::get().getButtonInfo().isAnyOfTheseButtonDown(DarkNet7::ButtonInfo::BUTTON_UP)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[1][0], " DOWN: %s", DarkNet7::get().getButtonInfo().isAnyOfTheseButtonDown(DarkNet7::ButtonInfo::BUTTON_DOWN)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[2][0], " LEFT: %s", DarkNet7::get().getButtonInfo().isAnyOfTheseButtonDown(DarkNet7::ButtonInfo::BUTTON_LEFT)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[3][0], "RIGHT: %s", DarkNet7::get().getButtonInfo().isAnyOfTheseButtonDown(DarkNet7::ButtonInfo::BUTTON_RIGHT)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[4][0], "  MID: %s", DarkNet7::get().getButtonInfo().isAnyOfTheseButtonDown(DarkNet7::ButtonInfo::BUTTON_MID)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[5][0], "FIRE1: %s", DarkNet7::get().getButtonInfo().isAnyOfTheseButtonDown(DarkNet7::ButtonInfo::BUTTON_FIRE1)?DarkNet7::sYES:DarkNet7::sNO);
	if(DarkNet7::get().getButtonInfo().isAnyOfTheseButtonDown(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
		TimesFireHasBeenHeld++;
	} else {
		TimesFireHasBeenHeld=0;
	}
	if(TimesFireHasBeenHeld>EXIT_COUNT) {
		nextState = DarkNet7::get().getDisplayMenuState();
	}


#if 0
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
	DarkNet7::get().getGUI().drawList(&ButtonList);
	return cmdc0de::StateBase::ReturnStateContext(nextState);
}

ErrorType TestState::onShutdown() {
	return ErrorType();
}

