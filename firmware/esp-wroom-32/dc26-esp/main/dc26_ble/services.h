#include "esp_system.h"
#include "esp_log.h"
#include "ble.h"

void setCharacteristicValue(BLEService* pService, BLEUUID* uuid, uint8_t* data, size_t size);


// Pairing and CISO/COSI (Client In/Server Out, Client Out/Server In)
#define PAIR_SERVICE_UUID  "64633236-6463-646e-3230-313870616972"
#define PAIRING_CISO_UUID  "646e0001-0001-0002-0003-000000000001"
#define PAIRING_COSI_UUID  "646e0001-0001-0002-0003-000000000002"
static BLEUUID uartServiceUUID(PAIR_SERVICE_UUID);
static BLEUUID uartCisoUUID(PAIRING_CISO_UUID);
static BLEUUID uartCosiUUID(PAIRING_COSI_UUID);
#define uartCisoCharProps (BLECharacteristic::PROPERTY_READ | \
                           BLECharacteristic::PROPERTY_NOTIFY)
#define uartCosiCharProps (BLECharacteristic::PROPERTY_WRITE)

// Gulo-tachi feeding
#define FOOD_STATION_CHAR  "646e0002-0001-0002-0003-000000000001"
#define WATER_STATION_CHAR "646e0002-0001-0002-0003-000000000002"
#define CARE_STATION_CHAR  "646e0002-0001-0002-0003-000000000004"
static BLEUUID foodUUID(FOOD_STATION_CHAR);
static BLEUUID waterUUID(WATER_STATION_CHAR);
static BLEUUID careUUID(CARE_STATION_CHAR);
#define foodCharProps (BLECharacteristic::PROPERTY_READ)
#define waterCharProps (BLECharacteristic::PROPERTY_READ)
#define careCharProps (BLECharacteristic::PROPERTY_READ)

// Gulo-tachi stats
#define GULO_STR_CHAR      "646e0004-0001-0002-0003-000000000000"
#define GULO_VIT_CHAR      "646e0004-0001-0002-0003-000000000001"
#define GULO_DEX_CHAR      "646e0004-0001-0002-0003-000000000002"
#define GULO_INT_CHAR      "646e0004-0001-0002-0003-000000000004"
#define GULO_MND_CHAR      "646e0004-0001-0002-0003-000000000008"
#define GULO_HP_CHAR       "646e0004-0001-0002-0003-000000000010"
#define GULO_MP_CHAR       "646e0004-0001-0002-0003-000000000012"
#define GULO_TP_CHAR       "646e0004-0001-0002-0003-000000000014"
#define GULO_CRIT_CHAR     "646e0004-0001-0002-0003-000000000018"
static BLEUUID strUUID(GULO_STR_CHAR);
static BLEUUID vitUUID(GULO_VIT_CHAR);
static BLEUUID dexUUID(GULO_DEX_CHAR);
static BLEUUID intUUID(GULO_INT_CHAR);
static BLEUUID mndUUID(GULO_MND_CHAR);
static BLEUUID hpUUID(GULO_HP_CHAR);
static BLEUUID mpUUID(GULO_MP_CHAR);
static BLEUUID tpUUID(GULO_TP_CHAR);
static BLEUUID critUUID(GULO_CRIT_CHAR);
#define strCharProps (BLECharacteristic::PROPERTY_READ)
#define vitCharProps (BLECharacteristic::PROPERTY_READ)
#define dexCharProps (BLECharacteristic::PROPERTY_READ)
#define intCharProps (BLECharacteristic::PROPERTY_READ)
#define mndCharProps (BLECharacteristic::PROPERTY_READ)
#define hpCharProps (BLECharacteristic::PROPERTY_READ)
#define mpCharProps (BLECharacteristic::PROPERTY_READ)
#define tpCharProps (BLECharacteristic::PROPERTY_READ)
#define critCharProps (BLECharacteristic::PROPERTY_READ)

// NPC Characteristics
#define RAT_CHAR           "646e0008-0001-0002-0003-000000007001"
#define CAT_CHAR           "646e0008-0001-0002-0003-000000007101"
#define FLEA_CHAR          "646e0008-0001-0002-0003-000000007201"
#define PMASK_CHAR         "646e0008-0001-0002-0003-000000005150"
static BLEUUID ratUUID(RAT_CHAR);
static BLEUUID catUUID(CAT_CHAR);
static BLEUUID fleaUUID(FLEA_CHAR);
static BLEUUID pmaskUUID(PMASK_CHAR);
#define ratCharProps (BLECharacteristic::PROPERTY_READ)
#define catCharProps (BLECharacteristic::PROPERTY_READ)
#define fleaCharProps (BLECharacteristic::PROPERTY_READ)
#define pmaskCharProps (BLECharacteristic::PROPERTY_READ)

// Infection
#define TOX_INFECT_CHAR    "646e0010-0001-0002-0003-000000000000"
#define POL_INFECT_CHAR    "646e0010-0001-0002-0003-000000000001"
#define PLA_INFECT_CHAR    "646e0010-0001-0002-0003-000000000002"
#define TET_INFECT_CHAR    "646e0010-0001-0002-0003-000000000004"
#define BOOP_INFECT_CHAR   "646e0010-0001-0002-0003-000000000008"
static BLEUUID toxUUID(TOX_INFECT_CHAR);
static BLEUUID polUUID(POL_INFECT_CHAR);
static BLEUUID plaUUID(PLA_INFECT_CHAR);
static BLEUUID tetUUID(TET_INFECT_CHAR);
static BLEUUID boopedUUID(BOOP_INFECT_CHAR);
#define toxCharProps (BLECharacteristic::PROPERTY_READ)
#define polCharProps (BLECharacteristic::PROPERTY_READ)
#define plaCharProps (BLECharacteristic::PROPERTY_READ)
#define tetCharProps (BLECharacteristic::PROPERTY_READ)
#define boopCharProps (BLECharacteristic::PROPERTY_READ)

// The Gourry Special
#define GOURRY_DOODLED_YOU "646e0011-0001-0002-0003-000000000000"
#define GOURRY_DINGLED_YOU "646e0011-0001-0002-0003-000000000000"
static BLEUUID doodleUUID(GOURRY_DOODLED_YOU);
static BLEUUID dingleUUID(GOURRY_DINGLED_YOU);
#define doodleCharProps (BLECharacteristic::PROPERTY_READ)
#define dingleCharProps (BLECharacteristic::PROPERTY_READ)

// Doodle Characteristics
#define DEVICE_ID_CHAR     "646e0088-0001-0002-0003-000000000000"
static BLEUUID idUUID(DEVICE_ID_CHAR);
#define idCharProps (BLECharacteristic::PROPERTY_READ)
