#ifndef PAIRWITH_MENU_H_
#define PAIRWITH_MENU_H_

#include "darknet7_base_state.h"
#include "../mcu_to_mcu.h"
#include "../KeyStore.h"

class PairWithState : public Darknet7BaseState {
public:
	struct AliceInitConvo {
		uint8_t irmsgid;
		uint8_t AlicePublicKey[ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH];
		uint16_t AliceRadioID;
		char AliceName[ContactStore::AGENT_NAME_LENGTH];
	};

	struct BobReplyToInit {
		uint8_t irmsgid;
		uint8_t BoBPublicKey[ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH];
		uint16_t BoBRadioID;
		char BobAgentName[ContactStore::AGENT_NAME_LENGTH];
		uint8_t SignatureOfAliceData[ContactStore::SIGNATURE_LENGTH];
	};

	struct AliceToBobSignature {
		uint8_t irmsgid;
		uint8_t signature[48];
	};
public:
	PairWithState();
	virtual ~PairWithState();
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BLESecurityConfirm>* mevt);
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BLEConnected>* mevt);
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BLEMessageFromDevice>* mevt);
protected:
	enum INTERNAL_STATE { NONE, CONNECTING, BOB_RECEIVE, BOB_SEND_ONE, BOB_SEND_TWO, PAIRING_SUCCESS, PAIRING_FAILED};
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	uint32_t loops = 0;
	INTERNAL_STATE InternalState;
	char MesgBuf[200];
	unsigned int MesgLen;
	unsigned char aliceMessage = 1;
	AliceInitConvo AIC;
	BobReplyToInit BRTI;
	AliceToBobSignature ATBS;
};

#endif
