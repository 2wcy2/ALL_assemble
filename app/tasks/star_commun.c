#include <string.h>
#include "cmsis_os2.h"
#include "usart.h"
#include "main.h"
char message1[50], message2[50];
int rx_varible_flag=0,rxcplt_flag=0;
int rx_tianqi_flag=0;

void Start_star_communication_task(void *argument) {
    // char trans_data[]="AT+CCLR\r\n";
    // HAL_UARTEx_ReceiveToIdle_DMA(&TIANQI_UART,message1,50);
    // rx_varible_flag=1;
    // osDelay(200);
    //  HAL_UART_Transmit(&TIANQI_UART,trans_data,strlen(trans_data),HAL_MAX_DELAY);
    while (1) {
        // if (rxcplt_flag==2) {
        //     HAL_UART_Transmit(&test_uart,message2,strlen(message2),HAL_MAX_DELAY);
        //     rxcplt_flag=0;
        // }
        // else if (rxcplt_flag==1) {
        //     HAL_UART_Transmit(&test_uart,message1,strlen(message1),HAL_MAX_DELAY);
        //     rxcplt_flag=0;
        // }
        osDelay(100);
    }
}
