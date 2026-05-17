#include "init.h"

#define INIT_ADDR 0x0000
#define GAP_HOUR_ADDR 0x0001
#define GAP_MINUTE_ADDR 0x0002
#define STEP_INIT_ADDR 0x0003

typedef enum
{
    non_init = 0x00,
    are_init = 0x01,

} gap_init_byte_type_def;
uint8_t read_config_from_eeprom(uint8_t *hour, uint8_t *minute);
static uint8_t parse_hour_minute(char *msg, uint8_t *hour, uint8_t *minute);

int gap_de_init() {
    M24C64_ON();
    M24C64_abble_write();
    M24C64_WriteByte(INIT_ADDR,non_init);
    M24C64_disable_write();
    M24C64_OFF();
}

int gap_init(void) {
    uint8_t init_state;
    char rx_message[50]="",tx_message[50]="";
    uint8_t hour = 0, minute = 0;
    M24C64_ON();
    M24C64_ReadByte((uint16_t)INIT_ADDR,&init_state);
    if (init_state == are_init) {
        read_config_from_eeprom(&hour, &minute);
        HAL_UART_Transmit(&test_uart,"are init\r\n",10,HAL_MAX_DELAY);
        sprintf(tx_message,"transmin_gap_set:%dH %dM\r\n",hour,minute);
        HAL_UART_Transmit(&test_uart,tx_message,strlen(tx_message),HAL_MAX_DELAY);
        M24C64_OFF();
        return HAL_OK;
    }

    uint16_t len;
    while(!parse_hour_minute((char*)rx_message, &hour, &minute)) {
        HAL_UART_Transmit(&test_uart,"transmit_gap: xH yM\r\n",21,HAL_MAX_DELAY);
        HAL_UARTEx_ReceiveToIdle(&test_uart,rx_message,50,&len,HAL_MAX_DELAY);
    }
    sprintf(tx_message,"transmin_gap_set:%dH %dM\r\n",hour,minute);

    HAL_UART_Transmit(&test_uart,tx_message,strlen(tx_message),HAL_MAX_DELAY);
    M24C64_abble_write();
    M24C64_WriteByte(GAP_HOUR_ADDR, hour);
    M24C64_WriteByte(GAP_MINUTE_ADDR, minute);
    M24C64_WriteByte(INIT_ADDR, are_init);
    M24C64_WriteByte(INIT_ADDR,are_init);
    HAL_Delay(100);
    M24C64_disable_write();
    M24C64_OFF();
    return 1;
}

uint8_t read_config_from_eeprom(uint8_t *hour, uint8_t *minute) {
    if (M24C64_ReadByte(GAP_HOUR_ADDR, hour) != HAL_OK) return HAL_ERROR;
    if (M24C64_ReadByte(GAP_MINUTE_ADDR, minute) != HAL_OK) return HAL_ERROR;
    return HAL_OK;
}

// 辅助函数：从字符串中解析 "xH yM" 格式，例如 "2H 30M"
static uint8_t parse_hour_minute(char *msg, uint8_t *hour, uint8_t *minute) {
    if (strlen(msg)<4) return 0;
    // 期望格式：数字 + 'H' 或 'h'，空格（可选），数字 + 'M' 或 'm'
    int h = 0, m = 0;
    char *p = msg;

    // 跳过前导空格
    while (*p == ' ') p++;

    // 解析小时数字
    if (!(*p >= '0' && *p <= '9')) return 0;
    h = strtoul(p, &p, 10);

    // 期望 'H' 或 'h'
    while (*p == ' ') p++;
    if (*p != 'H' && *p != 'h') return 0;
    p++;

    // 跳过空格
    while (*p == ' ') p++;

    // 解析分钟数字
    if (!(*p >= '0' && *p <= '9')) return 0;
    m = strtoul(p, &p, 10);

    // 期望 'M' 或 'm'
    while (*p == ' ') p++;
    if (*p != 'M' && *p != 'm') return 0;

    // 数值范围检查
    if (h < 0 || h > 23) return 0;   // 小时范围 0~23
    if (m < 0 || m > 59) return 0;   // 分钟范围 0~59

    *hour = (uint8_t)h;
    *minute = (uint8_t)m;
    return 1;
}