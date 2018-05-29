#ifndef ADDRESS_STATE_H
#define ADDRESS_STATE_H

#include "darknet7_base_state.h"
#include "../KeyStore.h"

class AddressState: public Darknet7BaseState {
public:
	AddressState();
	virtual ~AddressState();
	void resetSelection();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
	void setNext4Items(uint16_t startAt);
private:
	cmdc0de::GUIListData AddressList;
	cmdc0de::GUIListItemData Items[4];
	ContactStore::Contact CurrentContactList[4];
	cmdc0de::GUIListData ContactDetails;
	cmdc0de::GUIListItemData DetailItems[5];
	char RadioIDBuf[12];
	char PublicKey[64];
	char SignatureKey[128];
	uint8_t Index;
	cmdc0de::GUIListData *DisplayList;
};


#endif
