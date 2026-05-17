#include <string.h>

#include "cmsis_os2.h"
#include "gpio.h"
#include "usart.h"
int rx_gps_flag=0;
char message[700];
void Start_gps_locate_tast(void *argument) {
    HAL_GPIO_WritePin(GPS_PWR_GPIO_Port, GPS_PWR_Pin, GPIO_PIN_RESET);
    HAL_UARTEx_ReceiveToIdle_DMA(&GPS_UART,message,700);
    while (1) {
        if (rx_gps_flag==1) {

            rx_gps_flag=0;
            HAL_UART_Transmit(&test_uart,message,strlen(message),1000);
            memset(message,0,sizeof(message));
            HAL_UARTEx_ReceiveToIdle_DMA(&GPS_UART,message,700);
        }
    }
}
