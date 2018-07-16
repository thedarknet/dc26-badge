#ifndef EVENT_STATE_H
#define EVENT_STATE_H

#include "darknet7_base_state.h"

class TestState: public Darknet7BaseState {
public:
	static const uint16_t MAX_MSG_LEN = 64;
public:

public:
	TestState();
	virtual ~TestState();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	static const uint32_t EXIT_COUNT = 20;
	cmdc0de::GUIListData ButtonList;
	cmdc0de::GUIListItemData Items[7];
	char ListBuffer[7][24]; //height then width
	uint32_t TimesFireHasBeenHeld;
};
#endif
