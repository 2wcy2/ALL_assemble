#include "atsend.h"
#define receive_time 1000
#define star_uart &TIANQI_UART
/**
 * @brief   将 uint8_t 数组转换为大写十六进制字符串
 * @param   data     原始数据指针
 * @param   len      数据长度（字节）
 * @param   out_str  输出缓冲区，必须 >= len*2+1 字节
 */
void BytesToHexString(const uint8_t *data, uint16_t len, char *out_str) {
    static const char hex[] = "0123456789ABCDEF";
    for (uint16_t i = 0; i < len; i++) {
        out_str[i * 2]     = hex[(data[i] >> 4) & 0x0F];
        out_str[i * 2 + 1] = hex[data[i] & 0x0F];
    }
    out_str[len * 2] = '\0';
}


/**
 * @brief  检验模块是否在线
 * @return 1:正常 0：异常
 */
int ATwake() {
    char receivestr[10]="";
    HAL_UARTEx_ReceiveToIdle_DMA(star_uart,(uint8_t*)receivestr,10);
    HAL_UART_Transmit(star_uart,(uint8_t*)"AT\r\n",4,HAL_MAX_DELAY);
    while (rx_tianqi_flag==0) {
        osDelay(100);
        HAL_UART_Transmit(star_uart,(uint8_t*)"AT\r\n",4,HAL_MAX_DELAY);
    }
    rx_tianqi_flag=0;
    if (receivestr[2]=='O'&&receivestr[3]=='K') return 1;
    return 0;
}



/**
 * @brief   将 uint8_t 数组转换为ATsend命令后由串口发送,返回1：正常，0：异常
 * @param   data     原始数据指针
 * @param   data_len      数据长度（字节）
 */
void ATsendcmd(const uint8_t *data, uint16_t data_len) {
    char sendstr[50]="";
    strcpy(sendstr,"AT+SEND=");
    sprintf(sendstr,"%s%d,",sendstr,data_len);
    BytesToHexString(data,data_len,&sendstr[strlen(sendstr)]);
    strcat(sendstr,"\r\n");
    HAL_UART_Transmit(star_uart,(uint8_t*)sendstr,strlen(sendstr),HAL_MAX_DELAY);
}


