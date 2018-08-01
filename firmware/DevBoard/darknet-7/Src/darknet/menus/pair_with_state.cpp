#include "pair_with_state.h"
#include "../darknet7.h"
#include "darknet7_base_state.h"
#include <tim.h>
#include "../libstm32/crypto/crypto_helper.h"
#include "../messaging/stm_to_esp_generated.h"
#include "../messaging/esp_to_stm_generated.h"
#include "gui_list_processor.h"
#include "menu_state.h"

void PairWithState::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BLESecurityConfirm>* mevt)
{
	MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
	InternalState = PAIRING;
	return;
}

PairWithState::PairWithState() : Darknet7BaseState(), InternalState()
{
	this->loops = 0;
	this->InternalState = NONE;
}

PairWithState::~PairWithState()
{
	if (InternalState == PAIRING)
	{
		const MSGEvent<darknet7::BLESecurityConfirm> * si = 0;
		MCUToMCU::get().getBus().removeListener(this,si,&MCUToMCU::get());
	}
}

cmdc0de::ErrorType PairWithState::onInit()
{
	this->loops = 0;
	this->InternalState = NONE;
	DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
	DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Waiting for Connection", cmdc0de::RGBColor::BLUE);
	const MSGEvent<darknet7::BLESecurityConfirm> * si = 0;
	MCUToMCU::get().getBus().addListener(this, si, &MCUToMCU::get());
	return cmdc0de::ErrorType();
}

cmdc0de::StateBase::ReturnStateContext PairWithState::onRun()
{
	StateBase *nextState = this;
	flatbuffers::FlatBufferBuilder fbb;
	if(this->loops > 1000)
	{
		nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::NO_DATA_FROM_ESP,2000);
	}
	else if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID))
	{
		nextState = DarkNet7::get().getDisplayMenuState();
	}
	else if (InternalState == PAIRING)
	{
		DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
		DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Connect with This Badge?", cmdc0de::RGBColor::BLUE);
		DarkNet7::get().getDisplay().drawString(5,20,(const char *)"Verify Number On Second Screen", cmdc0de::RGBColor::BLUE);
		DarkNet7::get().getDisplay().drawString(5,30,(const char *)"Fire1: YES", cmdc0de::RGBColor::BLUE);
		DarkNet7::get().getDisplay().drawString(5,40,(const char *)"MID  : NO", cmdc0de::RGBColor::BLUE);
		if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID))
		{
			auto r = darknet7::CreateBLESendPINConfirmation(fbb, darknet7::RESPONSE_SUCCESS_False);
			auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendPINConfirmation, r.Union());
			darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
			nextState = DarkNet7::get().getDisplayMenuState();
			MCUToMCU::get().send(fbb); // send the connect message
		}
		else if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1))
		{
			//Accept
			auto r = darknet7::CreateBLESendPINConfirmation(fbb, darknet7::RESPONSE_SUCCESS_True);
			auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendPINConfirmation, r.Union());
			darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
			nextState = DarkNet7::get().getDisplayMenuState();
			// TODO: Maybe had an intermediate state that says whether pairing was successful
			MCUToMCU::get().send(fbb); // send the connect message
		}
	}
	this->loops++;
	return ReturnStateContext(nextState);
}

cmdc0de::ErrorType PairWithState::onShutdown()
{
	return cmdc0de::ErrorType();
}
