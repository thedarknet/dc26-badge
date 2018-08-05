/*
 * sao_menu.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: cmdc0de
 */



#include "sao_menu.h"
#include "../darknet7.h"
#include "menu_state.h"
#include "../virtual_key_board.h"
#include "gui_list_processor.h"
#include <i2c.h>

using cmdc0de::RGBColor;
using cmdc0de::ErrorType;
using cmdc0de::StateBase;


class EggPlant: public Darknet7BaseState {
private:
	enum INTERNAL_STATE {NONE, INITIAL, DISPLAY_DATA, INTERACT_SET_LED, INTERACT_SET_NAME };
	VirtualKeyBoard VKB;
	char NewEggPlantName[13];
	VirtualKeyBoard::InputHandleContext IHC;
	char CurrentEggPlantName[13];
	char Type[9];
	char Version[9];
	char Serial[9];
	uint8_t LedMode;
	uint16_t Address;
	uint8_t InfectionStatus;
	INTERNAL_STATE InternalState;
	cmdc0de::GUIListData EggList;
	cmdc0de::GUIListItemData Items[7];
	char ListBuffer[7][24];
public:
	EggPlant() : Darknet7BaseState(), VKB(), NewEggPlantName(), IHC(&NewEggPlantName[0],sizeof(NewEggPlantName)),
		CurrentEggPlantName(),Type(),Version(),Serial(), LedMode(0), Address(0), InfectionStatus(0),InternalState(NONE),
			EggList((const char *) "MENU", Items, 0, 0,	DarkNet7::DISPLAY_WIDTH, 85, 0, sizeof(Items) / sizeof(Items[0])){

	}
	virtual ~EggPlant() {}
	void setAddress(uint16_t a) {Address = a;}
	uint16_t getAddress() {return (Address<<1|0);}
protected:
	void fixUpString(char *p, uint32_t size) {
		for(uint32_t i=0;i<size;i++) {
			if(p[i]>126 || p[i]<32) p[i]='\0';
		}
	}
	bool fetchAllFromEggPlant() {
		memset(&Type[0],0,sizeof(Type));
		memset(&Version[0],0,sizeof(Version));
		memset(&Serial[0],0,sizeof(Serial));
		memset(&CurrentEggPlantName[0],0,sizeof(CurrentEggPlantName));
		uint8_t command[3] = {0,0,0};
		command[0]=0;
		command[1]=2;
		command[2]=0;
		uint8_t crap[14] = {0};
		if(HAL_OK==HAL_I2C_Master_Transmit(&hi2c3,getAddress(),&command[0],sizeof(command),100)) {
			if(HAL_OK!=HAL_I2C_Master_Receive(&hi2c3,getAddress(),&crap[0],sizeof(crap),100)) {
				return false;
			}
			LedMode = crap[0];
			for(uint8_t i=0;i<4;i++) {
				command[0]=0;
				command[1]=0;
				command[2]=i;
				if(HAL_OK==HAL_I2C_Master_Transmit(&hi2c3,getAddress(),&command[0],sizeof(command),300)) {
					uint8_t *recData;
					uint16_t size;
					switch(i) {
					case 0:
						recData = (uint8_t *)&Type[0];
						size = sizeof(Type)-1;
						break;
					case 1:
						recData = (uint8_t *)&Version[0];
						size = sizeof(Version)-1;
						break;
					case 2:
						recData = (uint8_t *)&Serial[0];
						size = sizeof(Serial)-1;
						break;
					case 3:
						recData = (uint8_t *)&CurrentEggPlantName[0];
						size = sizeof(CurrentEggPlantName)-1;
					}
					if(HAL_OK!=HAL_I2C_Master_Receive(&hi2c3,getAddress(),(uint8_t*)&recData[0],size,300)) {
						return false;
					}
				} else {
					return false;
				}
			}

			//do we infect the egg plant?
			int value = 0;
			const int BOTH = ContactStore::SettingsInfo::CHLAMYDIA|ContactStore::SettingsInfo::HERPES;
			int r = rand()%100;
			//so there is a off by one bug in the egg plant firmware...
			if(r<50) { //infect with Chlamydia
				value|=(ContactStore::SettingsInfo::CHLAMYDIA>>1);
			}
			r = rand()%100;
			if(r<30) {
				value|=(ContactStore::SettingsInfo::HERPES>>1);
			}
			if(value>0) { //yes
				uint8_t setInfection[4] = {0x0,0x5,0x0,0x0};
				setInfection[3] = value&0xFF;
				if(HAL_OK==HAL_I2C_Master_Transmit(&hi2c3,getAddress(),&setInfection[0],sizeof(setInfection),100)) {
					HAL_I2C_Master_Receive(&hi2c3,getAddress(),&crap[0],sizeof(crap),500);
				}
			}

			command[0]=0;
			command[1]=4;
			command[2]=1;
			if(HAL_OK==HAL_I2C_Master_Transmit(&hi2c3,getAddress(),&command[0],sizeof(command),100)) {
				if(HAL_OK!=HAL_I2C_Master_Receive(&hi2c3,getAddress(),&InfectionStatus,sizeof(InfectionStatus),100)) {
					return false;
				}
				//HAL_I2C_Master_Receive(&hi2c3,getAddress(),&crap[0],sizeof(crap),500);
				value=InfectionStatus;
				value<<=1; //fix off by one bug in eggplant
				if(((value&ContactStore::SettingsInfo::CHLAMYDIA)==ContactStore::SettingsInfo::CHLAMYDIA) || ((value&ContactStore::SettingsInfo::HERPES)==ContactStore::SettingsInfo::HERPES)
						|| ((value&BOTH)==BOTH)) {
					DarkNet7::get().getContacts().getSettings().setHealth(value);
				}
			}
			//HAL_I2C_Master_Receive(&hi2c3,getAddress(),&crap[0],sizeof(crap),500);

			fixUpString(&Type[0],sizeof(Type));
			fixUpString(&Version[0],sizeof(Version));
			fixUpString(&Serial[0],sizeof(Serial));
			fixUpString(&CurrentEggPlantName[0],sizeof(CurrentEggPlantName));
			return true;
		} else {
			return false;
		}
	}
	virtual cmdc0de::ErrorType onInit() {
		InternalState = INITIAL;
		memset(&ListBuffer[0],0,sizeof(ListBuffer));
		for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
			Items[i].text = &ListBuffer[i][0];
		}
		IHC.set(&NewEggPlantName[0],sizeof(NewEggPlantName));

		return ErrorType();
	}

	virtual cmdc0de::StateBase::ReturnStateContext onRun() {
		StateBase *nextState = this;
		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		switch(InternalState) {
		case INITIAL:
			memset(&NewEggPlantName[0],0,sizeof(NewEggPlantName));
			IHC.set(&NewEggPlantName[0],sizeof(NewEggPlantName));

			if(!fetchAllFromEggPlant()) {
				nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),(const char *)"Error Communicating with \nEggPlant.\n What are you doing to it?",2000);
			} else {
				sprintf(&ListBuffer[0][0], "Address: %.2x", Address);
				sprintf(&ListBuffer[1][0], "Type   : %s", &Type[0]);
				sprintf(&ListBuffer[2][0], "Version: %s", &Version[0]);
				sprintf(&ListBuffer[3][0], "Serial : %s", &Serial[0]);
				sprintf(&ListBuffer[4][0], "Name   : %s", &CurrentEggPlantName[0]);
				sprintf(&ListBuffer[5][0], "LEDMode: %d", (int)LedMode);
				sprintf(&ListBuffer[6][0], "Infectious: %s", (InfectionStatus>128?(const char *)"HIGHLY":InfectionStatus?DarkNet7::sYES:DarkNet7::sNO));
				InternalState = DISPLAY_DATA;
			}
			break;
		case DISPLAY_DATA:
			if(!GUIListProcessor::process(&EggList,EggList.ItemsCount)) {
				if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
					nextState = DarkNet7::get().getDisplayMenuState();
				} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_FIRE1)) {
					switch(EggList.selectedItem) {
					case 4:
						InternalState = INTERACT_SET_NAME;
						VKB.init(VirtualKeyBoard::STDKBNames,&IHC,5,DarkNet7::DISPLAY_WIDTH-5,100,RGBColor::WHITE, RGBColor::BLACK,RGBColor::BLUE,'_');
						break;
					case 5:
						InternalState = INTERACT_SET_LED;
						break;
					default:
						break;
					}
				}
			}
			break;
		case INTERACT_SET_LED: {
				sprintf(&ListBuffer[5][0], "LED Mode : %d", (int)LedMode);
				if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_UP)) {
					LedMode = ++LedMode%7;
				} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_DOWN)) {
					if(LedMode==0) {LedMode=6;}
					else {--LedMode;}
				} else if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
					uint8_t command[3] = {0x0, 0x3, 0x0};
					command[2] = LedMode;
					uint8_t readBuf[33] = {0};
					if(HAL_OK==HAL_I2C_Master_Transmit(&hi2c3,getAddress(),&command[0],sizeof(command),100)) {
						HAL_StatusTypeDef answer = HAL_I2C_Master_Receive(&hi2c3,getAddress(),&readBuf[0],sizeof(readBuf),500);
						if(HAL_OK==answer || HAL_TIMEOUT==answer) {
							InternalState = INITIAL;
						} else {
							nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),(const char *)"Error Communicating with EggPlant...\n what are you doing to it?",2000);
						}
					} else {
						nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),(const char *)"Error Communicating with EggPlant...\n what are you doing to it?",2000);
					}
				}
			}
			break;
		case INTERACT_SET_NAME:
			sprintf(&ListBuffer[4][0], "Name     : %s", &NewEggPlantName[0]);
			VKB.process();
			if (DarkNet7::get().getButtonInfo().wereAnyOfTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
				uint8_t command[sizeof(NewEggPlantName)+3] = {0x0, 0x1, 0x3};
				memcpy(&command[3], &NewEggPlantName[0],sizeof(NewEggPlantName)-1);
				uint8_t readBuf[33] = {0};
				if(HAL_OK==HAL_I2C_Master_Transmit(&hi2c3,getAddress(),&command[0],sizeof(command),100)) {
					HAL_StatusTypeDef answer = HAL_I2C_Master_Receive(&hi2c3,getAddress(),&readBuf[0],sizeof(readBuf),100);
					if(HAL_OK==answer || HAL_TIMEOUT==answer) {
						InternalState = INITIAL;
					} else {
						nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),(const char *)"Error Communicating with EggPlant...\n what are you doing to it?",2000);
					}
				} else {
					nextState = DarkNet7::get().getDisplayMessageState(DarkNet7::get().getDisplayMenuState(),(const char *)"Error Communicating with EggPlant...\n what are you doing to it?",2000);
				}
			}
			break;
		default:
			break;
		}
		DarkNet7::get().getGUI().drawList(&EggList);

		return ReturnStateContext(nextState);
	}

	virtual cmdc0de::ErrorType onShutdown() {
		return ErrorType();
	}
};
static EggPlant EggPlantSAO;


SAO::SAO() : Darknet7BaseState(), InternalState(NONE), Address(NOADDRESS) {

}

SAO::~SAO()
{

}

ErrorType SAO::onInit() {
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
	InternalState = DISPLAY_SCANNING;
	Address = NOADDRESS;
	return ErrorType();
}

StateBase::ReturnStateContext SAO::onRun() {
	StateBase *nextState = this;
	if(InternalState==DISPLAY_SCANNING) {
		DarkNet7::get().getDisplay().drawString(0,20,(const char *)"Starting I2C scan");
		InternalState = SCANNING;
	} else if(InternalState==SCANNING) {
		HAL_StatusTypeDef result;
		for (uint16_t i=3; i<128; i++) {
		  /*
		   * the HAL wants a left aligned i2c address
		   * &hi2c1 is the handle
		   * (uint16_t)(i<<1) is the i2c address left aligned
		   * retries 2
		   * timeout 2
		   */
		  result = HAL_I2C_IsDeviceReady(&hi2c3, i<<1|0, 2, 200);
		  if (result == HAL_OK) {
			  Address = i;
		  }
		}
		InternalState = INTERACTING;
	} else {
		DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
		if(Address==NOADDRESS) {
			DarkNet7::get().getDisplay().drawString(0,20,(const char *)"No Shitty Add on\nBadge found!",RGBColor::RED, RGBColor::BLACK, 1, true);
		} else {
			char buf[24];
			DarkNet7::get().getDisplay().drawString(0,20,(const char *)"Shitty Add on Badge found!", RGBColor::BLUE, RGBColor::BLACK, 1, true);
			sprintf(&buf[0],"Address: 0x%.2x", (int)Address);
			DarkNet7::get().getDisplay().drawString(0,30,&buf[0]);
			if(Address==0x36) { //its an egg plant!
				EggPlantSAO.setAddress(Address);
				nextState = &EggPlantSAO;
			} else {
				DarkNet7::get().getDisplay().drawString(0,50,(const char *)"But not sure what\nelse I can do...", RGBColor::BLUE, RGBColor::BLACK, 1, true);
			}
		}
		if(DarkNet7::get().getButtonInfo().wereTheseButtonsReleased(DarkNet7::ButtonInfo::BUTTON_MID)) {
			nextState =DarkNet7::get().getDisplayMenuState();
		}
	}
	return ReturnStateContext(nextState);
}

ErrorType SAO::onShutdown()
{
	return ErrorType();
}








