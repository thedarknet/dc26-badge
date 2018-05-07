#ifndef MENU_STATE_H
#define MENU_STATE_H

#include "libstm32/app/state_base.h"


class MenuState: public cmdc0de::StateBase {
public:
	MenuState();
	virtual ~MenuState();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	//GUI_ListData MenuList;
	//GUI_ListItemData Items[11];
};


#endif
