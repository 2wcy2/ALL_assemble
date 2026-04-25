#include <string.h>
#include "aht20.h"
#include "cmsis_os2.h"
#include "usart.h"
#include "main.h"

void triger_cal() {

}

void Start_temp_cal_task(void *argument) {
    HAL_GPIO_WritePin(AHT20_PWR_GPIO_Port, AHT20_PWR_Pin, GPIO_PIN_SET);
    float hum,tem;
    AHT20_Init();
    osDelay(10);
    char message[50];
    while (1) {
        osMutexAcquire(i2c1mutexHandle,osWaitForever);
        AHT20_Read(&tem,&hum);
        osMutexRelease(i2c1mutexHandle);
        osDelay(10);
        sprintf(message,"%.2f\n",tem);
        HAL_UART_Transmit(&test_uart,message,strlen(message),HAL_MAX_DELAY);
    }
 }