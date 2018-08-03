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
	, TimeInState(0), bobMessage(1) {

}

PairingState::~PairingState() {

}

ErrorType PairingState::onInit() {
	InternalState = FETCHING_DATA;

	// set up defaults
	this->timesRunCalledSinceReset = 0;
	this->bobMessage = 1;
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

	DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
	DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Scanning for Badges", cmdc0de::RGBColor::BLUE);

	// Send the STMToESP Message
	MCUToMCU::get().send(fbb);
	return ErrorType();
}

void PairingState::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BadgesInArea>* mevt) {
	DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
	const flatbuffers::Vector<flatbuffers::Offset<darknet7::Badge>>* badges;
	if (mevt->RequestID == this->ESPRequestID)
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

		const MSGEvent<darknet7::BadgesInArea> * si = 0;
		MCUToMCU::get().getBus().removeListener(this, si, &MCUToMCU::get());

		InternalState = DISPLAY_DATA;
		this->timesRunCalledSinceReset = 0;
	}
	return;
}

void PairingState::receiveSignal(MCUToMCU*, const MSGEvent<darknet7::BLEConnected>* mevt) {
	MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
	InternalState = ALICE_SEND_ONE;
	this->timesRunCalledSinceReset = 0;
	return;
}

void PairingState::receiveSignal(MCUToMCU*, const MSGEvent<darknet7::BLEMessageFromDevice>* mevt) {
	MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
	const flatbuffers::String* tmesg = mevt->InnerMsg->data();
	this->MesgLen = tmesg->Length();
	memcpy(&this->MesgBuf, tmesg->c_str(), this->MesgLen);
	this->MesgBuf[MesgLen] = 0x0;
	this->bobMessage = 2;
	InternalState = ALICE_SEND_TWO;
	this->timesRunCalledSinceReset = 0;
	return;
}

void PairingState::receiveSignal(MCUToMCU*, const MSGEvent<darknet7::BLEPairingComplete>* mevt) {
	MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
	this->bobMessage = 1;
	InternalState = PAIRING_COMPLETE;
	this->timesRunCalledSinceReset = 0;
	return;
}

StateBase::ReturnStateContext PairingState::onRun() {
	StateBase *nextState = this;
	flatbuffers::FlatBufferBuilder fbb;
	if (InternalState == FETCHING_DATA)
	{
		if (this->timesRunCalledSinceReset > 500)
		{
			const MSGEvent<darknet7::BadgesInArea> *mevt=0;
			MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
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

				// Register for a callback
				const MSGEvent<darknet7::BLEConnected> * blec = 0;
				MCUToMCU::get().getBus().addListener(this, blec, &MCUToMCU::get());

				InternalState = CONNECTING;
				MCUToMCU::get().send(fbb); // send the connect message
				this->timesRunCalledSinceReset = 0;
				nextState = this;
			}
			else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID))
			{
				nextState = DarkNet7::get().getDisplayMenuState();
			}
			else
				DarkNet7::get().getGUI().drawList(&BadgeList);
		}
		else
			DarkNet7::get().getGUI().drawList(&BadgeList);
	}
	else if (InternalState == CONNECTING)
	{
		DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
		DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Connecting", cmdc0de::RGBColor::BLUE);
		if (this->timesRunCalledSinceReset > 500)
		{
			const MSGEvent<darknet7::BLEConnected> *blecon=0;
			MCUToMCU::get().getBus().removeListener(this,blecon,&MCUToMCU::get());
			nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),DarkNet7::get().NO_DATA_FROM_ESP,2000);
		}
	}
	else if (InternalState == ALICE_SEND_ONE)
	{
		DarkNet7::get().getDisplay().drawString(5,20,(const char *)"Alice Send 1", cmdc0de::RGBColor::BLUE);
		// Add the listener for Bob Message 1
		const MSGEvent<darknet7::BLEMessageFromDevice> * frombob = 0;
		MCUToMCU::get().getBus().addListener(this, frombob, &MCUToMCU::get());

		//Make the Message
		AIC.irmsgid = 0;
		memcpy(&AIC.AlicePublicKey[0], "abcdefghijklmnopqrstuvwxy", //rc.getContactStore().getMyInfo().getCompressedPublicKey(),
				ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
		AIC.AliceRadioID = 0x1234; //rc.getContactStore().getMyInfo().getUniqueID();
		strncpy(&AIC.AliceName[0], (const char*)"12345678901\0", 12); // rc.getContactStore().getSettings().getAgentName(), sizeof(AIC.AliceName));
		auto sdata = fbb.CreateString((char*)&AIC.irmsgid, 40);

		//Send the message
		auto r = darknet7::CreateBLESendDataToDevice(fbb, sdata);
		auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendDataToDevice, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);

		InternalState = ALICE_RECEIVE;
		nextState = this;
	}
	else if (InternalState == ALICE_RECEIVE)
	{
		if (this->bobMessage == 1)
			DarkNet7::get().getDisplay().drawString(5,30,(const char *)"Alice Receive 1", cmdc0de::RGBColor::BLUE);
		else
			DarkNet7::get().getDisplay().drawString(5,50,(const char *)"Alice Receive 2", cmdc0de::RGBColor::BLUE);
		if (this->timesRunCalledSinceReset > 500)
		{
			if (this->bobMessage == 1)
			{
				const MSGEvent<darknet7::BLEMessageFromDevice> *removebob=0;
				MCUToMCU::get().getBus().removeListener(this,removebob,&MCUToMCU::get());
			}
			else if (this->bobMessage == 2)
			{
				const MSGEvent<darknet7::BLEPairingComplete> *removebob=0;
				MCUToMCU::get().getBus().removeListener(this,removebob,&MCUToMCU::get());
			}
			InternalState = PAIRING_FAILED;
		}
	}
	else if (InternalState == ALICE_SEND_TWO)
	{
		const MSGEvent<darknet7::BLEPairingComplete> *complete = 0;
		MCUToMCU::get().getBus().addListener(this,complete,&MCUToMCU::get());
		DarkNet7::get().getDisplay().drawString(5,40,(const char *)"Sending Data 2", cmdc0de::RGBColor::BLUE);

		// MesgBuf && MesgLen
		/*
		BobReplyToInit *brti = (BobReplyToInit*) MesgBuf;
		//using signature validate our data that bob signed
		uint8_t uncompressedPublicKey[ContactStore::PUBLIC_KEY_LENGTH];
		uECC_decompress(&brti->BoBPublicKey[0], &uncompressedPublicKey[0], THE_CURVE);
		uint8_t msgHash[SHA256_HASH_SIZE];
		ShaOBJ msgHashCtx;
		sha256_init(&msgHashCtx);
		uint16_t radioID = rc.getContactStore().getMyInfo().getUniqueID();
		sha256_add(&msgHashCtx, (uint8_t*) &radioID, sizeof(uint16_t));
		sha256_add(&msgHashCtx, (uint8_t*) rc.getContactStore().getMyInfo().getCompressedPublicKey(),
				ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
		sha256_digest(&msgHashCtx, &msgHash[0]);
		char displayBuf[24];
		if (uECC_verify(&uncompressedPublicKey[0], &msgHash[0], sizeof(msgHash), &brti->SignatureOfAliceData[0], THE_CURVE))
		{
			uint8_t message_hash[SHA256_HASH_SIZE];
			ShaOBJ messageHashCtx;
			sha256_init(&messageHashCtx);
			sha256_add(&messageHashCtx, (uint8_t*) &brti->BoBRadioID, sizeof(brti->BoBRadioID));
			sha256_add(&messageHashCtx, (uint8_t*) &brti->BoBPublicKey[0], sizeof(brti->BoBPublicKey));
			sha256_digest(&messageHashCtx, &message_hash[0]);
			uint8_t tmp[32 + 32 + 64];
			ATBS.irmsgid = 3;
			SHA256_HashContext ctx = { { &init_SHA256, &update_SHA256, &finish_SHA256, 64, 32, tmp } };
			uECC_sign_deterministic(rc.getContactStore().getMyInfo().getPrivateKey(), message_hash,
					sizeof(message_hash), &ctx.uECC, &ATBS.signature[0], THE_CURVE);

			//Add to contacts
			if(!rc.getContactStore().findContactByID(brti->BoBRadioID,c)) {
				rc.getContactStore().addContact(brti->BoBRadioID, &brti->BobAgentName[0], &brti->BoBPublicKey[0], &brti->SignatureOfAliceData[0]);
			}
		} else {
			sprintf(&displayBuf[0], "Signature Check Failed with %s", &brti->BobAgentName[0]);
		}
		*/

		auto sdata2 = fbb.CreateString((char *)"bbcdefghijklmnopqrstuvwxyz12345678901234567890ABC", 49);
		auto r = darknet7::CreateBLESendDataToDevice(fbb, sdata2);
		auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESendDataToDevice, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);

		InternalState = ALICE_RECEIVE;
	}
	else if (InternalState == PAIRING_COMPLETE)
	{
		const MSGEvent<darknet7::BLEMessageFromDevice> * si = 0;
		MCUToMCU::get().getBus().removeListener(this,si,&MCUToMCU::get());
		auto r = darknet7::CreateBLEDisconnect(fbb);
		this->ESPRequestID = DarkNet7::get().nextSeq();
		auto e = darknet7::CreateSTMToESPRequest(fbb, ESPRequestID, darknet7::STMToESPAny_BLEDisconnect, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);
		nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),DarkNet7::get().BLE_PAIRING_SUCCESS,2000);
	}
	else if (InternalState == PAIRING_FAILED)
	{
		const MSGEvent<darknet7::BLEMessageFromDevice> * si = 0;
		MCUToMCU::get().getBus().removeListener(this,si,&MCUToMCU::get());
		auto r = darknet7::CreateBLEDisconnect(fbb);
		this->ESPRequestID = DarkNet7::get().nextSeq();
		auto e = darknet7::CreateSTMToESPRequest(fbb, ESPRequestID, darknet7::STMToESPAny_BLEDisconnect, r.Union());
		darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
		MCUToMCU::get().send(fbb);
		nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),DarkNet7::get().BLE_PAIRING_FAILED,2000);
	}
	this->timesRunCalledSinceReset += 1;
	return ReturnStateContext(nextState);
	/*
	static const char *msg1 = "Init convo complete.";
	static const char *msg2 = "Listening for Bob";
	static const char *msg3 = "Sent final msg to Bob";
	uint32_t bytesAvailable = 0;//IRBytesAvailable();
	if (TransmitInternalState == ALICE_INIT_CONVERSATION) {

	} else if (TransmitInternalState == ALICE_RECEIVE_ONE) {

		}
		if ((HAL_GetTick() - TimeInState) > TimeoutMS) {
			CurrentRetryCount++;
			TransmitInternalState = ALICE_INIT_CONVERSATION;
			IRStopRX();
			if (CurrentRetryCount >= RetryCount) {
				return ReturnStateContext(
						StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Failed to pair", 5000));
			}
		}
	}
	*/
	//return ReturnStateContext(this);
}

ErrorType PairingState::onShutdown() {
	return ErrorType();
}
