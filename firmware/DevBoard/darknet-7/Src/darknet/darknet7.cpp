/*
 * DC26.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: dcomes
 */

#include "darknet7.h"
#include "libstm32/display/display_device.h"
#include "libstm32/display/gui.h"
#include "libstm32/display/fonts.h"
#include <main.h>
#include "libstm32/config.h"
#include "libstm32/logger.h"
#include <usart.h>
#include <spi.h>
#include <i2c.h>
#include "libstm32/app/display_message_state.h"
#include <ff.h>
#include "KeyStore.h"
#include "menus/test_state.h"
#include "menus/SendMsgState.h"
#include "menus/menu_state.h"
#include "menus/setting_state.h"
#include "menus/pairing_state.h"
#include "menus/AddressState.h"
#include "menus/menu3d.h"
#include "menus/GameOfLife.h"
#include "menus/badge_info_state.h"
#include "menus/mcu_info.h"
#include "menus/tamagotchi.h"
#include "menus/communications_settings.h"
#include "messaging/stm_to_esp_generated.h"
#include "mcu_to_mcu.h"
#include "menus/health.h"
#include "menus/scan.h"

using cmdc0de::ErrorType;
using cmdc0de::DisplayST7735;
using cmdc0de::DrawBufferNoBuffer;
using cmdc0de::GUIListData;
using cmdc0de::GUIListItemData;
using cmdc0de::GUI;

//TODO Can't swap between landscape and portait
#define START_LANDSCAPE
#ifdef START_LANDSCAPE
static const uint32_t DISPLAY_WIDTH = 160;
static const uint32_t DISPLAY_HEIGHT = 128;
#define START_ROT DisplayST7735::LANDSCAPE_TOP_LEFT
#else
static const uint32_t DISPLAY_WIDTH = 128;
static const uint32_t DISPLAY_HEIGHT = 160;
#define START_ROT DisplayST7735::PORTAIT_TOP_LEFT
#endif

static const uint32_t DISPLAY_OPT_WRITE_ROWS = DISPLAY_HEIGHT;
static uint16_t DrawBuffer[DISPLAY_WIDTH * DISPLAY_OPT_WRITE_ROWS]; //120 wide, 10 pixels high, 2 bytes per pixel (uint16_t)
//cmdc0de::DrawBufferNoBuffer NoBuffer(&Display, &DrawBuffer[0],DISPLAY_OPT_WRITE_ROWS);

static const uint8_t MyAddressInfoSector = 3;
static const uint32_t MyAddressInfoOffSet = 0;
static const uint8_t SettingSector = 1;
static const uint32_t SettingOffset = 0;
static const uint8_t StartContactSector = 2;
static const uint8_t EndContactSector = 3;

DarkNet7 *DarkNet7::mSelf = 0;

DarkNet7::ButtonInfo::ButtonInfo() :
		ButtonState(0),LastButtonState(0) {
}

void DarkNet7::ButtonInfo::reset() {
	ButtonState = LastButtonState = 0;
}

bool DarkNet7::ButtonInfo::areTheseButtonsDown(const int32_t &b) {
	return (ButtonState & b) == b;
}

bool DarkNet7::ButtonInfo::isAnyOfTheseButtonDown(const int32_t &b) {
	return (ButtonState&b)!=0;
}

bool DarkNet7::ButtonInfo::isAnyButtonDown() {
	return ButtonState!=0;
}

bool DarkNet7::ButtonInfo::wereTheseButtonsReleased(const int32_t &b) {
	//last state must match these buttons and current state must have none of these buttons
	return (LastButtonState & b) == b && (ButtonState&b)==0;
}

bool DarkNet7::ButtonInfo::wereAnyOfTheseButtonsReleased(const int32_t &b) {
	//last state must have at least 1 of the buttons and at least 1 of the buttons must not be down now
	return (LastButtonState & b) != 0 && !isAnyOfTheseButtonDown(b);
}

bool DarkNet7::ButtonInfo::wasAnyButtonReleased() {
	return ButtonState!=LastButtonState && LastButtonState!=0;
}


void DarkNet7::ButtonInfo::processButtons() {
	LastButtonState = ButtonState;
	ButtonState = 0;
	if (HAL_GPIO_ReadPin(MID_BUTTON1_GPIO_Port, MID_BUTTON1_Pin)
			== GPIO_PIN_RESET) {
		ButtonState|=BUTTON_MID;
	} else if (HAL_GPIO_ReadPin(BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin)
			== GPIO_PIN_RESET) {
		ButtonState|=BUTTON_RIGHT;
	} else if (HAL_GPIO_ReadPin(BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin)
			== GPIO_PIN_RESET) {
		ButtonState|=BUTTON_LEFT;
	} else if (HAL_GPIO_ReadPin(BUTTON_UP_GPIO_Port, BUTTON_UP_Pin)
			== GPIO_PIN_RESET) {
		ButtonState|=BUTTON_UP;
	} else if (HAL_GPIO_ReadPin(BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin)
			== GPIO_PIN_RESET) {
		ButtonState|=BUTTON_DOWN;
	} else if (HAL_GPIO_ReadPin(BUTTON_FIRE1_GPIO_Port, BUTTON_FIRE1_Pin)
			== GPIO_PIN_RESET) {
		ButtonState|=BUTTON_FIRE1;
	}
}

DarkNet7 &DarkNet7::get() {
	if (0 == mSelf) {
		mSelf = new DarkNet7();
	}
	return *mSelf;
}

cmdc0de::DisplayST7735 &DarkNet7::getDisplay() {
	return Display;
}

const cmdc0de::DisplayST7735 &DarkNet7::getDisplay() const {
	return Display;
}

ContactStore &DarkNet7::getContacts() {
	return MyContacts;
}

const ContactStore &DarkNet7::getContacts() const {
	return MyContacts;
}

cmdc0de::GUI &DarkNet7::getGUI() {
	return MyGUI;
}

const cmdc0de::GUI &DarkNet7::getGUI() const {
	return MyGUI;
}

DarkNet7::ButtonInfo &DarkNet7::getButtonInfo() {
	return MyButtons;
}

const DarkNet7::ButtonInfo& DarkNet7::getButtonInfo() const {
	return MyButtons;
}

DarkNet7::DarkNet7() :
		Apa106s(GPIO_APA106_DATA_Pin, GPIO_APA106_DATA_GPIO_Port, TIM1, DMA2_Stream0, DMA2_Stream2_IRQn)
				//		my Info, start setting address, start Contact address, end contact address
				, MyContacts(MyAddressInfoSector, MyAddressInfoOffSet, SettingSector, SettingOffset, StartContactSector,
				EndContactSector)
				, Display(DISPLAY_WIDTH, DISPLAY_HEIGHT, START_ROT)
		, DisplayBuffer(static_cast<uint8_t>(DISPLAY_WIDTH),static_cast<uint8_t>(DISPLAY_HEIGHT),&DrawBuffer[0],&Display)
		, DMS(), MyGUI(&Display), MyButtons(), SequenceNum(0) {

		}

DarkNet7::~DarkNet7() {
	// TODO Auto-generated destructor stub
}

ErrorType DarkNet7::onInit() {
	ErrorType et;

	GUIListItemData items[4];
	GUIListData DrawList((const char *) "Self Check", items, uint8_t(0),
			uint8_t(0), uint8_t(DISPLAY_WIDTH), uint8_t(DISPLAY_HEIGHT / 2), uint8_t(0), uint8_t(0));
	//DO SELF CHECK
	if ((et = Display.init(DisplayST7735::FORMAT_16_BIT, &Font_6x10, &DisplayBuffer)).ok()) {
		HAL_Delay(1000);
		items[0].set(0, "OLED_INIT");
		DrawList.ItemsCount++;
		Display.setTextColor(cmdc0de::RGBColor::WHITE);
	}
#if 1
	HAL_GPIO_WritePin(SIMPLE_LED1_GPIO_Port, SIMPLE_LED1_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(SIMPLE_LED2_GPIO_Port, SIMPLE_LED2_Pin, GPIO_PIN_SET);
#endif
	GUI gui(&Display);
	gui.drawList(&DrawList);
	Display.swap();
	Display.fillScreen(cmdc0de::RGBColor::BLACK);
	Display.swap();
	Display.fillRec(30, 10, 80, 40, cmdc0de::RGBColor(0, 255, 0));
	Display.swap();
#if DEBUG_WHY_CANT_CHANGE_ROTATION
	//Display.setRotation(cmdc0de::DisplayDevice::LANDSCAPE_TOP_LEFT,true);
	Display.fillScreen(cmdc0de::RGBColor::BLACK);
	Display.swap();
	gui.drawList(&DrawList);
	Display.swap();
	Display.fillRec(30,10,80,40,cmdc0de::RGBColor(0,255,0));
	Display.swap();
#endif

	uint16_t r = 0, g = 0, b = 0;
	uint16_t y = 0;
	while (y < Display.getHeight()) {
		for (uint32_t i = 0; i < Display.getWidth(); i++) {
			Display.drawPixel(i, y, cmdc0de::RGBColor(r, g, b));
			if (r == 0xFF && g == 0xFF && b == 0xFF) {
				r = g = b = 0;
			} else if (r == 0xFF && g == 0xFF) {
				++b;
			} else if (r == 0xFF) {
				++g;
			} else {
				++r;
			}
		}
		++y;
	}
	Display.swap();
	HAL_Delay(500);
	setCurrentState(getDisplayMenuState());

#define TEST_SD_CARD
#ifdef TEST_SD_CARD
	//disk_initialize(0);
	FATFS myFS;
	f_mount(&myFS,"0:",1);
	DIR d;
	if(FR_OK==f_opendir(&d,"/")) {
		FILINFO fno;
		if(FR_OK==::f_readdir(&d,&fno)) {
			DBGMSG("%s",&fno.fname[0]);
		}
	}
#endif

#if 1
	 flatbuffers::FlatBufferBuilder fbb;
	 auto setup = darknet7::CreateSetupAPDirect(fbb,"test","test",darknet7::WifiMode_WPA2);
	 flatbuffers::Offset<darknet7::STMToESPRequest> of = darknet7::CreateSTMToESPRequest(fbb,1U,darknet7::STMToESPAny_SetupAP,setup.Union());
	 darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,of);

	 flatbuffers::uoffset_t size = fbb.GetSize();
	 uint8_t *p = fbb.GetBufferPointer();

	 flatbuffers::Verifier v(p,size);
	 if(darknet7::VerifySizePrefixedSTMToESPRequestBuffer(v)) {
		 v.GetComputedSize();
		 auto t = darknet7::GetSizePrefixedSTMToESPRequest(p);
		 auto msg = t->Msg_as_SetupAP();
		 if(msg->mode()==darknet7::WifiMode_WPA2) {
			 DBGMSG("yeah");
		 }
	 }
#endif

	 MCUToMCU::get().init(&huart1);
	return et;
}

static const char *RFAILED = "Receive Failed";
static const char *TFAILED = "Transmit Failed";
//static const uint16_t ESP_ADDRESS = 1;

ErrorType DarkNet7::onRun() {
	MyButtons.processButtons();

	//emit new messages
	MCUToMCU::get().process();

	cmdc0de::StateBase::ReturnStateContext rsc = getCurrentState()->run();
	Display.swap();

	if (rsc.Err.ok()) {
		if (getCurrentState() != rsc.NextMenuToRun) {
			//on state switches reset keyboard and give a 1 second pause on reading from keyboard.
			MyButtons.reset();
			setCurrentState(rsc.NextMenuToRun);
		}
		//if (CurrentState != StateFactory::getGameOfLifeState()
		//		&& (tick > KB.getLastPinSelectedTick())
		//		&& (tick - KB.getLastPinSelectedTick()
		//				> (1000 * 60
		//						* rc.getContactStore().getSettings().getScreenSaverTime()))) {
		//	CurrentState->shutdown();
		//	CurrentState = StateFactory::getGameOfLifeState();
		//} else {
		//	CurrentState = rsc.NextMenuToRun;
		//}
	} else {
		//setCurrentState(StateCollection::getDisplayMessageState(
		//		StateCollection::getDisplayMenuState(), (const char *)"Run State Error....", uint16_t (2000)));
	}

	return ErrorType();
}

cmdc0de::DisplayMessageState *DarkNet7::getDisplayMessageState(cmdc0de::StateBase *bm, const char *message,
		uint16_t timeToDisplay) {
	DMS.setMessage(message);
	DMS.setNextState(bm);
	DMS.setTimeInState(timeToDisplay);
	DMS.setDisplay(&Display);
	return &DMS;
}

uint32_t DarkNet7::nextSeq() {
	return ++SequenceNum;
}

static MenuState MyMenu;
static TestState MyTestState;
static SendMsgState MySendMsgState;
static SettingState MySettingState;
static PairingState MyPairingState;
static AddressState MyAddressState;
static Menu3D MyMenu3D;
static GameOfLife MyGameOfLife;
static CommunicationSettingState MyCommunicationSettings;
static BadgeInfoState MyBadgeInfoState;
static MCUInfoState MyMCUInfoState;
static Tamagotchi MyTamagotchi;
static Health MyHealth;
static Scan MyScan;

MenuState *DarkNet7::getDisplayMenuState() {
	return &MyMenu;
}

TestState *DarkNet7::getTestState() {
	return &MyTestState;
}

SendMsgState *DarkNet7::getSendMsgState() {
	return &MySendMsgState;
}

SettingState *DarkNet7::getSettingState() {
	return &MySettingState;
}

PairingState *DarkNet7::getPairingState() {
	return &MyPairingState;
}

AddressState *DarkNet7::getAddressBookState() {
	return &MyAddressState;
}

Menu3D *DarkNet7::get3DState() {
	return &MyMenu3D;
}

GameOfLife *DarkNet7::getGameOfLifeState() {
	return &MyGameOfLife;
}

CommunicationSettingState *DarkNet7::getCommunicationSettingState() {
	return &MyCommunicationSettings;
}

BadgeInfoState * DarkNet7::getBadgeInfoState() {
	return &MyBadgeInfoState;
}

MCUInfoState *DarkNet7::getMCUInfoState() {
	return &MyMCUInfoState;
}

Tamagotchi *DarkNet7::getTamagotchiState() {
	return &MyTamagotchi;
}

Health *DarkNet7::getHealthState() {
	return &MyHealth;
}

Scan *DarkNet7::getScanState() {
	return &MyScan;
}
