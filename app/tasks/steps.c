#include "cmsis_os2.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "MY_lis2dux12/MY_lis2dux12.h"


void Start_steps_cal_task(void *argument) {
    register_io();
    if (is_lis2dux12_initialized() != 1) {
        init_lis2dux12();
    }
    char msg[50];
    // ------------------- 主循环 -------------------//
    uint16_t steps;

    while (1) {
        // osMutexAcquire(i2c1mutexHandle, osWaitForever);
        // // 读取步数
        // if (read_step(&steps) == 0) {
        //     sprintf(msg, "Steps: %u\r\n", steps);
        //     HAL_UART_Transmit(&test_uart, (uint8_t *) msg, strlen(msg), 100);
        // } else {
        //     HAL_UART_Transmit(&test_uart, (uint8_t *) "Read steps error\r\n", 18, 100);
        // }
        //
        // osMutexRelease(i2c1mutexHandle);
        osDelay(500); // 每 500ms 读取一次
    }
}
