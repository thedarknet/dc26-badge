#ifndef ESP32_I2CDEVICE_H_
#define ESP32_I2CDEVICE_H_

//#include "i2cdevice.h"
#include <esp_types.h>
#include "driver/i2c.h"
#include "rom/ets_sys.h"


class ESP32_I2CDevice {
protected:
	ESP32_I2CDevice(gpio_num_t scl, gpio_num_t sda, i2c_port_t p, uint32_t rxBufSize, uint32_t txBufSize);
public:
	~ESP32_I2CDevice();
	gpio_num_t getSCL() const {return SCL;}
	gpio_num_t getSDA() const {return SDA;}
	i2c_port_t getPort() const {return Port;}
 	uint16_t getRXBufferSize() const {return RXBufferSize;}
	uint16_t getTXBufferSize() const {return TXBufferSize;}
	void deinit();
protected:
	gpio_num_t SCL;
	gpio_num_t SDA;
	i2c_port_t Port;
	uint16_t RXBufferSize;
	uint16_t TXBufferSize;
};


class ESP32_I2CSlaveDevice : public ESP32_I2CDevice {
public:
	ESP32_I2CSlaveDevice(gpio_num_t scl, gpio_num_t sda, uint8_t address, i2c_port_t p, uint32_t rxBufSize, uint32_t txBufSize );
public:
	bool init();
	/* writes the data on the next request from the master
	 * @return number of bytes written or -1 for error
	 */
	int write(uint8_t *data, uint16_t size, uint16_t ticksToWait);
	/* reads the data from master
	 * @return number of bytes read or -1 for error
	 */
	int read(uint8_t *data, uint16_t size, uint16_t ticksToWait);
private:
	uint8_t Address;
};

class ESP32_I2CMaster : public ESP32_I2CDevice {
public:
		enum ACK_TYPE {
			ACK_EVERY_BYTE = 0
				, NACK_EVERY_BYTE = 1
				, NACK_LAST_BYTE = 2
		};
public:
	ESP32_I2CMaster(gpio_num_t scl, gpio_num_t sda, uint32_t clock, i2c_port_t p, uint32_t rxBufSize, uint32_t txBufSize );
	~ESP32_I2CMaster();
	bool init();
	bool start(uint8_t addr, bool forWrite);
	bool write(uint8_t *data, uint16_t size, bool ack);
	bool read(uint8_t *data, uint16_t size, ACK_TYPE ackType);
	bool stop(uint16_t ticksToWait);
	uint8_t read(void);
private:
	uint32_t MasterClock;
	i2c_cmd_handle_t CmdHandle;
};

#endif

