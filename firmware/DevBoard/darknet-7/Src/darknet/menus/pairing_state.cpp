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
	I_AM_ALICE_DISABLE_LISTEN,
	ALICE_INIT_CONVERSATION,
	ALICE_RECEIVE_ONE
};

PairingState::PairingState() : Darknet7BaseState()
	, BadgeList("Badge List:", Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0])))
	, Items(), ListBuffer(), InternalState(NONE), ESPRequestID(0), timesRunCalledSinceReset(0), TimeoutMS(1000), RetryCount(3), CurrentRetryCount(0)
	, TimeInState(0), TransmitInternalState(ALICE_INIT_CONVERSATION), ReceiveInternalState(BOB_WAITING_FOR_FIRST_TRANSMIT), bloop(0) {

}

PairingState::~PairingState() {

}

ErrorType PairingState::onInit() {
	//stop receiving
	//IRStopRX();
	InternalState = FETCHING_DATA;

	// set up defaults
	this->timesRunCalledSinceReset = 0;
	this->bloop = 0;
	CurrentRetryCount = 0;
	memset(&AIC, 0, sizeof(AIC));
	memset(&BRTI, 0, sizeof(BRTI));
	memset(&ATBS, 0, sizeof(ATBS));

	//TODO Build the STM to ESP Buffer
	flatbuffers::FlatBufferBuilder fbb;
	auto r = darknet7::CreateBLEScanForDevices(fbb, darknet7::BLEDeviceFilter_BADGE);
	// TODO Add the filter
	this->ESPRequestID = DarkNet7::get().nextSeq();
	auto e = darknet7::CreateSTMToESPRequest(fbb, ESPRequestID, darknet7::STMToESPAny_BLEScanForDevices, r.Union());
	darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);

	// Register a callback handler
	const MSGEvent<darknet7::BadgesInArea> * si = 0;
	MCUToMCU::get().getBus().addListener(this, si, &MCUToMCU::get());

	TransmitInternalState = ALICE_INIT_CONVERSATION;
	ReceiveInternalState = I_AM_ALICE_DISABLE_LISTEN;
	DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
	DarkNet7::get().getDisplay().drawString(5,10,(const char *)"Scanning for Badges", cmdc0de::RGBColor::BLUE);

	// Send the STMToESP Message
	MCUToMCU::get().send(fbb);
	return ErrorType();
}

void PairingState::receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BadgesInArea>* mevt) {
	// TODO
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
		MCUToMCU::get().getBus().removeListener(this, mevt, &MCUToMCU::get());
		InternalState = DISPLAY_DATA;
	}
	return;
}



//alice starts convo
//bob
StateBase::ReturnStateContext PairingState::onRun() {
	StateBase *nextState = this;
	if (InternalState == FETCHING_DATA)
	{
		if (this->timesRunCalledSinceReset > 500)
		{
			const MSGEvent<darknet7::BadgesInArea> *mevt=0;
			MCUToMCU::get().getBus().removeListener(this,mevt,&MCUToMCU::get());
			nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),DarkNet7::get().NO_DATA_FROM_ESP,2000);
		}
	}
	else
	{
		if (!GUIListProcessor::process(&BadgeList,(sizeof(Items) / sizeof(Items[0])))) {
			if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
				DarkNet7::get().getDisplay().fillScreen(cmdc0de::RGBColor::BLACK);
				switch(BadgeList.selectedItem) {
				default:
					nextState = this;
					break;
				}
			} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
				nextState = DarkNet7::get().getDisplayMenuState();
			}
		}
		DarkNet7::get().getGUI().drawList(&BadgeList);
	}
	this->timesRunCalledSinceReset += 1;
	return ReturnStateContext(nextState);
	/*
	static const char *msg1 = "Init convo complete.";
	static const char *msg2 = "Listening for Bob";
	static const char *msg3 = "Sent final msg to Bob";
	uint32_t bytesAvailable = 0;//IRBytesAvailable();
	if (TransmitInternalState == ALICE_INIT_CONVERSATION) {
		AIC.irmsgid = 1;
		memcpy(&AIC.AlicePublicKey[0], rc.getContactStore().getMyInfo().getCompressedPublicKey(),
				ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
		AIC.AliceRadioID = rc.getContactStore().getMyInfo().getUniqueID();
		strncpy(&AIC.AliceName[0], rc.getContactStore().getSettings().getAgentName(), sizeof(AIC.AliceName));
		IRTxBuff((uint8_t*) &AIC, sizeof(AIC));
		TransmitInternalState = ALICE_RECEIVE_ONE;
		rc.getDisplay().drawString(0,10,msg1);
		TimeInState = HAL_GetTick();
		//ok start recieving
		//IRStartRx();
	} else if (TransmitInternalState == ALICE_RECEIVE_ONE) {
		rc.getDisplay().drawString(0,10,msg1);
		rc.getDisplay().drawString(0,20,msg2);
		if (bytesAvailable >= 88) {
			uint8_t *buf = IRGetBuff();
			if (buf[0] == 2) {
				//first stop receiving
				//IRStopRX();
				BobReplyToInit *brti = (BobReplyToInit*) buf;
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
				if (uECC_verify(&uncompressedPublicKey[0], &msgHash[0], sizeof(msgHash), &brti->SignatureOfAliceData[0],
				THE_CURVE)) {

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

					IRTxBuff((uint8_t*) &ATBS, sizeof(ATBS));

					rc.getDisplay().drawString(0,30,msg3);
					ContactStore::Contact c;
					if(!rc.getContactStore().findContactByID(brti->BoBRadioID,c)) {
						//ok to add to contacts
						if (rc.getContactStore().addContact(brti->BoBRadioID, &brti->BobAgentName[0], &brti->BoBPublicKey[0],
								&brti->SignatureOfAliceData[0])) {

							sprintf(&displayBuf[0], "New Contact: %s", &brti->BobAgentName[0]);
							//StateFactory::getEventState()->addMessage(&displayBuf[0]);
						} else {
							sprintf(&displayBuf[0], "Failed to save contact (full?): %s", &brti->BobAgentName[0]);
							//StateFactory::getEventState()->addMessage(&displayBuf[0]);
						}
					} else {

					}
				} else {
					sprintf(&displayBuf[0], "Signature Check Failed with %s", &brti->BobAgentName[0]);
					//StateFactory::getEventState()->addMessage(&displayBuf[0]);
				}
				//IRStartRx();
				return ReturnStateContext(StateFactory::getDisplayMessageState(StateFactory::getMenuState(),&displayBuf[0],3000));
			}
			return ReturnStateContext(StateFactory::getMenuState());
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

void PairingState::ListenForAlice() {
	/*
	uint32_t bytesAvailable = 0;//IRBytesAvailable();
	if (ReceiveInternalState == BOB_WAITING_FOR_FIRST_TRANSMIT) {
		if (bytesAvailable >= 40) {
			//IRStopRX();
			uint8_t *buf = 0;//IRGetBuff();
			if (buf[0] == 1) {
				AliceInitConvo *aic = (AliceInitConvo*) buf;
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
				uECC_sign_deterministic(rc.getContactStore().getMyInfo().getPrivateKey(), message_hash,
						sizeof(message_hash), &ctx.uECC, signature, THE_CURVE);
				BRTI.irmsgid = 2;
				BRTI.BoBRadioID = rc.getContactStore().getMyInfo().getUniqueID();
				memcpy(&BRTI.BoBPublicKey[0], rc.getContactStore().getMyInfo().getCompressedPublicKey(),
						sizeof(BRTI.BoBPublicKey));
				strncpy(&BRTI.BobAgentName[0], rc.getContactStore().getSettings().getAgentName(),
						sizeof(BRTI.BobAgentName));
				memcpy(&BRTI.SignatureOfAliceData[0], &signature[0], sizeof(BRTI.SignatureOfAliceData));
				IRTxBuff((uint8_t*) &BRTI, sizeof(BRTI));
				ReceiveInternalState = BOB_WAITING_FOR_SECOND_TRANSMIT;
				TimeInState = HAL_GetTick();
			}
			IRStartRx();
		} else {
			//reset buffers!
			ReceiveInternalState = BOB_WAITING_FOR_FIRST_TRANSMIT;
		}
	} else if (ReceiveInternalState == BOB_WAITING_FOR_SECOND_TRANSMIT) {
		if (bytesAvailable >= sizeof(AliceToBobSignature)) {
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
	} else {
		//I_AM_ALICE_DISABLE_LISTEN:
		//break;
	}
	*/

}

void PairingState::BeTheBob() {
	ReceiveInternalState = BOB_WAITING_FOR_FIRST_TRANSMIT;
	//IRStartRx();
}

ErrorType PairingState::onShutdown() {
	//go back to listening for alice
	BeTheBob();
	return ErrorType();
}
