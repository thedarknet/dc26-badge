#ifndef MENU_STATE_H
#define MENU_STATE_H

#include "darknet7_base_state.h"


class MenuState: public Darknet7BaseState {
public:
	MenuState();
	virtual ~MenuState();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	cmdc0de::GUIListData MenuList;
	cmdc0de::GUIListItemData Items[10];
};


#endif
