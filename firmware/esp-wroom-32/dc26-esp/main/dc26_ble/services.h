#include "esp_system.h"
#include "esp_log.h"
#include "ble.h"

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

// Serial Message display characteristics
#define SERIAL_CISO_UUID   "646e0012-0001-0002-0003-000000000001"
#define SERIAL_COSI_UUID   "646e0012-0001-0002-0003-000000000002"
static BLEUUID serialCisoUUID(SERIAL_CISO_UUID);
static BLEUUID serialCosiUUID(SERIAL_COSI_UUID);
#define serialCisoCharProps (BLECharacteristic::PROPERTY_READ | \
                           BLECharacteristic::PROPERTY_NOTIFY)
#define serialCosiCharProps (BLECharacteristic::PROPERTY_WRITE)

