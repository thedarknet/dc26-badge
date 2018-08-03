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
	const MSGEvent<darknet7::BLESecurityConfirm> * si = 0;
	MCUToMCU::get().getBus().removeListener(this,si,&MCUToMCU::get());
	InternalState = CONNECTING;
	this->loops = 0;
	return;
}

void PairWithState::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BLEConnected>* mevt)
{
	MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
	this->loops = 0;
	return;
}

void PairWithState::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BLEMessageFromDevice>* mevt)
{
	const MSGEvent<darknet7::BLEMessageFromDevice> * si = 0;
	MCUToMCU::get().getBus().removeListener(this,si,&MCUToMCU::get());
	//TODO: store the information locally
	const flatbuffers::String* tmesg = mevt->InnerMsg->data();
	this->MesgLen = tmesg->Length();
	memcpy(&this->MesgBuf, tmesg->c_str(), this->MesgLen);
	this->MesgBuf[MesgLen] = 0x0;
	if (this->aliceMessage == 1)
	{
		InternalState = BOB_SEND_ONE;
		this->aliceMessage = 2;
	}
	else if (this->aliceMessage == 2)
	{
		this->aliceMessage = 1;
		InternalState = BOB_SEND_TWO;
	}
	this->loops = 0;
	return;
}

PairWithState::PairWithState() : Darknet7BaseState(), InternalState(), MesgBuf(), MesgLen(), aliceMessage()
{
	this->loops = 0;
	this->InternalState = NONE;
	this->aliceMessage = 1;
}

PairWithState::~PairWithState()
{
	if (InternalState == CONNECTING)
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
	cmdc0de::StateBase *nextState = this;
	flatbuffers::FlatBufferBuilder fbb;
	if(DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID))
	{
		nextState = DarkNet7::get().getDisplayMenuState();
	}
	else if (InternalState == CONNECTING)
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
			DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
			const MSGEvent<darknet7::BLEConnected> * dev1 = 0;
			MCUToMCU::get().getBus().addListener(this, dev1, &MCUToMCU::get());
			const MSGEvent<darknet7::BLEMessageFromDevice> * dev2 = 0;
			MCUToMCU::get().getBus().addListener(this, dev2, &MCUToMCU::get());
			this->loops = 0;
			InternalState = BOB_RECEIVE;

			//Accept
			auto r = darknet7::CreateBLESendPINConfirmation(fbb, darknet7::RESPONSE_SUCCESS_True);
			auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendPINConfirmation, r.Union());
			darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
			MCUToMCU::get().send(fbb); // send the connect message
		}
	}
	else if (InternalState == BOB_RECEIVE)
	{
		if (aliceMessage == 1)
			DarkNet7::get().getDisplay().drawString(5,10,(const char *)"BOB Receiving 1", cmdc0de::RGBColor::BLUE);
		else if (aliceMessage == 2)
			DarkNet7::get().getDisplay().drawString(5,30,(const char *)"BOB Receiving 1", cmdc0de::RGBColor::BLUE);
		if(this->loops > 300)
		{
			const MSGEvent<darknet7::BLEMessageFromDevice> * ra1 = 0;
			MCUToMCU::get().getBus().removeListener(this, ra1, &MCUToMCU::get());
			InternalState = PAIRING_FAILED;
		}
	}
	else if (InternalState == BOB_SEND_ONE)
	{
		DarkNet7::get().getDisplay().drawString(5,20,(const char *)"BOB Sending First", cmdc0de::RGBColor::BLUE);
		const MSGEvent<darknet7::BLEMessageFromDevice> * alice2 = 0;
		MCUToMCU::get().getBus().addListener(this, alice2, &MCUToMCU::get());

		AliceInitConvo *aic = (AliceInitConvo*) MesgBuf;
		memcpy(&AIC,aic,sizeof(AIC));
		uint8_t message_hash[SHA256_HASH_SIZE];
		ShaOBJ messageHashCtx;
		sha256_init(&messageHashCtx);
		sha256_add(&messageHashCtx, (uint8_t*) &aic->AliceRadioID, sizeof(aic->AliceRadioID));
		sha256_add(&messageHashCtx, (uint8_t*) &aic->AlicePublicKey, sizeof(aic->AlicePublicKey));
		sha256_digest(&messageHashCtx, &message_hash[0]);
		uint8_t signature[ContactStore::SIGNATURE_LENGTH];

		uint8_t tmp[32 + 32 + 64];
		SHA256_HashContext ctx = { { &init_SHA256, &update_SHA256, &finish_SHA256, 64, 32, &tmp[0] } };
		uECC_sign_deterministic(DarkNet7::get().getContacts().getMyInfo().getPrivateKey(), message_hash,
				sizeof(message_hash), &ctx.uECC, signature, THE_CURVE);
		BRTI.irmsgid = 2;
		BRTI.BoBRadioID = DarkNet7::get().getContacts().getMyInfo().getUniqueID();
		memcpy(&BRTI.BoBPublicKey[0], DarkNet7::get().getContacts().getMyInfo().getCompressedPublicKey(),
				sizeof(BRTI.BoBPublicKey));
		strncpy(&BRTI.BobAgentName[0], DarkNet7::get().getContacts().getSettings().getAgentName(),
				sizeof(BRTI.BobAgentName));
		memcpy(&BRTI.SignatureOfAliceData[0], &signature[0], sizeof(BRTI.SignatureOfAliceData));

		// TODO: Get the data
		auto sdata = fbb.CreateString((char*)&BRTI, sizeof(BRTI));
		auto r = darknet7::CreateBLESendDataToDevice(fbb, sdata);
		auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendDataToDevice, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);

		InternalState = BOB_RECEIVE;
	}
	else if (InternalState == BOB_SEND_TWO)
	{
		DarkNet7::get().getDisplay().drawString(5,40,(const char *)"BOB Sending Complete", cmdc0de::RGBColor::BLUE);
		/*
		 *if (bytesAvailable >= sizeof(AliceToBobSignature)) {
			uint8_t *buf = 0;//IRGetBuff();
			if (buf[0] == 3) {
				//IRStopRX();
				AliceToBobSignature *atbs = (AliceToBobSignature*) buf;
				uint8_t uncompressedPublicKey[ContactStore::PUBLIC_KEY_LENGTH];
				uECC_decompress(&AIC.AlicePublicKey[0], &uncompressedPublicKey[0], THE_CURVE);
				uint8_t msgHash[SHA256_HASH_SIZE];
				ShaOBJ msgHashCtx;
				sha256_init(&msgHashCtx);
				uint16_t radioID = rc.getContactStore().getMyInfo().getUniqueID();
				sha256_add(&msgHashCtx, (uint8_t*) &radioID, sizeof(uint16_t));
				sha256_add(&msgHashCtx, (uint8_t*) rc.getContactStore().getMyInfo().getCompressedPublicKey(),
						ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
				//verify alice's signature of my public key and unique id
				sha256_digest(&msgHashCtx, &msgHash[0]);
				if (uECC_verify(&uncompressedPublicKey[0], &msgHash[0], sizeof(msgHash), &atbs->signature[0],
				THE_CURVE)) {
					ContactStore::Contact c;
					if(!rc.getContactStore().findContactByID(AIC.AliceRadioID,c)) {
						char displayBuf[24];
						//ok to add to contacts
						if (rc.getContactStore().addContact(AIC.AliceRadioID, &AIC.AliceName[0], &AIC.AlicePublicKey[0],
								&atbs->signature[0])) {
							sprintf(&displayBuf[0], "New Contact: %s", &AIC.AliceName[0]);
							//StateFactory::getEventState()->addMessage(&displayBuf[0]);
						} else {
							sprintf(&displayBuf[0], "Could not write contact. full?");
							//StateFactory::getEventState()->addMessage(&displayBuf[0]);
						}
					} else {
					}
				}
				//IRStartRx();
			} else {
				//reset buffers!
				//IRStopRX();
				//IRStartRx();
			}
			ReceiveInternalState = BOB_WAITING_FOR_FIRST_TRANSMIT;
		} else {
			if ((HAL_GetTick() - TimeInState) > TimeoutMS) {
				TransmitInternalState = BOB_WAITING_FOR_FIRST_TRANSMIT;
				//IRStopRX();
				//IRStartRx();
			}
		}
		 */
		auto r = darknet7::CreateBLESendDNPairComplete(fbb);
		auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendDNPairComplete, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);
		InternalState = PAIRING_SUCCESS;
	}
	else if (InternalState == PAIRING_SUCCESS)
	{
		// We don't want to disconnect, Alice initiates that when she receives the final message. Report success
		nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::BLE_PAIRING_SUCCESS,2000);
	}
	else if (InternalState == PAIRING_FAILED)
	{
		auto r = darknet7::CreateBLEDisconnect(fbb);
		auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLEDisconnect, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);
		nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::BLE_PAIRING_FAILED,2000);
	}
	else if(this->loops > 1000)
	{
		nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::NO_DATA_FROM_ESP,2000);
	}
	this->loops++;
	return ReturnStateContext(nextState);
}

cmdc0de::ErrorType PairWithState::onShutdown()
{
	return cmdc0de::ErrorType();
}
