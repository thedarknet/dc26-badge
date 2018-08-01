#ifndef PAIRWITH_MENU_H_
#define PAIRWITH_MENU_H_

#include "darknet7_base_state.h"
#include "../mcu_to_mcu.h"
#include "../KeyStore.h"

class PairWithState : public Darknet7BaseState {
public:
public:
	PairWithState();
	virtual ~PairWithState();
	void receiveSignal(MCUToMCU*,const MSGEvent<darknet7::BLESecurityConfirm>* mevt);
protected:
	enum INTERNAL_STATE { NONE, PAIRING };
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	uint32_t loops = 0;
	INTERNAL_STATE InternalState;
};

#endif
