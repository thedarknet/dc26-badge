#ifndef SEND_MSG_STATE_H
#define SEND_MSG_STATE_H

#include "darknet7_base_state.h"

class SendMsgState: public Darknet7BaseState {
public:
	static const uint16_t NO_CONTACT = 0xFFFF;
	enum INTERNAL_STATE {
		TYPE_MESSAGE, CONFIRM_SEND, SENDING
	};
	SendMsgState();
	virtual ~SendMsgState();
	void setContactToMessage(const uint16_t radioID, const char *agentName);
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	uint16_t RadioID;
	const char *AgentName;
	char MsgBuffer[60];
	INTERNAL_STATE InternalState;

};


#endif
