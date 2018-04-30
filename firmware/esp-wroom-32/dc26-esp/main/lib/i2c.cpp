#include <esp_log.h>
#include "i2c.hpp"
#include "driver/i2c.h"

static const char tag[] = "I2C";

ESP32_I2CDevice::ESP32_I2CDevice(gpio_num_t scl, gpio_num_t sda, i2c_port_t p, uint32_t rxBufSize, uint32_t txBufSize) 
	: SCL(scl), SDA(sda), Port(p), RXBufferSize(rxBufSize), TXBufferSize(txBufSize) {

	
}

void ESP32_I2CDevice::deinit() {
	if(I2C_NUM_MAX!=Port) {
		i2c_driver_delete(Port);
		Port = I2C_NUM_MAX;
	}
}

ESP32_I2CDevice::~ESP32_I2CDevice() {
	deinit();
}

////////////////////////////////////////////////////////////////////////////
//
ESP32_I2CSlaveDevice::ESP32_I2CSlaveDevice(gpio_num_t scl, gpio_num_t sda, uint8_t address, i2c_port_t p, uint32_t rxBufSize, uint32_t txBufSize ) 
	: ESP32_I2CDevice(scl,sda,p,rxBufSize,txBufSize), Address(address) {

}


bool ESP32_I2CSlaveDevice::init() {
	bool bRetVal = false;
	i2c_config_t conf;
	conf.mode = I2C_MODE_SLAVE;
	conf.sda_io_num = SDA;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = SCL;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.slave.addr_10bit_en = 0;
	conf.slave.slave_addr = Address;

	if(ESP_OK==i2c_param_config(Port,&conf)) {
		if(ESP_OK==i2c_driver_install(Port, conf.mode, RXBufferSize, TXBufferSize, 0)) {
			bRetVal = true;
		}
	}
	return bRetVal;
}


int ESP32_I2CSlaveDevice::write(uint8_t *data, uint16_t size, uint16_t ticksToWait) {
	return i2c_slave_write_buffer(getPort(), data, size, ticksToWait );
}

int ESP32_I2CSlaveDevice::read(uint8_t *data, uint16_t size, uint16_t ticksToWait ) {
	return i2c_slave_read_buffer(getPort(), data, size, ticksToWait);
}

////////////////////////////////////////////////////////////////////////////
ESP32_I2CMaster::ESP32_I2CMaster(gpio_num_t scl, gpio_num_t sda, uint32_t clock, i2c_port_t p, uint32_t rxBufSize, uint32_t txBufSize ) 
	: ESP32_I2CDevice(scl,sda,p,rxBufSize,txBufSize), MasterClock(clock), CmdHandle(0) {

}

ESP32_I2CMaster::~ESP32_I2CMaster() {

}

bool ESP32_I2CMaster::init() {
	bool bRetVal = false;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = SDA;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = SCL;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = MasterClock;
	if(ESP_OK==i2c_param_config(Port,&conf)) {
		if(ESP_OK==i2c_driver_install(Port, conf.mode, RXBufferSize, TXBufferSize, 0)) {
			bRetVal = true;
		} else {
			ESP_LOGE(tag,"ESP_OK==i2c_driver_install");
		}
	} else {
		ESP_LOGE(tag,"ESP_OK==i2c_param_config");
	}
	return bRetVal;
}

bool ESP32_I2CMaster::start(uint8_t addr, bool forWrite) {
	bool bRetVal = false;
	if((CmdHandle=i2c_cmd_link_create())!=0) {
		if(ESP_OK==i2c_master_start(CmdHandle)) {
			if(forWrite) {
				if(ESP_OK==i2c_master_write_byte(CmdHandle, addr<<1|I2C_MASTER_WRITE, true)) { //check the ack since its a command
					bRetVal = true;
				} else {
					ESP_LOGE(tag,"WRITE:  ESP_OK==i2c_master_write_byte");
				}
			} else {
				if(ESP_OK==i2c_master_write_byte(CmdHandle, addr<<1|I2C_MASTER_READ, true)) { //check the ack since its a command
					bRetVal = true;
				} else {
					ESP_LOGE(tag,"READ:  ESP_OK==i2c_master_write_byte");
				}
			}
		} else {
			ESP_LOGE(tag,"ESP_OK==i2c_master_start");
		}
	} else {
		ESP_LOGE(tag,"CmdHandle=i2c_cmd_link_create");
	}
	return bRetVal;
}

bool ESP32_I2CMaster::write(uint8_t *data, uint16_t size, bool ack) {
	return ESP_OK==i2c_master_write(CmdHandle,data,size,ack);
}

bool ESP32_I2CMaster::stop(uint16_t ticksToWait) {
	bool bRetVal = false;
	if(ESP_OK==i2c_master_stop(CmdHandle)) {
	   esp_err_t retVal;
		if((retVal=i2c_master_cmd_begin(Port, CmdHandle, ticksToWait))==ESP_OK) {
			bRetVal = true;
		} else {
			ESP_LOGE(tag,"ESP_OK==i2c_master_cmd_begin: %s\n",esp_err_to_name(retVal));
		}
	} else {
		ESP_LOGE(tag,"ESP_OK==i2c_master_stop");
	}
	return bRetVal;
}

bool ESP32_I2CMaster::read(uint8_t *data, uint16_t size, ACK_TYPE ackType) {
	return ESP_OK==i2c_master_read(CmdHandle, data, size, (i2c_ack_type_t) ackType);
}


