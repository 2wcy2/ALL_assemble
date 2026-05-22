/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "stdio.h"
#include "task.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
  extern osMutexId_t i2c1mutexHandle;   // 声明 I2C 互斥量句柄
  extern int rx_tianqi_flag;
  extern int rx_4g_flag;
  extern int rx_gps_flag;
  extern int rx_test_flag;
  extern uint16_t len_test;
  extern uint16_t len_4g;
  extern TaskHandle_t xGpsTaskHandle;
  extern osThreadId_t info_assemble_tHandle;
  extern osMessageQueueId_t info_transHandle;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_6
#define LED_GPIO_Port GPIOA
#define AHT20_PWR_Pin GPIO_PIN_1
#define AHT20_PWR_GPIO_Port GPIOB
#define PWR_4G_Pin GPIO_PIN_2
#define PWR_4G_GPIO_Port GPIOB
#define EEPROM_WP_Pin GPIO_PIN_3
#define EEPROM_WP_GPIO_Port GPIOB
#define EEPROM_PWR_Pin GPIO_PIN_4
#define EEPROM_PWR_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define huart1 GPS_UART
#define huart2 UART_4g
#define huart3 TIANQI_UART
#define hlpuart1  test_uart
#define FLAG_SENSOR_READY  0x01
#define FLAG_GNSS_READY    0x02
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
