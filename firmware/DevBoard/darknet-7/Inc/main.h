/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define MID_BUTTON1_Pin GPIO_PIN_13
#define MID_BUTTON1_GPIO_Port GPIOC
#define BUTTON_RIGHT_Pin GPIO_PIN_14
#define BUTTON_RIGHT_GPIO_Port GPIOC
#define BUTTON_FIRE1_Pin GPIO_PIN_15
#define BUTTON_FIRE1_GPIO_Port GPIOC
#define LCD_DATA_CMD_Pin GPIO_PIN_0
#define LCD_DATA_CMD_GPIO_Port GPIOC
#define LCD_CS_Pin GPIO_PIN_1
#define LCD_CS_GPIO_Port GPIOC
#define BUTTON_LEFT_Pin GPIO_PIN_2
#define BUTTON_LEFT_GPIO_Port GPIOC
#define SPI2_LCD_MOSI_Pin GPIO_PIN_3
#define SPI2_LCD_MOSI_GPIO_Port GPIOC
#define SD_CS_Pin GPIO_PIN_4
#define SD_CS_GPIO_Port GPIOC
#define LCD_RESET_Pin GPIO_PIN_5
#define LCD_RESET_GPIO_Port GPIOC
#define LCD_BACKLIGHT_Pin GPIO_PIN_2
#define LCD_BACKLIGHT_GPIO_Port GPIOB
#define SPI2_LCD_SCK_Pin GPIO_PIN_10
#define SPI2_LCD_SCK_GPIO_Port GPIOB
#define SPI3_SD_SCK_Pin GPIO_PIN_12
#define SPI3_SD_SCK_GPIO_Port GPIOB
#define GPIO_APA106_DATA_Pin GPIO_PIN_7
#define GPIO_APA106_DATA_GPIO_Port GPIOC
#define GPIO_APA102_DATA_Pin GPIO_PIN_8
#define GPIO_APA102_DATA_GPIO_Port GPIOC
#define TIM_APA102_CLK_Pin GPIO_PIN_9
#define TIM_APA102_CLK_GPIO_Port GPIOC
#define SHITTY_ADD_ON_BADGE_I2C_SCL_Pin GPIO_PIN_8
#define SHITTY_ADD_ON_BADGE_I2C_SCL_GPIO_Port GPIOA
#define USART1_TX_TO_ESP_RX_PIN_5_Pin GPIO_PIN_9
#define USART1_TX_TO_ESP_RX_PIN_5_GPIO_Port GPIOA
#define USART1_RX_TO_ESP_TX_PIN_4_Pin GPIO_PIN_10
#define USART1_RX_TO_ESP_TX_PIN_4_GPIO_Port GPIOA
#define SIMPLE_LED1_Pin GPIO_PIN_10
#define SIMPLE_LED1_GPIO_Port GPIOC
#define SPI3_SD_MISO_Pin GPIO_PIN_11
#define SPI3_SD_MISO_GPIO_Port GPIOC
#define SPI3_SD_MOSI_Pin GPIO_PIN_12
#define SPI3_SD_MOSI_GPIO_Port GPIOC
#define ESP_INTER_Pin GPIO_PIN_2
#define ESP_INTER_GPIO_Port GPIOD
#define ESP_INTER_EXTI_IRQn EXTI2_IRQn
#define SHITTY_ADD_ON_BADGE_I2C_SDA_Pin GPIO_PIN_4
#define SHITTY_ADD_ON_BADGE_I2C_SDA_GPIO_Port GPIOB
#define ESP_I2C1_SCL_Pin GPIO_PIN_6
#define ESP_I2C1_SCL_GPIO_Port GPIOB
#define ESP_I2C1_SDA_Pin GPIO_PIN_7
#define ESP_I2C1_SDA_GPIO_Port GPIOB
#define BUTTON_UP_Pin GPIO_PIN_8
#define BUTTON_UP_GPIO_Port GPIOB
#define BUTTON_DOWN_Pin GPIO_PIN_9
#define BUTTON_DOWN_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
 #define USE_FULL_ASSERT    1U 

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
