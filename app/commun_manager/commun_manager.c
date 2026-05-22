#include "commun_manager/commun_manager.h"


//0:未收到 1：收到
int AT_sendFourg(const char* tx_message,char*rx_message) {
    for (int i=0;i<5&&rx_4g_flag==0;i++) {
        HAL_UARTEx_ReceiveToIdle_DMA(&UART_4g,rx_message,100);
        HAL_UART_Transmit(&UART_4g,tx_message,strlen(tx_message),1000);
        osDelay(200);
    }
    if (rx_4g_flag==0) {
        return 0;
    }else {
        rx_4g_flag=0;
        return 1;
    }
}


CommMethod_t SelectCommMethod(void)
{
    char message[100]="";
    HAL_GPIO_WritePin(PWR_4G_GPIO_Port,PWR_4G_Pin,GPIO_PIN_SET);
    osDelay(5000);
    if (AT_sendFourg("AT\r\n",message)==0) {
        return COMM_SATELLITE;
    }
}
