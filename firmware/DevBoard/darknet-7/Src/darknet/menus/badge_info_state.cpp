/*
 * badge_info_state.cpp
 *
 *  Created on: May 29, 2018
 *      Author: dcomes
 */

#include "badge_info_state.h"
#include "../darknet7.h"
#include "../libstm32/crypto/sha256.h"
#include "menu_state.h"

using cmdc0de::RGBColor;
using cmdc0de::ErrorType;
using cmdc0de::StateBase;

BadgeInfoState::BadgeInfoState() :
		Darknet7BaseState(), BadgeInfoList("Badge Info:", Items, 0, 0, 128, 160,
				0, (sizeof(Items) / sizeof(Items[0]))), RegCode() {

	memset(&RegCode, 0, sizeof(RegCode));
}

BadgeInfoState::~BadgeInfoState() {

}

const char *BadgeInfoState::getRegCode(ContactStore &cs) {
	if (RegCode[0] == 0) {
		ShaOBJ hashObj;
		sha256_init(&hashObj);
		sha256_add(&hashObj, cs.getMyInfo().getPrivateKey(),
				ContactStore::PRIVATE_KEY_LENGTH);
		uint16_t id = cs.getMyInfo().getUniqueID();
		sha256_add(&hashObj, (uint8_t *) &id, sizeof(id));
		uint8_t rH[SHA256_HASH_SIZE];
		sha256_digest(&hashObj, &rH[0]);
		sprintf(&RegCode[0], "%02x%02x%02x%02x%02x%02x%02x%02x", rH[0], rH[1],
				rH[2], rH[3], rH[4], rH[5], rH[6], rH[7]);
	}
	return &RegCode[0];
}

static const char *VERSION = "dn7.dc26.1";

ErrorType BadgeInfoState::onInit() {
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	sprintf(&ListBuffer[0][0], "Name: %s",
			DarkNet7::get().getContacts().getSettings().getAgentName());
	sprintf(&ListBuffer[1][0], "Num contacts: %u",
			DarkNet7::get().getContacts().getSettings().getNumContacts());
	sprintf(&ListBuffer[2][0], "REG: %s",
			getRegCode(DarkNet7::get().getContacts()));
	sprintf(&ListBuffer[3][0], "UID: %u",
			DarkNet7::get().getContacts().getMyInfo().getUniqueID());
	uint8_t *pCP =	DarkNet7::get().getContacts().getMyInfo().getCompressedPublicKey();
	sprintf(&ListBuffer[4][0], "PK: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			pCP[0], pCP[1], pCP[2], pCP[3], pCP[4], pCP[5], pCP[6], pCP[7],
			pCP[8], pCP[9], pCP[10], pCP[11], pCP[12], pCP[13], pCP[14],
			pCP[15], pCP[16], pCP[17], pCP[18], pCP[19], pCP[20], pCP[21],
			pCP[22], pCP[23], pCP[24]);
	sprintf(&ListBuffer[5][0], "DEVID: %lu", HAL_GetDEVID());
	sprintf(&ListBuffer[6][0], "REVID: %lu", HAL_GetREVID());
	sprintf(&ListBuffer[7][0], "HAL Version: %lu", HAL_GetHalVersion());
	sprintf(&ListBuffer[8][0], "SVer: %s", VERSION);

	for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
		Items[i].text = &ListBuffer[i][0];
		Items[i].id = i;
		Items[i].setShouldScroll();
	}
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
	DarkNet7::get().getGUI().drawList(&BadgeInfoList);
	return ErrorType();
}

StateBase::ReturnStateContext BadgeInfoState::onRun() {

	StateBase *nextState = this;
	if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
		nextState = DarkNet7::get().getDisplayMenuState();
	}
	return ReturnStateContext(nextState);
}

ErrorType BadgeInfoState::onShutdown() {
	return ErrorType();
}

