/*
 * sao_menu.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: cmdc0de
 */



#include "sao_menu.h"
#include "../darknet7.h"
#include "menu_state.h"
#include <i2c.h>

using cmdc0de::RGBColor;
using cmdc0de::ErrorType;
using cmdc0de::StateBase;


SAO::SAO() : Darknet7BaseState(), InternalState(NONE), Address(NOADDRESS) {

}

SAO::~SAO()
{

}

ErrorType SAO::onInit() {
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
	InternalState = DISPLAY_SCANNING;
	Address = NOADDRESS;
	return ErrorType();
}

StateBase::ReturnStateContext SAO::onRun() {
	StateBase *nextState = this;
	if(InternalState==DISPLAY_SCANNING) {
		DarkNet7::get().getDisplay().drawString(0,20,(const char *)"Starting I2C scan");
		InternalState = SCANNING;
	} else if(InternalState==SCANNING) {
		HAL_StatusTypeDef result;
		for (uint8_t i=1; i<128; i++) {
		  /*
		   * the HAL wants a left aligned i2c address
		   * &hi2c1 is the handle
		   * (uint16_t)(i<<1) is the i2c address left aligned
		   * retries 2
		   * timeout 2
		   */
		  result = HAL_I2C_IsDeviceReady(&hi2c3, (uint16_t)(i<<1), 2, 2);
		  if (result == HAL_OK) {
			  Address = (uint16_t)(i<<1);
		  }
		}
		InternalState = INTERACTING;
	} else {
		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		if(Address==NOADDRESS) {
			DarkNet7::get().getDisplay().drawString(0,20,(const char *)"No Shitty Add on Badge found!",RGBColor::RED, RGBColor::BLACK, 1, true);
		} else {
			char buf[24];
			DarkNet7::get().getDisplay().drawString(0,20,(const char *)"Shitty Add on Badge found!", RGBColor::BLUE, RGBColor::BLACK, 1, true);
			sprintf(&buf[0],"Address: %d", (int)Address);
			DarkNet7::get().getDisplay().drawString(0,30,&buf[0]);
		}
		if(DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
			nextState =DarkNet7::get().getDisplayMenuState();
		}
	}
	return ReturnStateContext(nextState);
}

ErrorType SAO::onShutdown()
{
	return ErrorType();
}








