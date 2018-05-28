#ifndef MENU_STATE_H
#define MENU_STATE_H

#include "../libstm32/app/state_base.h"
#include "../messaging/stm_to_esp_generated.h"
#include "../libstm32/display/gui.h"


class MenuState: public cmdc0de::StateBase {
public:
	MenuState();
	virtual ~MenuState();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
private:
	cmdc0de::GUIListData MenuList;
	cmdc0de::GUIListItemData Items[1];
};


#endif
