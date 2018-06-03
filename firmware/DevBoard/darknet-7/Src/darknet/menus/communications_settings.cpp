/*
 * communications_settings.cpp
 *
 *  Created on: May 29, 2018
 *      Author: dcomes
 */


#include "communications_settings.h"
using cmdc0de::ErrorType;
using cmdc0de::RGBColor;
using cmdc0de::StateBase;

CommunicationSettingState::CommunicationSettingState() : Darknet7BaseState()
	, RadioInfoList("Radio Info:", Items, 0, 0, 128, 160, 0, (sizeof(Items) / sizeof(Items[0]))), Items(), ListBuffer() {

}

CommunicationSettingState::~CommunicationSettingState()
{

}

ErrorType CommunicationSettingState::onInit() {
	/*
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
		Items[i].text = &ListBuffer[i][0];
	}
	sprintf(&ListBuffer[0][0], "Frequency: %lu", (rc.getRadio().getFrequency() / (1000 * 1000)));
	sprintf(&ListBuffer[1][0], "RSSI: %d", rc.getRadio().readRSSI());
	sprintf(&ListBuffer[2][0], "RSSI Threshold: %u", rc.getRadio().getRSSIThreshHold());
	sprintf(&ListBuffer[3][0], "Gain: %u", rc.getRadio().getCurrentGain());
	sprintf(&ListBuffer[4][0], "Temp: %u", rc.getRadio().readTemperature());
	sprintf(&ListBuffer[5][0], "Impedance: %u", rc.getRadio().getImpedenceLevel());
	rc.getDisplay().fillScreen(RGBColor::BLACK);
	rc.getGUI().drawList(&RadioInfoList);
	*/
	return ErrorType();
}

StateBase::ReturnStateContext CommunicationSettingState::onRun() {

	StateBase *nextState = this;
	/*
	uint8_t pin = rc.getKB().getLastKeyReleased();
	if (pin == 9) {
		nextState = StateFactory::getMenuState();
	}
	*/
	return ReturnStateContext(nextState);
}

ErrorType CommunicationSettingState::onShutdown()
{
	return ErrorType();
}


