#include <string.h>

#include "cmsis_os2.h"
#include "usart.h"
#include "main.h"
char message[100];
int tianqi_flag = 0;
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    tianqi_flag = 1;
}
void Start_communication_task(void *argument) {

    HAL_UARTEx_ReceiveToIdle_DMA(&TIANQI_UART,message,100);
    osDelay(10);
    HAL_UART_Transmit(&TIANQI_UART,"AT+CGMI?\n",9,HAL_MAX_DELAY);

    while (1) {
        //HAL_UART_Transmit(&test_uart,message,strlen(message),HAL_MAX_DELAY);
        if (tianqi_flag==1) {
            tianqi_flag = 0;
            HAL_UART_Transmit(&test_uart,message,strlen(message),HAL_MAX_DELAY);
        }
        osDelay(100);
    }
}
