#include <string.h>
#include "usart.h"
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "gprmc/gprmc.h"

int rx_gps_flag=0;
char gps_message[800]="";



void Clear_UART_Error(UART_HandleTypeDef *huart) {
    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE | UART_FLAG_FE | UART_FLAG_NE | UART_FLAG_PE);
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_READY;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
    if (huart->hdmarx != NULL) {
        HAL_DMA_Abort(huart->hdmarx);
    }
}


void Start_gps_locate_tast(void *argument) {
    HAL_GPIO_WritePin(GPS_PWR_GPIO_Port, GPS_PWR_Pin, GPIO_PIN_RESET);
    osDelay(2000);
    //HAL_UART_Receive(&GPS_UART,gps_message,400,HAL_MAX_DELAY);
    Clear_UART_Error(&GPS_UART);
    HAL_UARTEx_ReceiveToIdle_DMA(&GPS_UART,gps_message,1000);
    while (1) {
        if (rx_gps_flag==1) {
            GPRMC_DATA rmc_data;
            memset(&rmc_data, 0, sizeof(rmc_data));
            if (parse_gprmc_from_buffer(gps_message, &rmc_data) == 0) {
                // 解析成功
                char msg[128];
                sprintf(msg, "Lon:%.6f Lat:%.6f Speed:%.2f km/h\r\n",
                        rmc_data.longitude, rmc_data.latitude, rmc_data.speed);
                HAL_UART_Transmit(&test_uart, (uint8_t*)msg, strlen(msg), 100);
            }
            rx_gps_flag=0;
            memset(gps_message,0,sizeof(gps_message));
            Clear_UART_Error(&GPS_UART);
            HAL_UARTEx_ReceiveToIdle_DMA(&GPS_UART,gps_message,1000);
        }
        osDelay(100);
    }
}

