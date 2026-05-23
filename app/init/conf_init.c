#include "init.h"

#define INIT_ADDR 0x0000
#define GAP_HOUR_ADDR 0x0001
#define GAP_MINUTE_ADDR 0x0002
#define STEP_INIT_ADDR 0x0003
#define NET_CONFIG_FLAG_ADDR  0x0010   // 标记 IP/端口是否已配置（are_init / non_init）
#define IP_ADDR_BASE          0x0011   // IP 地址（4字节，大端存储）
#define PORT_ADDR             0x0015   // 端口号（2字节，大端存储）

// 为了清晰，可以定义 IP 各字节的偏移
#define IP_ADDR_0  (IP_ADDR_BASE + 0)
#define IP_ADDR_1  (IP_ADDR_BASE + 1)
#define IP_ADDR_2  (IP_ADDR_BASE + 2)
#define IP_ADDR_3  (IP_ADDR_BASE + 3)

typedef enum
{
    non_init = 0x00,
    are_init = 0x01,

} gap_init_byte_type_def;

uint8_t g_server_ip[4] = {0};
uint16_t g_server_port = 0;


uint8_t read_config_from_eeprom(uint8_t *hour, uint8_t *minute);
static uint8_t parse_hour_minute(char *msg, uint8_t *hour, uint8_t *minute);
int gap_init(void);
int network_init(void);
int gap_de_init() {
    M24C64_ON();
    M24C64_abble_write();
    M24C64_WriteByte(INIT_ADDR,non_init);
    M24C64_disable_write();
    M24C64_OFF();
}
int conf_init() {

    M24C64_ON();
    HAL_Delay(100);
    //gap_de_init();
    gap_init();
    uint8_t data;
    network_init();
    M24C64_OFF();
}


int gap_init(void) {
    uint8_t init_state;
    M24C64_ReadByte(INIT_ADDR, &init_state);
    uint8_t hour = 0, minute = 0;
    char rx_message[50]="",tx_message[50]="";
    if (init_state == are_init) {
        read_config_from_eeprom(&hour, &minute);
        HAL_UART_Transmit(&test_uart,"are init\r\n",10,HAL_MAX_DELAY);
        sprintf(tx_message,"transmin_gap_set:%dH %dM\r\n",hour,minute);
        HAL_UART_Transmit(&test_uart,tx_message,strlen(tx_message),HAL_MAX_DELAY);
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
    M24C64_disable_write();
    HAL_Delay(100);

    return HAL_OK;
}

uint8_t read_config_from_eeprom(uint8_t *hour, uint8_t *minute) {
    if (M24C64_ReadByte(GAP_HOUR_ADDR, hour) != HAL_OK) return HAL_ERROR;
    if (M24C64_ReadByte(GAP_MINUTE_ADDR, minute) != HAL_OK) return HAL_ERROR;
    return HAL_OK;
}

/**
 * @brief  网络参数初始化（IP 与端口）
 * @note   若未配置，通过 test_uart 交互输入；已配置则直接跳过
 */
int network_init(void) {
    uint8_t config_flag;
    M24C64_ReadByte(NET_CONFIG_FLAG_ADDR, &config_flag);

    if (config_flag == are_init) {
        // 已配置过，直接返回
        read_network_config_from_eeprom(g_server_ip, &g_server_port);
        char message[50]="";
        snprintf(message, sizeof(message), "IP:%d.%d.%d.%d Port:%d\r\n",
             g_server_ip[0], g_server_ip[1], g_server_ip[2], g_server_ip[3], g_server_port);
        HAL_UART_Transmit(&test_uart, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
        HAL_UART_Transmit(&test_uart, "network already config\r\n", 23, HAL_MAX_DELAY);
        return HAL_OK;
    }

    uint8_t ip[4] = {0};
    uint16_t port = 0;
    char rx_message[80] = "";
    char tx_message[80] = "";
    uint16_t len;

    // 循环直到用户输入合法的 IP 和端口
    while (1) {
        HAL_UART_Transmit(&test_uart, "Enter IP and port (e.g., 192.168.1.100 8080):\r\n",
                          49, HAL_MAX_DELAY);

        HAL_UARTEx_ReceiveToIdle(&test_uart, (uint8_t*)rx_message, sizeof(rx_message), &len, HAL_MAX_DELAY);

        // 解析：格式 "xxx.xxx.xxx.xxx xxxxx" (IP 空格 端口)
        unsigned int a, b, c, d, p;
        if (sscanf(rx_message, "%u.%u.%u.%u %u", &a, &b, &c, &d, &p) == 5) {
            if (a <= 255 && b <= 255 && c <= 255 && d <= 255 && p <= 65535) {
                ip[0] = (uint8_t)a;
                ip[1] = (uint8_t)b;
                ip[2] = (uint8_t)c;
                ip[3] = (uint8_t)d;
                port = (uint16_t)p;
                break;  // 输入合法，退出循环
            }
        }
        HAL_UART_Transmit(&test_uart, "Invalid format, please retry.\r\n", 30, HAL_MAX_DELAY);
    }

    // 写入 EEPROM
    M24C64_abble_write();
    HAL_Delay(100);
    M24C64_WriteByte(IP_ADDR_0, ip[0]);
    M24C64_WriteByte(IP_ADDR_1, ip[1]);
    M24C64_WriteByte(IP_ADDR_2, ip[2]);
    M24C64_WriteByte(IP_ADDR_3, ip[3]);
    M24C64_WriteByte(PORT_ADDR,     (uint8_t)(port >> 8));   // 高字节
    M24C64_WriteByte(PORT_ADDR + 1, (uint8_t)(port & 0xFF)); // 低字节
    M24C64_WriteByte(NET_CONFIG_FLAG_ADDR, are_init);        // 标记已配置
    M24C64_disable_write();

    // 回显确认
    snprintf(tx_message, sizeof(tx_message), "Set IP: %d.%d.%d.%d Port: %d\r\n",
             ip[0], ip[1], ip[2], ip[3], port);
    HAL_UART_Transmit(&test_uart, (uint8_t*)tx_message, strlen(tx_message), HAL_MAX_DELAY);

    return HAL_OK;
}

/**
 * @brief  从 EEPROM 读取网络配置
 * @param  ip   : 输出，4字节 IP 数组（需外部提供 uint8_t[4]）
 * @param  port : 输出，端口号
 * @return HAL_OK / HAL_ERROR
 */
int read_network_config_from_eeprom(uint8_t *ip, uint16_t *port) {
    if (M24C64_ReadByte(IP_ADDR_0, &ip[0]) != HAL_OK) return HAL_ERROR;
    if (M24C64_ReadByte(IP_ADDR_1, &ip[1]) != HAL_OK) return HAL_ERROR;
    if (M24C64_ReadByte(IP_ADDR_2, &ip[2]) != HAL_OK) return HAL_ERROR;
    if (M24C64_ReadByte(IP_ADDR_3, &ip[3]) != HAL_OK) return HAL_ERROR;

    uint8_t port_high, port_low;
    if (M24C64_ReadByte(PORT_ADDR, &port_high) != HAL_OK) return HAL_ERROR;
    if (M24C64_ReadByte(PORT_ADDR + 1, &port_low) != HAL_OK) return HAL_ERROR;
    *port = ((uint16_t)port_high << 8) | port_low;
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