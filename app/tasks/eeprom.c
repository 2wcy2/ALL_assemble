#include <string.h>

#include "cmsis_os2.h"
#include "usart.h"
#include "M24c64/m24c64.h"
void Start_store_task(void *argument) {
    osDelay(100);
    // M24C64_ON();
    // M24C64_abble_write();
    // HAL_StatusTypeDef rt=osMutexAcquire(i2c1mutexHandle, osWaitForever);
    // char trans[]="42",message[50];
    // M24C64_WritePage(0,trans,strlen(trans));
    // osMutexRelease(i2c1mutexHandle);
    // M24C64_disabble_write();
    // uint8_t data;
    // M24C64_ReadSeq(0,message,50);
    // HAL_UART_Transmit(&test_uart, (uint8_t *)message, 50, HAL_MAX_DELAY);
    // M24C64_OFF();
    while (1) {
        osDelay(1000);
    }
}
