#include "pairing_state.h"
#include "../darknet7.h"
#include <tim.h>
#include "../libstm32/crypto/crypto_helper.h"
#include "../messaging/stm_to_esp_generated.h"
#include "../messaging/esp_to_stm_generated.h"
#include "gui_list_processor.h"
#include "menu_state.h"

using cmdc0de::ErrorType;
using cmdc0de::StateBase;

enum {
	BOB_WAITING_FOR_FIRST_TRANSMIT,
	BOB_WAITING_FOR_SECOND_TRANSMIT,
};

PairingState::PairingState() : Darknet7BaseState()
	, BadgeList("Badge List:", Items, 0, 0, 160, 128, 0, (sizeof(Items) / sizeof(Items[0])))
	, Items(), ListBuffer(), AddressBuffer(), MesgBuf(), MesgLen(), InternalState(NONE), ESPRequestID(0)
	, timesRunCalledSinceReset(0), TimeoutMS(1000), RetryCount(3), CurrentRetryCount(0)
	, TimeInState(0), msgId(1), gotBadgeList(false), securityConfirmed(false), bleConnected(false)
	, isAlice(false){

}

PairingState::~PairingState() {

}

ErrorType PairingState::onInit() {
	InternalState = FETCHING_DATA;

	// set up defaults
	this->timesRunCalledSinceReset = 0;
	this->msgId = 1;
	this->gotBadgeList = false;
	this->securityConfirmed = false;
	this->bleConnected = false;
	this->isAlice = false;
	CurrentRetryCount = 0;
	memset(&AIC, 0, sizeof(AIC));
	memset(&BRTI, 0, sizeof(BRTI));
	memset(&ATBS, 0, sizeof(ATBS));

	flatbuffers::FlatBufferBuilder fbb;
	auto r = darknet7::CreateBLEScanForDevices(fbb, darknet7::BLEDeviceFilter_BADGE);
	this->ESPRequestID = DarkNet7::get().nextSeq();
	auto e = darknet7::CreateSTMToESPRequest(fbb, ESPRequestID, darknet7::STMToESPAny_BLEScanForDevices, r.Union());
	darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);

	// Register a callback handler
	const MSGEvent<darknet7::BadgesInArea> * si = 0;
	MCUToMCU::get().getBus().addListener(this, si, &MCUToMCU::get());

	const MSGEvent<darknet7::BLESecurityConfirm> * si2 = 0;
	MCUToMCU::get().getBus().addListener(this, si2, &MCUToMCU::get());

	const MSGEvent<darknet7::BLEConnected> * si3 = 0;
	MCUToMCU::get().getBus().addListener(this, si3, &MCUToMCU::get());

	const MSGEvent<darknet7::BLEMessageFromDevice> * dev2 = 0;
	MCUToMCU::get().getBus().addListener(this, dev2, &MCUToMCU::get());

	const MSGEvent<darknet7::BLEPairingComplete> * si5 = 0;
	MCUToMCU::get().getBus().addListener(this, si5, &MCUToMCU::get());

	DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
	DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Scanning for Badges", cmdc0de::RGBColor::BLUE);

	// Send the STMToESP Message
	MCUToMCU::get().send(fbb);
	return ErrorType();
}

void PairingState::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BadgesInArea>* mevt) {
	this->gotBadgeList = true;
	DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
	const flatbuffers::Vector<flatbuffers::Offset<darknet7::Badge>>* badges;
	if (mevt->RequestID == this->ESPRequestID)
	{
		if(this->InternalState == FETCHING_DATA)
		{
			badges = mevt->InnerMsg->BadgeList();
			unsigned int len = badges->Length();
			for (unsigned int i = 0; i < len; i++)
			{
				const darknet7::Badge* badge = badges->Get(i);
				sprintf(&this->ListBuffer[i][0], "%s", badge->name()->c_str());
				sprintf(&this->AddressBuffer[i][0], "%s", badge->address()->c_str());
				Items[i].text = &this->ListBuffer[i][0];
				Items[i].id = i;
				Items[i].setShouldScroll();
			}
			DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
			DarkNet7::get().getGUI().drawList(&BadgeList);

			InternalState = DISPLAY_DATA;
			this->timesRunCalledSinceReset = 0;
		}
	}
	return;
}

void PairingState::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BLESecurityConfirm>* mevt)
{
	InternalState = CONNECTING;
	return;
}

void PairingState::receiveSignal(MCUToMCU*, const MSGEvent<darknet7::BLEConnected>* mevt) {
	DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
	bool success = mevt->InnerMsg->success();
	if(!success)
		InternalState = PAIRING_FAILED;
	this->isAlice = mevt->InnerMsg->isAlice();
	this->msgId = 1;
	if (this->isAlice)
		InternalState = ALICE_SEND_ONE; // Alice Sends First
	else
		InternalState = RECEIVE_DATA; // Bob Receives first

	this->timesRunCalledSinceReset = 0;
	return;
}

void PairingState::receiveSignal(MCUToMCU*, const MSGEvent<darknet7::BLEMessageFromDevice>* mevt) {
	const flatbuffers::String* tmesg = mevt->InnerMsg->data();
	memset(&this->MesgBuf, 0, sizeof(MesgBuf));
	this->MesgLen = tmesg->Length();
	memcpy(&this->MesgBuf, tmesg->c_str(), this->MesgLen);
	this->MesgBuf[MesgLen] = 0x0;
	AliceInitConvo* AIC = (AliceInitConvo*) &this->MesgBuf;
	if (AIC->irmsgid == ALICE_SEND_ONE)
		InternalState = BOB_SEND_ONE;
	else if (AIC->irmsgid == ALICE_SEND_TWO)
		InternalState = BOB_SEND_TWO;
	else if (AIC->irmsgid == BOB_SEND_ONE)
		InternalState = ALICE_SEND_TWO;
	else
		InternalState = PAIRING_FAILED;
	this->msgId = 2;
	this->timesRunCalledSinceReset = 0;
	return;
}

void PairingState::receiveSignal(MCUToMCU*, const MSGEvent<darknet7::BLEPairingComplete>* mevt) {
	InternalState = PAIRING_COMPLETE;
	this->timesRunCalledSinceReset = 0;
	return;
}

void PairingState::CleanUp()
{
	const MSGEvent<darknet7::BadgesInArea> *mevt=0;
	MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
	const MSGEvent<darknet7::BLESecurityConfirm> * si2 = 0;
	MCUToMCU::get().getBus().removeListener(this, si2, &MCUToMCU::get());
	const MSGEvent<darknet7::BLEConnected> * si3 = 0;
	MCUToMCU::get().getBus().removeListener(this, si3, &MCUToMCU::get());
	const MSGEvent<darknet7::BLEMessageFromDevice> * si4 = 0;
	MCUToMCU::get().getBus().removeListener(this, si4, &MCUToMCU::get());
	const MSGEvent<darknet7::BLEPairingComplete> * si5 = 0;
	MCUToMCU::get().getBus().removeListener(this, si5, &MCUToMCU::get());
}

StateBase::ReturnStateContext PairingState::onRun() {
	StateBase *nextState = this;
	flatbuffers::FlatBufferBuilder fbb;
	if (InternalState == FETCHING_DATA)
	{
		if (this->timesRunCalledSinceReset > 500)
		{
			this->CleanUp();
			nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),DarkNet7::get().NO_DATA_FROM_ESP,2000);
		}
	}
	else if (InternalState == DISPLAY_DATA)
	{
		if (!GUIListProcessor::process(&BadgeList,(sizeof(Items) / sizeof(Items[0])))) {
			if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1))
			{
				// send connect message with address
				auto sdata = fbb.CreateString((char *)&this->AddressBuffer[BadgeList.selectedItem][0], 17);
				auto r = darknet7::CreateBLEPairWithDevice(fbb, sdata);
				this->ESPRequestID = DarkNet7::get().nextSeq();
				auto e = darknet7::CreateSTMToESPRequest(fbb, ESPRequestID, darknet7::STMToESPAny_BLEPairWithDevice, r.Union());
				darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);

				InternalState = INITIATING_CONNECTION;
				MCUToMCU::get().send(fbb); // send the connect message
				this->timesRunCalledSinceReset = 0;
				nextState = this;
			}
			else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID))
			{
				this->CleanUp();
				nextState = DarkNet7::get().getDisplayMenuState();
			}
		}
		DarkNet7::get().getGUI().drawList(&BadgeList);
	}
	else if (InternalState == INITIATING_CONNECTION)
	{
		DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
		DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Connecting", cmdc0de::RGBColor::BLUE);
		if (this->timesRunCalledSinceReset > 500)
			this->InternalState = PAIRING_FAILED;
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
			DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);;

			InternalState = CONFIRMING;

			//Accept
			auto r = darknet7::CreateBLESendPINConfirmation(fbb, darknet7::RESPONSE_SUCCESS_True);
			auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendPINConfirmation, r.Union());
			darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
			MCUToMCU::get().send(fbb); // send the connect message
		}
	}
	else if (InternalState == CONFIRMING)
	{
		DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
		DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Confirming", cmdc0de::RGBColor::BLUE);
	}
	else if (InternalState == ALICE_SEND_ONE)
	{
		DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
		DarkNet7::get().getDisplay().drawString(5,20,(const char *)"Waiting", cmdc0de::RGBColor::BLUE);
		// Wait for 1/2 a second until BOB is probably set up
		HAL_Delay(1000);
		DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
		DarkNet7::get().getDisplay().drawString(5,20,(const char *)"Alice Send 1", cmdc0de::RGBColor::BLUE);
		//Make the Message
		AIC.irmsgid = ALICE_SEND_ONE;
		memcpy(&AIC.AlicePublicKey[0], DarkNet7::get().getContacts().getMyInfo().getCompressedPublicKey(),
				ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
		AIC.AliceRadioID = DarkNet7::get().getContacts().getMyInfo().getUniqueID();
		strncpy(&AIC.AliceName[0], DarkNet7::get().getContacts().getSettings().getAgentName(), sizeof(AIC.AliceName));

		//Send the message
		auto sdata = fbb.CreateString((char*)&AIC, sizeof(AIC));
		auto r = darknet7::CreateBLESendDataToDevice(fbb, sdata);
		auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendDataToDevice, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);

		InternalState = RECEIVE_DATA;
		nextState = this;
	}
	else if (InternalState == ALICE_SEND_TWO)
	{
		DarkNet7::get().getDisplay().drawString(5,40,(const char *)"Alice Data 2", cmdc0de::RGBColor::BLUE);

		BobReplyToInit *brti = (BobReplyToInit*) MesgBuf;
		//using signature validate our data that bob signed
		uint8_t uncompressedPublicKey[ContactStore::PUBLIC_KEY_LENGTH];
		uECC_decompress(&brti->BoBPublicKey[0], &uncompressedPublicKey[0], THE_CURVE);
		uint8_t msgHash[SHA256_HASH_SIZE];
		ShaOBJ msgHashCtx;
		sha256_init(&msgHashCtx);
		uint16_t radioID = DarkNet7::get().getContacts().getMyInfo().getUniqueID();
		sha256_add(&msgHashCtx, (uint8_t*) &radioID, sizeof(uint16_t));
		sha256_add(&msgHashCtx, (uint8_t*) DarkNet7::get().getContacts().getMyInfo().getCompressedPublicKey(),
				ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
		sha256_digest(&msgHashCtx, &msgHash[0]);

		ATBS.irmsgid = ALICE_SEND_TWO;
		if (uECC_verify(&uncompressedPublicKey[0], &msgHash[0], sizeof(msgHash), &brti->SignatureOfAliceData[0], THE_CURVE))
		{
			uint8_t message_hash[SHA256_HASH_SIZE];
			ShaOBJ messageHashCtx;
			sha256_init(&messageHashCtx);
			sha256_add(&messageHashCtx, (uint8_t*) &brti->BoBRadioID, sizeof(brti->BoBRadioID));
			sha256_add(&messageHashCtx, (uint8_t*) &brti->BoBPublicKey[0], sizeof(brti->BoBPublicKey));
			sha256_digest(&messageHashCtx, &message_hash[0]);
			uint8_t tmp[32 + 32 + 64];
			ATBS.irmsgid = ALICE_SEND_TWO;
			SHA256_HashContext ctx = { { &init_SHA256, &update_SHA256, &finish_SHA256, 64, 32, tmp } };
			uECC_sign_deterministic((const unsigned char*)DarkNet7::get().getContacts().getMyInfo().getPrivateKey(),
					message_hash, sizeof(message_hash), &ctx.uECC, &ATBS.signature[0], THE_CURVE);

			//Add to contacts
			ContactStore::Contact c;
			if(!DarkNet7::get().getContacts().findContactByID(brti->BoBRadioID,c)) {
				DarkNet7::get().getContacts().addContact(brti->BoBRadioID, &brti->BobAgentName[0],
						&brti->BoBPublicKey[0], &brti->SignatureOfAliceData[0]);
			}
		}
		else
		{
		//	sprintf(&displayBuf[0], "Signature Check Failed with %s", &brti->BobAgentName[0]);
		}

		auto sdata2 = fbb.CreateString((char *)&ATBS, sizeof(ATBS));
		auto r = darknet7::CreateBLESendDataToDevice(fbb, sdata2);
		auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendDataToDevice, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);

		InternalState = RECEIVE_DATA;
	}
	else if (InternalState == BOB_SEND_ONE)
	{
		DarkNet7::get().getDisplay().drawString(5,20,(const char *)"BOB Send 1", cmdc0de::RGBColor::BLUE);

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
		BRTI.irmsgid = BOB_SEND_ONE;
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

		InternalState = RECEIVE_DATA;
	}
	else if (InternalState == BOB_SEND_TWO)
	{
		AliceToBobSignature *atbs = (AliceToBobSignature*) &this->MesgBuf;
		uint8_t uncompressedPublicKey[ContactStore::PUBLIC_KEY_LENGTH];
		uECC_decompress(&AIC.AlicePublicKey[0], &uncompressedPublicKey[0], THE_CURVE);
		uint8_t msgHash[SHA256_HASH_SIZE];
		ShaOBJ msgHashCtx;
		sha256_init(&msgHashCtx);
		uint16_t radioID = DarkNet7::get().getContacts().getMyInfo().getUniqueID();
		sha256_add(&msgHashCtx, (uint8_t*) &radioID, sizeof(uint16_t));
		sha256_add(&msgHashCtx, (uint8_t*) DarkNet7::get().getContacts().getMyInfo().getCompressedPublicKey(),
		ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
		//verify alice's signature of my public key and unique id
		sha256_digest(&msgHashCtx, &msgHash[0]);
		if (uECC_verify(&uncompressedPublicKey[0], &msgHash[0], sizeof(msgHash), &atbs->signature[0], THE_CURVE))
		{
			ContactStore::Contact c;
			if ((DarkNet7::get().getContacts().findContactByID(AIC.AliceRadioID,c) ||
				DarkNet7::get().getContacts().addContact(AIC.AliceRadioID, &AIC.AliceName[0], &AIC.AlicePublicKey[0], &atbs->signature[0])))
			{
				DarkNet7::get().getDisplay().drawString(5,40,(const char *)"BOB Send Complete", cmdc0de::RGBColor::BLUE);
				auto r = darknet7::CreateBLESendDNPairComplete(fbb);
				auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendDNPairComplete, r.Union());
				darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
				MCUToMCU::get().send(fbb);
				InternalState = PAIRING_SUCCESS; // Don't initiate the disconnect as Bob
			}
			else
				InternalState = PAIRING_FAILED;
		}
		else
			InternalState = PAIRING_FAILED;
	}
	else if (InternalState == RECEIVE_DATA)
	{
		if (this->isAlice)
		{
			if (this->msgId == 1)
				DarkNet7::get().getDisplay().drawString(5,30,(const char *)"Alice Receive 1", cmdc0de::RGBColor::BLUE);
			else
				DarkNet7::get().getDisplay().drawString(5,50,(const char *)"Alice Receive 2", cmdc0de::RGBColor::BLUE);
			if (this->timesRunCalledSinceReset > 500)
				InternalState = PAIRING_FAILED;
		}
		else
		{
			if (this->msgId == 1)
				DarkNet7::get().getDisplay().drawString(5,10,(const char *)"BOB Receiving 1", cmdc0de::RGBColor::BLUE);
			else if (this->msgId == 2)
				DarkNet7::get().getDisplay().drawString(5,30,(const char *)"BOB Receiving 2", cmdc0de::RGBColor::BLUE);
			if(this->timesRunCalledSinceReset > 500)
				InternalState = PAIRING_FAILED;
		}
	}
	else if (InternalState == PAIRING_COMPLETE)
	{
		auto r = darknet7::CreateBLEDisconnect(fbb);
		this->ESPRequestID = DarkNet7::get().nextSeq();
		auto e = darknet7::CreateSTMToESPRequest(fbb, ESPRequestID, darknet7::STMToESPAny_BLEDisconnect, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);
		InternalState = PAIRING_SUCCESS;
	}
	else if (InternalState == PAIRING_SUCCESS)
	{
		// We don't want to disconnect as bob, Alice initiates that when she receives the final message. Report success
		this->CleanUp();
		nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(), DarkNet7::BLE_PAIRING_SUCCESS,2000);
	}
	else if (InternalState == PAIRING_FAILED)
	{
		auto r = darknet7::CreateBLEDisconnect(fbb);
		this->ESPRequestID = DarkNet7::get().nextSeq();
		auto e = darknet7::CreateSTMToESPRequest(fbb, ESPRequestID, darknet7::STMToESPAny_BLEDisconnect, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);
		this->CleanUp();
		nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),DarkNet7::get().BLE_PAIRING_FAILED,2000);
	}
	this->timesRunCalledSinceReset += 1;
	return ReturnStateContext(nextState);
}

ErrorType PairingState::onShutdown() {
	return ErrorType();
}
