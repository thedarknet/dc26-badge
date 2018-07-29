/*
 * health.cpp
 *
 *  Created on: May 30, 2018
 *      Author: cmdc0de
 */



#include "health.h"
#include "../darknet7.h"
#include "../virtual_key_board.h"
#include "../libstm32/crypto/sha256.h"
#include "menu_state.h"
#include "gui_list_processor.h"

using cmdc0de::RGBColor;
using cmdc0de::ErrorType;
using cmdc0de::StateBase;

static const char *VirusNames [] =  {
		"Avian Flu"
		,"Measles"
		, "Tetanus"
		, "Polio"
		, "Plague"
		, "Toxoplasmosis"
		, "Chlamydia"
		, "Herpes"
};

static const char *CureCodes [] =  {
		  "AGTAGAAACAAGG"
		, "GTCAGTTCCACAT"
		, "GAGGTGCAGCTGG"
		, "ATTCTAACCATGG"
		, "AAGAGTATAATCG"
		, "CCTAAACCCTGAA"
		, "GTATTAGTATTTG"
		, "GATCGTTATTCCC"
};

class CureEntry: public Darknet7BaseState {
public:
	enum INTERNAL_STATE {NONE, VIRUS_ENTRY, DISPLAY_QUEST_KEY};
private:
	VirtualKeyBoard VKB;
	char CureSequence[14];
	VirtualKeyBoard::InputHandleContext IHC;
	INTERNAL_STATE InternalState;
	uint8_t CurrentVirus;
	uint8_t FinalHexHash[SHA256_HASH_SIZE];
public:
	CureEntry() : Darknet7BaseState(), VKB(), CureSequence(), IHC(&CureSequence[0],sizeof(CureSequence)), InternalState(NONE), CurrentVirus(0), FinalHexHash() {

	}
	virtual ~CureEntry() {}
	void setVirus(uint8_t v) {CurrentVirus = v;}
protected:
	virtual cmdc0de::ErrorType onInit() {
		memset(&FinalHexHash[0],0,sizeof(FinalHexHash));
		memset(&CureSequence[0],0,sizeof(CureSequence));
		IHC.set(&CureSequence[0],sizeof(CureSequence));
		VKB.init(VirtualKeyBoard::STDCAPS,&IHC,5,DarkNet7::DISPLAY_WIDTH-5,100,RGBColor::WHITE, RGBColor::BLACK, RGBColor::BLUE,'_');
		InternalState = VIRUS_ENTRY;
		return ErrorType();
	}
	bool validateCure() {
		return !strcmp(&CureSequence[0],CureCodes[CurrentVirus]);
	}

	virtual cmdc0de::StateBase::ReturnStateContext onRun() {
		StateBase *nextState = this;
		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		if(InternalState==VIRUS_ENTRY) {
			VKB.process();

			DarkNet7::get().getDisplay().drawString(0,10,(const char *)"Enter Sequence press:");
			DarkNet7::get().getDisplay().drawString(0,20, &CureSequence[0]);
			DarkNet7::get().getDisplay().drawString(0,60, (const char *)"Press MID to commit cure");
			if(DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
				if(validateCure()) {
					InternalState = DISPLAY_QUEST_KEY;
					uint8_t mhash[SHA256_HASH_SIZE] = { 0 };
					ShaOBJ HCtx;
					sha256_init(&HCtx);
					sha256_add(&HCtx, DarkNet7::get().getContacts().getMyInfo().getPublicKey(), ContactStore::PUBLIC_KEY_LENGTH);
					sha256_add(&HCtx, (const unsigned char *) &CureCodes[CurrentVirus], strlen((const char *) &CureCodes[CurrentVirus]));
					sha256_digest(&HCtx, &mhash[0]);
					sprintf((char *) &FinalHexHash[0], "%02x%02x%02x%02x%02x%02x%02x%02x", mhash[0], mhash[1], mhash[2],mhash[3], mhash[4], mhash[5], mhash[6], mhash[7]);
					//+1 bc cure all is 0 bit
					DarkNet7::get().getContacts().getSettings().cure(1<<(CurrentVirus+1));
				} else {
					nextState = DarkNet7::get().getDisplayMessageState(this,(const char *)"Invalid Cure Code", 1000);
				}
			}
		} else if(InternalState==DISPLAY_QUEST_KEY) {
			char buf[24];
			sprintf(&buf[0],"of %s", VirusNames[CurrentVirus]);
			DarkNet7::get().getDisplay().drawString(0, 10, "Your badge has been cured");
			DarkNet7::get().getDisplay().drawString(0, 20, &buf[0]);
			DarkNet7::get().getDisplay().drawString(0, 40, "Send To Daemon: ");
			DarkNet7::get().getDisplay().drawString(0, 50, (const char *) &FinalHexHash[0]);
			if(DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
				nextState = DarkNet7::get().getHealthState();
			}
		}
		return ReturnStateContext(nextState);
	}

	virtual cmdc0de::ErrorType onShutdown() {
		return ErrorType();
	}
};
static CureEntry EnterCure;


Health::Health() : Darknet7BaseState(), HealthList("Infections:", Items, 0, 0, DarkNet7::DISPLAY_WIDTH, DarkNet7::DISPLAY_HEIGHT, 0, (sizeof(Items) / sizeof(Items[0]))) {

}

Health::~Health()
{

}

ErrorType Health::onInit() {
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	sprintf(&ListBuffer[0][0], "Avian Flu    : %s", DarkNet7::get().getContacts().getSettings().isInfectedWith(ContactStore::SettingsInfo::AVIAN_FLU)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[1][0], "Measles      : %s", DarkNet7::get().getContacts().getSettings().isInfectedWith(ContactStore::SettingsInfo::MEASLES)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[2][0], "Tetanus      : %s", DarkNet7::get().getContacts().getSettings().isInfectedWith(ContactStore::SettingsInfo::TETANUS)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[3][0], "Polio        : %s", DarkNet7::get().getContacts().getSettings().isInfectedWith(ContactStore::SettingsInfo::POLIO)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[4][0], "Plague       : %s", DarkNet7::get().getContacts().getSettings().isInfectedWith(ContactStore::SettingsInfo::PLAGUE)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[5][0], "Toxoplasmosis: %s", DarkNet7::get().getContacts().getSettings().isInfectedWith(ContactStore::SettingsInfo::TOXOPLASMOSIS)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[6][0], "Chlamydia    : %s", DarkNet7::get().getContacts().getSettings().isInfectedWith(ContactStore::SettingsInfo::CHLAMYDIA)?DarkNet7::sYES:DarkNet7::sNO);
	sprintf(&ListBuffer[7][0], "Herpes       : %s", DarkNet7::get().getContacts().getSettings().isInfectedWith(ContactStore::SettingsInfo::HERPES)?DarkNet7::sYES:DarkNet7::sNO);

	for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
		Items[i].text = &ListBuffer[i][0];
		Items[i].id = 1<<(i+1);
		Items[i].setShouldScroll();
	}
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
	DarkNet7::get().getGUI().drawList(&HealthList);
	return ErrorType();

}

StateBase::ReturnStateContext Health::onRun() {
	StateBase *nextState = this;
	if (!GUIListProcessor::process(&HealthList,(sizeof(Items) / sizeof(Items[0])))) {
		if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
			nextState = DarkNet7::get().getDisplayMenuState();
		} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1) &&
				DarkNet7::get().getContacts().getSettings().isInfectedWith(HealthList.items[HealthList.selectedItem].id)) {
			EnterCure.setVirus(HealthList.selectedItem);
			nextState = &EnterCure;
		}
	}
	DarkNet7::get().getGUI().drawList(&HealthList);
	return ReturnStateContext(nextState);
}

ErrorType Health::onShutdown()
{
	return ErrorType();
}






