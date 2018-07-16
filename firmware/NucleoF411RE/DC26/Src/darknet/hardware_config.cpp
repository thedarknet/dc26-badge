/*
 * hardware_config.cpp
 *
 *  Created on: Nov 4, 2017
 *      Author: cmdc0de
 */


#include "libstm32/display/display_device.h"
#include "libstm32/sensors/bmp280/bmp280.h"
#include <spi.h>
#include <main.h>
#include <i2c.h>

static cmdc0de::PinConfig BackLit(LCD_BACKLIGHT_GPIO_Port,LCD_BACKLIGHT_Pin);
static cmdc0de::PinConfig DataCmd(LCD_DATA_CMD_GPIO_Port,LCD_DATA_CMD_Pin);
static cmdc0de::PinConfig DisplayCS(LCD_CS_GPIO_Port,LCD_CS_Pin);
static cmdc0de::PinConfig DiplayReset(LCD_RESET_GPIO_Port,LCD_RESET_Pin);


const cmdc0de::PinConfig & cmdc0de::DisplayST7735::HardwareConfig::getBackLit() {
	return BackLit;
}

SPI_HandleTypeDef *cmdc0de::DisplayST7735::HardwareConfig::getSPI() {
	return &hspi2;
}

const cmdc0de::PinConfig &cmdc0de::DisplayST7735::HardwareConfig::getDataCmd() {
	return DataCmd;
}

const cmdc0de::PinConfig &cmdc0de::DisplayST7735::HardwareConfig::getCS() {
	return DisplayCS;
}

const cmdc0de::PinConfig &cmdc0de::DisplayST7735::HardwareConfig::getReset() {
	return DiplayReset;
}

I2C_HandleTypeDef *cmdc0de::BMP280::Hardware::getI2C(uint8_t i2caddress) {
	return &hi2c1;
}
