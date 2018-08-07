#include "KeyStore.h"
#include <string.h>
#include "libstm32/crypto/micro-ecc/uECC.h"
#include "mcu_to_mcu.h"
#include "messaging/stm_to_esp_generated.h"
#include "messaging/esp_to_stm_generated.h"
#include "../darknet7.h"

const uint8_t ContactStore::DaemonPublic[ContactStore::PUBLIC_KEY_LENGTH] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

class FLASH_LOCKER {
public:
	FLASH_LOCKER() {
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP| FLASH_FLAG_WRPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_RDERR);
	}
	~FLASH_LOCKER() {
		HAL_FLASH_Lock();
	}
};

struct SectorInfo {
	uint32_t StartAddress;
	uint32_t Size;
};

static SectorInfo SectorToAddress[] =
		{
				{ FLASH_BASE + 0x0000000, 16384 } // (0x4000 16kB) not protected - program
				, { FLASH_BASE + 0x00004000, 16384 } //(0x4000 16kB) not protected - data for settings
				, { FLASH_BASE + 0x00008000, 16384 } //(0x4000 16kB) not protected - data for address
				, { FLASH_BASE + 0x0000c000, 16384 } //(0x4000 16kB) not protected - data
				, { FLASH_BASE + 0x00010000, 65536 } //(0x10000 64kB) not protected - data resources
				, { FLASH_BASE + 0x00020000, 131072 } //(0x20000 128kB) not protected - program
				, { FLASH_BASE + 0x00040000, 131072 } //(0x20000 128kB) not protected - program
				, { FLASH_BASE + 0x00060000, 131072 } //(0x20000 128kB) not protected - program
		};

#define SECTOR_TO_ADDRESS(sector) (SectorToAddress[sector].StartAddress)

ContactStore::SettingsInfo::SettingsInfo(uint8_t sector, uint32_t offSet, uint8_t endSector) :
		SettingSector(sector), OffSet(offSet), EndSettingSector(endSector), AgentName() {
	CurrentAddress = getStartAddress();
	memset(&AgentName[0], 0, sizeof(AgentName));
}

void ContactStore::SettingsInfo::receiveSignal(MCUToMCU*, const MSGEvent<darknet7::BLEInfectionData>* mevt) {
	uint16_t exposures = mevt->InnerMsg->exposures();
	uint16_t infections = 0x0;
	uint32_t unlucky = rand() % 100;
	//infections |= (unlucky < 12)                  ? (exposures & 0x2) : 0;
	infections |= (unlucky < 5)  ? (exposures & 0x4) : 0;
	infections |= (unlucky < 50)                  ? (exposures & 0x8) : 0;
	//infections |= (unlucky > 11 && unlucky < 25)  ? (exposures & 0x10) : 0;
	//infections |= (unlucky > 24 && unlucky < 75)  ? (exposures & 0x20) : 0;
	//infections |= (unlucky > 25 && unlucky < 45)  ? (exposures & 0x40) : 0;
	//infections |= (unlucky > 50)                  ? (exposures & 0x80) : 0;
	//infections |= (unlucky > 45 && unlucky < 75)  ? (exposures & 0x100) : 0;

	this->setHealth(infections);

	//Uncomment this to infect other people
	infections = DarkNet7::get().getContacts().getSettings().getHealth();
	infections &= 0x4; // Only expose others to measels
	flatbuffers::FlatBufferBuilder fbb;
	auto r = darknet7::CreateBLESetInfectionData(fbb, infections);
	auto e = darknet7::CreateSTMToESPRequest(fbb, 0, darknet7::STMToESPAny_BLESetInfectionData, r.Union());
	darknet7::FinishSizePrefixedSTMToESPRequestBuffer(fbb,e);
	MCUToMCU::get().send(fbb);

	return;
}

bool ContactStore::SettingsInfo::init() {
	const MSGEvent<darknet7::BLEInfectionData> *removebob=0;
	MCUToMCU::get().getBus().addListener(this,removebob,&MCUToMCU::get());

	for (uint32_t addr = getStartAddress(); addr < getEndAddress(); addr += SettingsInfo::SIZE) {
		uint32_t value = *((uint32_t*) addr);
		if (value == SETTING_MARKER) {
			CurrentAddress = addr;
			const char *AgentNameAddr = ((const char *) (CurrentAddress + sizeof(uint32_t) + sizeof(uint32_t)));
			strncpy(&AgentName[0], AgentNameAddr, sizeof(AgentName));
			return true;
		}
	}
	//couldn't find DS
	CurrentAddress = getEndAddress();
	DataStructure ds;
	ds.Health = 0; //0x1FE; //all the virus
	ds.ScreenSaverTime = 1;
	ds.ScreenSaverType = 0;
	ds.SleepTimer = 3;
	ds.NumContacts = 0;
	return writeSettings(ds);
}

uint32_t ContactStore::SettingsInfo::getStartAddress() {
	return SECTOR_TO_ADDRESS(SettingSector) + OffSet;
}

uint32_t ContactStore::SettingsInfo::getEndAddress() {
	return SECTOR_TO_ADDRESS(EndSettingSector);
}

bool ContactStore::SettingsInfo::setAgentname(const char name[AGENT_NAME_LENGTH]) {
	strncpy(&AgentName[0], &name[0], sizeof(AgentName));
	DataStructure ds = getSettings();
	return writeSettings(ds);
}

bool ContactStore::SettingsInfo::setHealth(uint16_t v) {
	DataStructure ds = getSettings();
	if(v==CLEAR_ALL) {
		ds.Health = 0;
	} else {
		ds.Health|=v;
	}
	return writeSettings(ds);
}

uint16_t ContactStore::SettingsInfo::getHealth() {
	DataStructure ds = getSettings();
	return (ds.Health & 0xFFFF);
}

bool ContactStore::SettingsInfo::isInfectedWith(uint16_t v) {
	DataStructure ds = getSettings();
	return ((v&ds.Health)==v);
}

bool ContactStore::SettingsInfo::cure(uint16_t v) {
	DataStructure ds = getSettings();
	ds.Health = (ds.Health&~v);
	return writeSettings(ds);
}

bool ContactStore::SettingsInfo::isNameSet() {
	return (AgentName[0] != '\0' && AgentName[0] != '_');
}

const char *ContactStore::SettingsInfo::getAgentName() {
	return &AgentName[0];
}

uint32_t ContactStore::SettingsInfo::getVersion() {
	return *((uint32_t*) CurrentAddress);
}

uint8_t ContactStore::SettingsInfo::getNumContacts() {
	DataStructure ds = getSettings();
	return ds.NumContacts;
}

ContactStore::SettingsInfo::DataStructure ContactStore::SettingsInfo::getSettings() {
	return *((ContactStore::SettingsInfo::DataStructure*) (CurrentAddress + sizeof(uint32_t)));
}

void ContactStore::SettingsInfo::resetToFactory() {
	{
		FLASH_LOCKER f;
		uint32_t sectorError = 0;
		FLASH_EraseInitTypeDef EraseInitStruct;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
		EraseInitStruct.Sector = this->SettingSector;
		EraseInitStruct.Banks = 0;
		EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		EraseInitStruct.NbSectors = 1;
		HAL_FLASHEx_Erase(&EraseInitStruct, &sectorError);
	}
	init();
}

bool ContactStore::SettingsInfo::writeSettings(const DataStructure &ds) {
	FLASH_LOCKER f;
	uint32_t startNewAddress = CurrentAddress + SettingsInfo::SIZE;
	uint32_t endNewAddress = startNewAddress + SettingsInfo::SIZE;
	if (endNewAddress >= getEndAddress()) {
		FLASH_EraseInitTypeDef EraseInitStruct;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
		EraseInitStruct.Sector = this->SettingSector;
		EraseInitStruct.Banks = 0;
		EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		EraseInitStruct.NbSectors = 1;
		uint32_t SectorError = 0;

		if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
			return false;
		}
		CurrentAddress = getStartAddress();
		startNewAddress = CurrentAddress;
		endNewAddress = CurrentAddress + SettingsInfo::SIZE;
	} else {
		//zero out the one we were on
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress, 0); //2
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress + 4, 0); //4
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress + 8, 0);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress + 12, 0);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress + 16, 0);
		CurrentAddress = startNewAddress;
	}
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress, SETTING_MARKER);
	uint32_t data = *((uint32_t*) &ds);
	if (HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (CurrentAddress + sizeof(uint32_t)), data)) {
		uint32_t agentStart = CurrentAddress + sizeof(uint32_t) + sizeof(uint32_t);
		if (HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, agentStart, (*((uint32_t *) &AgentName[0])))) {
			if (HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, agentStart + 4, (*((uint32_t *) &AgentName[4])))) {
				if (HAL_OK
						== HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, agentStart + 8, (*((uint32_t *) &AgentName[8])))) {
					return true;
				}
			}
		}
	}
	return false;
}

uint8_t ContactStore::SettingsInfo::setNumContacts(uint8_t num) {
	if (num > MAX_CONTACTS)
		return MAX_CONTACTS;
	DataStructure ds = getSettings();
	ds.NumContacts = num;
	writeSettings(ds);
	return num;
}

bool ContactStore::SettingsInfo::setScreenSaverType(uint8_t value) {
	DataStructure ds = getSettings();
	ds.ScreenSaverType = value & 0xF;
	return writeSettings(ds);
}

uint8_t ContactStore::SettingsInfo::getScreenSaverType() {
	DataStructure ds = getSettings();
	return ds.ScreenSaverType;
}

bool ContactStore::SettingsInfo::setScreenSaverTime(uint8_t value) {
	DataStructure ds = getSettings();
	ds.ScreenSaverTime = value & 0xF;
	return writeSettings(ds);
}

uint8_t ContactStore::SettingsInfo::getScreenSaverTime() {
	return getSettings().ScreenSaverTime;
}

bool ContactStore::SettingsInfo::setSleepTime(uint8_t n) {
	DataStructure ds = getSettings();
	ds.SleepTimer = n & 0xF;
	return writeSettings(ds);
}

uint8_t ContactStore::SettingsInfo::getSleepTime() {
	return getSettings().SleepTimer;
}

// MyInfo
//===========================================================
ContactStore::MyInfo::MyInfo(uint32_t startAddress) :
		StartAddress(startAddress) {

}

bool ContactStore::MyInfo::init() {
	return (*(uint16_t*) StartAddress) == 0xdcdc;
}

uint8_t *ContactStore::MyInfo::getPrivateKey() {
	return ((uint8_t*) (StartAddress + sizeof(uint16_t) + sizeof(uint16_t)));
}

uint16_t ContactStore::MyInfo::getUniqueID() {
	return *((uint16_t*) (StartAddress + sizeof(uint16_t)));
}

//TODO make this a member var of MyInfo
uint8_t publicKey[ContactStore::PUBLIC_KEY_LENGTH] = { 0 };
uint8_t *ContactStore::MyInfo::getPublicKey() {
	//given our private key, generate our public key and ensure its good
	if (uECC_compute_public_key(getPrivateKey(), &publicKey[0], THE_CURVE)) {
		if (uECC_valid_public_key(&publicKey[0], THE_CURVE) == 1) {
			return &publicKey[0];
		}
	}
	return 0;
}

uint8_t compressedPublicKey[ContactStore::PUBLIC_KEY_COMPRESSED_STORAGE_LENGTH] = { 0 };
uint8_t *ContactStore::MyInfo::getCompressedPublicKey() {
	uECC_compress(getPublicKey(), &compressedPublicKey[0], THE_CURVE);
	return &compressedPublicKey[0];
}

bool ContactStore::MyInfo::isUberBadge() {
	return ((getFlags() & 0x1) != 0);
}

uint16_t ContactStore::MyInfo::getFlags() {
	return *((uint16_t*) (StartAddress + sizeof(uint16_t) + sizeof(uint16_t) + PRIVATE_KEY_LENGTH));
}

/////////////////////////////////////////////////////////////////////////////////
ContactStore::Contact::Contact(uint32_t startAddr) :
		StartAddress(startAddr) {
}

uint16_t ContactStore::Contact::getUniqueID() {
	return *((uint16_t*) StartAddress);
}

const char *ContactStore::Contact::getAgentName() {
	return ((char*) (StartAddress + sizeof(uint16_t) + PUBLIC_KEY_COMPRESSED_STORAGE_LENGTH + SIGNATURE_LENGTH));
}

uint8_t *ContactStore::Contact::getCompressedPublicKey() {
	return ((uint8_t*) (StartAddress + sizeof(uint16_t)));
}

void ContactStore::Contact::getUnCompressedPublicKey(uint8_t key[PUBLIC_KEY_LENGTH]) {
	uECC_decompress(getCompressedPublicKey(), &key[0], THE_CURVE);
}

uint8_t *ContactStore::Contact::getPairingSignature() {
	return ((uint8_t*) (StartAddress + sizeof(uint16_t) + PUBLIC_KEY_COMPRESSED_STORAGE_LENGTH));
}

void ContactStore::Contact::setUniqueID(uint16_t id) {
	FLASH_LOCKER f;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, StartAddress, id);
}

void ContactStore::Contact::setAgentname(const char name[AGENT_NAME_LENGTH]) {
	//int len = strlen(name);
	FLASH_LOCKER f;
	uint32_t s = StartAddress + sizeof(uint16_t) + PUBLIC_KEY_COMPRESSED_STORAGE_LENGTH + SIGNATURE_LENGTH;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s, (*((uint32_t *) &name[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 4, (*((uint32_t *) &name[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 8, (*((uint32_t *) &name[8])));
}

void ContactStore::Contact::setCompressedPublicKey(const uint8_t key1[PUBLIC_KEY_COMPRESSED_LENGTH]) {
	uint32_t s = StartAddress + sizeof(uint16_t);
	uint8_t key[PUBLIC_KEY_COMPRESSED_STORAGE_LENGTH];
	memset(&key[0], 0, sizeof(key)); //set array to 0
	memcpy(&key[0], &key1[0], PUBLIC_KEY_COMPRESSED_LENGTH); //copy over just the 25 bytes of the compressed public key
	FLASH_LOCKER f;
	//store all bits
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s, (*((uint32_t *) &key[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 4, (*((uint32_t *) &key[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 8, (*((uint32_t *) &key[8])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 12, (*((uint32_t *) &key[12])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 16, (*((uint32_t *) &key[16])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 20, (*((uint32_t *) &key[20])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, s + 24, (*((uint32_t *) &key[24])));
}

void ContactStore::Contact::setPairingSignature(const uint8_t sig[SIGNATURE_LENGTH]) {
	uint32_t s = StartAddress + sizeof(uint16_t) + PUBLIC_KEY_COMPRESSED_STORAGE_LENGTH;
	FLASH_LOCKER f;
	//for(uint32_t i=0;i<sizeof(sig);i+=4) {
	//	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s+i, (*((uint32_t *) &sig[i])));
	//}
	//if you look at the assembler being generated the loop will only run twice so we'll run roll it due to time
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 0, (*((uint32_t *) &sig[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 4, (*((uint32_t *) &sig[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 8, (*((uint32_t *) &sig[8])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 12, (*((uint32_t *) &sig[12])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 16, (*((uint32_t *) &sig[16])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 20, (*((uint32_t *) &sig[20])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 24, (*((uint32_t *) &sig[24])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 28, (*((uint32_t *) &sig[28])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 32, (*((uint32_t *) &sig[32])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 36, (*((uint32_t *) &sig[36])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 40, (*((uint32_t *) &sig[40])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 44, (*((uint32_t *) &sig[44])));
}

//====================================================

ContactStore::MyInfo &ContactStore::getMyInfo() {
	return MeInfo;
}

ContactStore::SettingsInfo &ContactStore::getSettings() {
	return Settings;
}

//ContactStore MyContacts( MyAddressInfoSector, MyAddressInfoOffSet, SettingSector, SettingOffset, StartContactSector, EndContactSector);
//=============================================
ContactStore::ContactStore(uint8_t myAddressInfoSector, uint32_t myAddressInfoOffset, uint8_t settingSector,
		uint32_t settingOffset,
		uint8_t startContactSector, uint8_t endContactSector) :
		Settings(settingSector,settingOffset,settingSector+1),
		MeInfo(SECTOR_TO_ADDRESS(myAddressInfoSector) + myAddressInfoOffset),
		StartingContactSector(startContactSector), EndContactSector(endContactSector) {

}

void ContactStore::resetToFactory() {
	getSettings().resetToFactory();
	{
		FLASH_LOCKER f;
		FLASH_EraseInitTypeDef EraseInitStruct;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
		EraseInitStruct.Sector = StartingContactSector;
		EraseInitStruct.Banks = 0;
		EraseInitStruct.NbSectors = 1;
		EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

		uint32_t SectorError = 0;

		for(int i=StartingContactSector;i<EndContactSector;++i) {
			HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
		}
	}
}

bool ContactStore::init() {
	if (getMyInfo().init() && Settings.init()) {
		//version is good, validate keys:
		uint8_t publicKey[PUBLIC_KEY_LENGTH] = { 0 };
		//given our private key, generate our public key and ensure its good
		if (uECC_compute_public_key(MeInfo.getPrivateKey(), &publicKey[0], THE_CURVE)) {
			if (uECC_valid_public_key(&publicKey[0], THE_CURVE) == 1) {
				return true;
			}
		}
	}
	return false;
}

bool ContactStore::getContactAt(uint16_t numContact, Contact &c) {
	uint8_t currentContacts = Settings.getNumContacts();
	if (numContact < currentContacts) {
		//determine page
		uint16_t sector = StartingContactSector + numContact / CONTACTS_PER_PAGE;
		//valid page is in our range
		if (sector < EndContactSector) {
			uint16_t offSet = numContact % CONTACTS_PER_PAGE;
			uint32_t sectorAddress = SECTOR_TO_ADDRESS(sector);
			c.StartAddress = sectorAddress + (offSet * Contact::SIZE);
			return true;
		}
	}
	return false;
}

bool ContactStore::findContactByID(uint16_t uid, Contact &c) {
	if (uid == 0)
		return false;
	uint8_t currentContacts = Settings.getNumContacts();
	for (int i = 0; i < currentContacts; i++) {
		if (getContactAt(i, c)) {
			if (uid == c.getUniqueID()) {
				return true;
			}
		}
	}
	c.StartAddress = 0;
	return false;
}

bool ContactStore::addContact(uint16_t uid, char agentName[AGENT_NAME_LENGTH],
		uint8_t key[PUBLIC_KEY_COMPRESSED_LENGTH],
		uint8_t sig[SIGNATURE_LENGTH]) {
	uint8_t currentContacts = Settings.getNumContacts();
	uint8_t newNumContacts = Settings.setNumContacts(currentContacts + 1);

	if (newNumContacts == currentContacts) {
		return false;
	}
	Contact c(0xFFFFFFFF);
	if (getContactAt(currentContacts, c)) {
		c.setUniqueID(uid);
		c.setAgentname(agentName);
		c.setCompressedPublicKey(key);
		c.setPairingSignature(sig);
		return true;
	}
	return false;
}

uint8_t ContactStore::getNumContactsThatCanBeStored() {
	return MAX_CONTACTS;
}

