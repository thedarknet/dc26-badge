#ifndef EVENT_STATE_H
#define EVENT_STATE_H

#include "darknet7_base_state.h"

class MessageState: public Darknet7BaseState {
public:
	static const uint16_t MAX_MSG_LEN = 64;
public:
	struct RadioMessage {
		char Msg[MAX_MSG_LEN];
		uint16_t FromUID;
		int8_t Rssi;
		RadioMessage();
	};
	enum {
		MESSAGE_LIST, DETAIL
	};
public:
	MessageState();
	virtual ~MessageState();
	void addMsg(const char *msg, uint16_t msgSize, uint16_t uid, uint8_t rssi);
	bool hasNewMsg() {return NewMessage;}
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	RadioMessage RMsgs[8];
	uint8_t InternalState;
	cmdc0de::GUIListData MsgList;
	cmdc0de::GUIListItemData Items[8];
	uint8_t CurrentPos:7;
	uint8_t NewMessage:1;
public:
	static const uint16_t MAX_R_MSGS = (sizeof(RMsgs) / sizeof(RMsgs[0]));
};
#endif
