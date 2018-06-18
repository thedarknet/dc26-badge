#ifndef IRMENU_H_
#define IRMENU_H

#include "darknet7_base_state.h"
#include "../KeyStore.h"

class PairingState: public Darknet7BaseState {
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
	PairingState();
	virtual ~PairingState();
	void ListenForAlice();
	void BeTheBob();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	uint16_t TimeoutMS;
	uint8_t RetryCount;
	uint8_t CurrentRetryCount;
	uint32_t TimeInState;
	AliceInitConvo AIC;
	BobReplyToInit BRTI;
	AliceToBobSignature ATBS;
	uint16_t TransmitInternalState;
	uint16_t ReceiveInternalState;
};

#endif

