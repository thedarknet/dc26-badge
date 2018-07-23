#include "test_state.h"
#include "../darknet7.h"
#include "menu_state.h"

using cmdc0de::ErrorType;
using cmdc0de::RGBColor;


TestState::TestState() : Darknet7BaseState(), ButtonList("Button Info:", Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0]))), TimesMidHasBeenHeld(0) {

}

 TestState::~TestState() {

}


ErrorType TestState::onInit() {
	TimesMidHasBeenHeld=0;
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	sprintf(&ListBuffer[6][0], "Exit hold MID");
	sprintf(&ListBuffer[7][0], "Hold LF and DN for LED");

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
	if(DarkNet7::get().getButtonInfo().areTheseButtonsDown(DarkNet7::ButtonInfo::BUTTON_LEFT | DarkNet7::ButtonInfo::BUTTON_DOWN)) {
		HAL_GPIO_WritePin(SIMPLE_LED1_GPIO_Port, SIMPLE_LED1_Pin, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(SIMPLE_LED1_GPIO_Port, SIMPLE_LED1_Pin, GPIO_PIN_RESET);
	}
	if(DarkNet7::get().getButtonInfo().isAnyOfTheseButtonDown(DarkNet7::ButtonInfo::BUTTON_MID)) {
		TimesMidHasBeenHeld++;
	} else {
		TimesMidHasBeenHeld=0;
	}
	if(TimesMidHasBeenHeld>EXIT_COUNT) {
		nextState = DarkNet7::get().getDisplayMenuState();
	}

	DarkNet7::get().getGUI().drawList(&ButtonList);
	return cmdc0de::StateBase::ReturnStateContext(nextState);
}

ErrorType TestState::onShutdown() {
	return ErrorType();
}

