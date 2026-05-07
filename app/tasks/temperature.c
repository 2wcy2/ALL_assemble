#include <string.h>
#include "AHT20-F/aht20.h"
#include "cmsis_os2.h"
#include "usart.h"
#include "main.h"

void triger_cal(float *tem,float *hum) {
    HAL_GPIO_WritePin(AHT20_PWR_GPIO_Port, AHT20_PWR_Pin, GPIO_PIN_SET);
    osDelay(10);
    AHT20_Init();
    osDelay(20);
    osMutexAcquire(i2c1mutexHandle,osWaitForever);
    AHT20_Read(tem,hum);
    osMutexRelease(i2c1mutexHandle);
}

void Start_temp_cal_task(void *argument) {
    // float tem,hum;
    // triger_cal(&tem,&hum);
    // char str[50];
    // sprintf(str,"Temperature: %.2f C",tem);
    // HAL_UART_Transmit(&test_uart,(uint8_t *)str,strlen(str),1000);
    while (1) {
        osDelay(10);
    }
 }