#include "gpio.h"
#include "main.h"
#include "usart.h"
#include <string.h>
int rx_4g_flag=0;
char message_rx_4g[1000]="";
char message_trans_4g[200]="";
uint16_t len_test, len_4g;
void Start_4g_commun() {
    HAL_GPIO_WritePin(PWR_4G_GPIO_Port,PWR_4G_Pin,1);
    HAL_Delay(500);
    HAL_UARTEx_ReceiveToIdle_DMA(&test_uart,(uint8_t*)message_trans_4g,200);
    HAL_UARTEx_ReceiveToIdle_DMA(&UART_4g,(uint8_t*)message_rx_4g,1000);
    //HAL_UART_Transmit(&UART_4g,"AT+CLAC\r\n",11,HAL_MAX_DELAY);
    while (1) {
        if (rx_test_flag == 1) {
            rx_test_flag=0;
            HAL_UART_Transmit(&UART_4g,(uint8_t*)message_trans_4g,len_test,HAL_MAX_DELAY);
            memset(message_trans_4g,'\0',sizeof(message_trans_4g));
            HAL_UARTEx_ReceiveToIdle_DMA(&test_uart,(uint8_t*)message_trans_4g,200);
        }
        else if (rx_4g_flag == 1) {
            rx_4g_flag=0;
            HAL_UART_Transmit(&test_uart,(uint8_t*)message_rx_4g,len_4g,HAL_MAX_DELAY);
            memset(message_rx_4g,'\0',sizeof(message_rx_4g));
            HAL_UARTEx_ReceiveToIdle_DMA(&UART_4g,(uint8_t*)message_rx_4g,1000);
        }
        osDelay(1);
    }
}