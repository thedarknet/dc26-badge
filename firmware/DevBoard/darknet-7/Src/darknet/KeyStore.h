#ifndef KEY_STORE_H
#define KEY_STORE_H

#include <stm32f4xx_hal.h>
#include "mcu_to_mcu.h"
#include "messaging/stm_to_esp_generated.h"
#include "messaging/esp_to_stm_generated.h"

class ContactStore {
public:
#define THE_CURVE uECC_secp192r1()
	//should trait this in the future for different curves
	static const uint8_t PUBLIC_KEY_LENGTH = 48; //uncompressed size
	static const uint8_t PUBLIC_KEY_COMPRESSED_LENGTH = 25; //compressed size
	static const uint8_t PUBLIC_KEY_COMPRESSED_STORAGE_LENGTH = 26;
	static const uint8_t PRIVATE_KEY_LENGTH = 24;
	static const uint8_t DaemonPublic[PUBLIC_KEY_LENGTH];
	static const uint8_t SIGNATURE_LENGTH = 48;
	static const uint8_t AGENT_NAME_LENGTH = 12;
	static const uint8_t CURRENT_VERSION = 0xDC;

	// storage sizes, all must be 4 byte aligned
	static const uint8_t _SIZE_OF_ID_STORAGE      = (4);
	static const uint8_t _SIZE_OF_PUBKEY_STORAGE  = (28);
	static const uint8_t _SIZE_OF_SIG_STORAGE     = (48);
	static const uint8_t _SIZE_OF_NAME_STORAGE    = (12);
	static const uint8_t _SIZE_OF_CONTACT         = (_SIZE_OF_ID_STORAGE + _SIZE_OF_PUBKEY_STORAGE + _SIZE_OF_SIG_STORAGE + _SIZE_OF_NAME_STORAGE);
	static const uint8_t _OFFSET_OF_ID            = (0);
	static const uint8_t _OFFSET_OF_PUBKEY        = (_SIZE_OF_ID_STORAGE);
	static const uint8_t _OFFSET_OF_SIG           = (_OFFSET_OF_PUBKEY + _SIZE_OF_PUBKEY_STORAGE);
	static const uint8_t _OFFSET_OF_AGENT_NAME    = (_OFFSET_OF_SIG + _SIZE_OF_SIG_STORAGE);

	static const uint8_t MAX_CONTACTS = 186; //16384/88;
	static const uint8_t CONTACTS_PER_PAGE = MAX_CONTACTS; //for STM32F411 1 sector is being used of 16K so numbers are the same
	/////////////////////////////
	// Sector 57: erase then rotate though saving SettingInfo
	//				  	byte 0: 0xDC
	//					byte 1: 0xDC
	//					byte 2: 0xDC
	//					byte 3: 0xDC
	//					byte 4: 0
	//					byte 5: <number of contacts>
	//					byte 6: bits 0-3: screen save type
	//					byte 6: bits 4-7: sleep timer
	//					byte 7: bits 0-3: screen saver time
	//					byte 7: bits 4-7: ?
	//					byte 8-19: Agent Name
	// Sector 58:
	//				0-87 Contact 0
	//				...
	//				880-967 Contact 10
	//
	//	Sector 59:
	//				0-87 Contact 11
	//				...
	//				880-967 Contact 22
	//
	//	...
	//	Sector 63:
	//				0-87 Contact 55
	//				...
	//				880-967 Contact 66

	//			980-1020 My Info
	//					My Info Address[0-1] = 0xdcdc
	//					My Info Address[2-3] = radio unique id
	//					My Info Address[4-27] = badge owner private key
	//					My Info Address[28-29] = static settings
	//[ address book ]
	//		Contact
	//				[0-3] unique id
	//				[4-31] public key (compressed version - 25 (26) bytes)
	//				[32-79] Signature (contact signs you're id+public key)
	//				[80-91] Agent name
	//start address[(every 46 bytes)] = Contact1
	/////////////////////////////

	class Contact {
	public:
		friend class ContactStore;
		static const uint8_t SIZE = _SIZE_OF_CONTACT;

		uint16_t getUniqueID();
		const char *getAgentName();
		uint8_t *getCompressedPublicKey();
		void getUnCompressedPublicKey(uint8_t key[PUBLIC_KEY_LENGTH]);
		uint8_t *getPairingSignature();
		void setUniqueID(uint16_t id);
		void setAgentname(const char name[AGENT_NAME_LENGTH]);
		void setCompressedPublicKey(const uint8_t key[PUBLIC_KEY_COMPRESSED_LENGTH]);
		void setPairingSignature(const uint8_t sig[SIGNATURE_LENGTH]);
		Contact(uint32_t startAddress = 0);
		protected:
		uint32_t StartAddress;
	};

	/*
	 *
		InfectionID				vector		remedy			permanence		contraction%
		Clear Bit (Bit:0)  		Wireless  	Operative Only						100%
		Avian Flu (Bit: 1)		Wireless	AGTAGAAACAAGG		Recurring		12%
		Measles (Bit: 2)		Wireless	GTCAGTTCCACAT		Curable			90%
		Tetanus (Bit: 3)		BLE GATT	GAGGTGCAGCTGG		Recurring		50%
		Polio (Bit: 4)			BLE GATT	ATTCTAACCATGG		Curable			13%
		Plague (Bit: 5)			BLE GATT	AAGAGTATAATCG / DN Table Curable	50%
		Toxoplasmosis (Bit: 6)  BLE GATT	CCTAAACCCTGAA		Recurring		20%
		Chlamydia (Bit: 7) 		Add-On		GTATTAGTATTTG		Curable			50%
		Herpes (Bit: 8)			Add-On		GATCGTTATTCCC		Recurring		30%
	 *
	 *
	 */
	class SettingsInfo {
	public:
		enum {
			CLEAR_ALL = 0x1,
			AVIAN_FLU = 0x2,
			MEASLES	 = 0x4,
			TETANUS = 0x8,
			POLIO 	= 0x10,
			PLAGUE	= 0x20,
			TOXOPLASMOSIS = 0x40,
			CHLAMYDIA	=	0x80,
			HERPES		=	0x100
		};
		static const uint32_t SETTING_MARKER = 0xDCDCDCDC;
		static const uint8_t SIZE = 8 + AGENT_NAME_LENGTH;
		struct DataStructure {
			uint32_t Health :12;
			uint32_t NumContacts :8;
			uint32_t ScreenSaverType :4;
			uint32_t SleepTimer :4;
			uint32_t ScreenSaverTime :4;
		};
	public:
		SettingsInfo(uint8_t sector, uint32_t offSet, uint8_t endSector);
		void receiveSignal(MCUToMCU*, const MSGEvent<darknet7::BLEInfectionData>* mevt);
		bool init();
		uint32_t getVersion();
		uint8_t setNumContacts(uint8_t n);
		uint8_t getNumContacts();
		bool getContactAt(uint8_t n, Contact &c);
		bool setScreenSaverType(uint8_t value);
		uint8_t getScreenSaverType();
		bool setScreenSaverTime(uint8_t value);
		uint8_t getScreenSaverTime();
		bool setSleepTime(uint8_t n);
		uint8_t getSleepTime();
		const char *getAgentName();
		bool isNameSet();
		bool setAgentname(const char name[AGENT_NAME_LENGTH]);
		void resetToFactory();
		bool setHealth(uint16_t v);
		uint16_t getHealth();
		bool isInfectedWith(uint16_t v);
		bool cure(uint16_t v);
	protected:
		bool writeSettings(const DataStructure &ds);
		DataStructure getSettings();
		uint32_t getStartAddress();
		uint32_t getEndAddress();
	private:
		uint16_t SettingSector;
		uint32_t OffSet;
		uint8_t EndSettingSector;
		uint32_t CurrentAddress;
		char AgentName[AGENT_NAME_LENGTH];
	};

	class MyInfo {
	public:
		//								0xdcdc			//radio id								//static settings
		static const uint8_t SIZE = sizeof(uint16_t) + sizeof(uint16_t) + PRIVATE_KEY_LENGTH + sizeof(uint16_t);
	public:
		MyInfo(uint32_t startAddress);
		bool init();
		uint16_t getUniqueID();
		uint8_t *getPublicKey();
		uint8_t *getPrivateKey();
		bool isUberBadge();
		uint8_t *getCompressedPublicKey();
	protected:
		uint16_t getFlags();
	private:
		uint32_t StartAddress;
	};

public:
	ContactStore(uint8_t myAddressInfoSector, uint32_t myAddressInfoOffset, uint8_t settingSector, uint32_t settingOffset,
			uint8_t startContactSector, uint8_t endContactSector);
	MyInfo &getMyInfo();
	SettingsInfo &getSettings();
	bool init();
	bool addContact(uint16_t uid, char agentName[AGENT_NAME_LENGTH], uint8_t key[PUBLIC_KEY_LENGTH],
			uint8_t sig[SIGNATURE_LENGTH]);
	uint8_t getNumContactsThatCanBeStored();
	bool getContactAt(uint16_t numContact, Contact &c);
	bool findContactByID(uint16_t uid, Contact &c);
	void resetToFactory();
	private:
	SettingsInfo Settings;
	MyInfo MeInfo;
	uint8_t StartingContactSector;
	uint8_t EndContactSector;
};

#endif
