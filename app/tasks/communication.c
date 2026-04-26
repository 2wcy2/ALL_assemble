#include <string.h>
#include "cmsis_os2.h"
#include "usart.h"
#include "main.h"
char message[1000];
int tianqi_flag = 0;
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    tianqi_flag = 1;
    HAL_UART_Transmit(&test_uart,message,strlen(message),HAL_MAX_DELAY);
    HAL_UARTEx_ReceiveToIdle_DMA(&TIANQI_UART,message,1000);
}
void Start_communication_task(void *argument) {

    HAL_UARTEx_ReceiveToIdle_DMA(&TIANQI_UART,message,1000);
    osDelay(200);
    // HAL_UART_Transmit(&TIANQI_UART,"ATE1\r\n",9,HAL_MAX_DELAY);
    // osDelay(1000);
    HAL_UART_Transmit(&TIANQI_UART,"AT+CLAC?\r\n",9,HAL_MAX_DELAY);
    osDelay(1000);
    while (1) {
        osDelay(100);
    }
}
