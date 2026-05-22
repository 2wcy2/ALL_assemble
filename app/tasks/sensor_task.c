#include "adc.h"
#include"main.h"
#include "global/animal_state.h"
#include "cmsis_os2.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "MY_lis2dux12/MY_lis2dux12.h"
#include "AHT20-F/aht20.h"



void triger_tem_cal(float *tem,float *hum) {
    HAL_GPIO_WritePin(AHT20_PWR_GPIO_Port, AHT20_PWR_Pin, GPIO_PIN_SET);
    osDelay(100);
    AHT20_Init();
    osDelay(20);
    osMutexAcquire(i2c1mutexHandle,osWaitForever);
    AHT20_Read(tem,hum);
    osMutexRelease(i2c1mutexHandle);
}


void Start_SensorTask(void *argument) {
    register_io();
    if (is_lis2dux12_initialized() != 1) {
        init_lis2dux12();
    }
    // ------------------- 主循环 -------------------//
    uint16_t steps;

    while (1) {
        osMutexAcquire(i2c1mutexHandle, osWaitForever);
        // 读取步数
        if (read_step(&steps) == 0) {
            animal_state.steps=steps;
            // sprintf(msg, "Steps: %u\r\n", steps);
            // HAL_UART_Transmit(&test_uart, (uint8_t *) msg, strlen(msg), 100);
        } else {
            HAL_UART_Transmit(&test_uart, (uint8_t *) "Read steps error\r\n", 18, 100);
        }

        osMutexRelease(i2c1mutexHandle);


        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 2000)==HAL_OK) {
            uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
            float voltage = (adc_value * 3.3f) / 4095.0f;
            animal_state.voltage = 2.0*voltage;
        }


        float tem,hum;
        triger_tem_cal(&tem,&hum);
        animal_state.temperature=tem;



        osThreadFlagsSet(info_assemble_tHandle, FLAG_SENSOR_READY);
        osDelay(5000);
    }
}