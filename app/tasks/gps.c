#include <string.h>
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gprmc/gprmc.h"
#include "usart.h"
#include "global/animal_state.h"

int rx_gps_flag=0;
char gps_message[1500]="";
TaskHandle_t xGpsTaskHandle = NULL;


void Clear_UART_Error(UART_HandleTypeDef *huart) {
    // 1. 清除所有错误标志
    __HAL_UART_CLEAR_OREFLAG(&GPS_UART);
    __HAL_UART_CLEAR_FEFLAG(&GPS_UART);
    __HAL_UART_CLEAR_NEFLAG(&GPS_UART);
    __HAL_UART_CLEAR_PEFLAG(&GPS_UART);

    // 2. 如果有 FIFO，先禁用再清空（不是所有 MCU 都有）
#if defined(USART_CR1_FIFOEN)
    GPS_UART.Instance->CR1 &= ~USART_CR1_FIFOEN;
#endif

    // 3. 反复读 DR 直到 RXNE 不再有效，确保移位寄存器和 DR 被清空
    volatile uint8_t dummy;
    while (__HAL_UART_GET_FLAG(&GPS_UART, UART_FLAG_RXNE)) {
        dummy = (uint8_t)(GPS_UART.Instance->RDR);
    }
}


void Start_gps_locate_task(void *argument) {
    xGpsTaskHandle = xTaskGetCurrentTaskHandle();
    Clear_UART_Error(&GPS_UART);
    HAL_UARTEx_ReceiveToIdle_DMA(&GPS_UART,gps_message,1500);
    uint32_t error = HAL_UART_GetError(&GPS_UART);
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        rx_gps_flag=0;
        GPRMC_DATA rmc_data;
        memset(&rmc_data, 0, sizeof(rmc_data));
        if (parse_gprmc_from_buffer(gps_message, &rmc_data) == 0) {
            if (rmc_data.valid==1) {
                strcpy(animal_state.beijing_date,rmc_data.beijing_date);
                strcpy(animal_state.beijing_time,rmc_data.beijing_time);
                animal_state.latitude = rmc_data.latitude;
                animal_state.longitude = rmc_data.longitude;
                animal_state.speed = rmc_data.speed;
                animal_state.beijing_date_time = rmc_data.beijing_date_time;
                animal_state.gps_data_valid = rmc_data.valid;
                animal_state.gps_location_valid = rmc_data.status;
                osThreadFlagsSet(info_assemble_tHandle, FLAG_GNSS_READY);
            }
            // 解析成功
            // char msg[128];
            // sprintf(msg, "Lon:%.6f Lat:%.6f Speed:%.2f km/h\r\n",
            //         rmc_data.longitude, rmc_data.latitude, rmc_data.speed);
            // HAL_UART_Transmit(&test_uart, (uint8_t*)msg, strlen(msg), 100);
        }
        memset(gps_message,0,sizeof(gps_message));
        Clear_UART_Error(&GPS_UART);
        HAL_UARTEx_ReceiveToIdle_DMA(&GPS_UART,gps_message,1500);
    }
}

