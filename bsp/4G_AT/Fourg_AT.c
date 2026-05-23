#include"4G_AT/Fourg_AT.h"

/**
 * @brief   将 uint8_t 数组转换为大写十六进制字符串
 * @param   data     原始数据指针
 * @param   len      数据长度（字节）
 * @param   out_str  输出缓冲区，必须 >= len*2+1 字节
 */
static void BytesToHexString(const uint8_t *data, uint16_t len, char *out_str) {
    static const char hex[] = "0123456789ABCDEF";
    for (uint16_t i = 0; i < len; i++) {
        out_str[i * 2]     = hex[(data[i] >> 4) & 0x0F];
        out_str[i * 2 + 1] = hex[data[i] & 0x0F];
    }
    out_str[len * 2] = '\0';
}


/* ========== 基础 AT 命令收发 ========== */
int AT_sendFourg(const char *tx_message, char *rx_buffer, uint16_t buf_size, uint32_t timeout_ms)
{
    if (!rx_buffer || buf_size == 0) return 0;

    memset(rx_buffer, 0, buf_size);
    rx_4g_flag = 0;
    HAL_UART_AbortReceive(&UART_4g);
    HAL_UARTEx_ReceiveToIdle_DMA(&UART_4g, (uint8_t *)rx_buffer, buf_size);

    HAL_UART_Transmit(&UART_4g, (uint8_t *)tx_message, strlen(tx_message), 1000);

    uint32_t elapsed = 0;
    const uint32_t tick = 50;
    while (elapsed < timeout_ms) {
        if (rx_4g_flag == 1) {
            rx_4g_flag = 0;
            HAL_UART_AbortReceive(&UART_4g);
            return 1;
        }
        osDelay(tick);
        elapsed += tick;
    }

    HAL_UART_AbortReceive(&UART_4g);
    return 0;
}

/* ========== 上电（如果已上电则跳过） ========== */
void FourG_PowerOnAndWait(void)
{
    if (HAL_GPIO_ReadPin(PWR_4G_GPIO_Port, PWR_4G_Pin) == GPIO_PIN_SET) {
        return;   // 已经上电，无需重复操作
    }

    HAL_GPIO_WritePin(PWR_4G_GPIO_Port, PWR_4G_Pin, GPIO_PIN_SET);
    osDelay(5000);  // 等待模块启动
}
/* ========== 断电 ========== */
void FourG_PowerOffAndWait(void)
{
    HAL_GPIO_WritePin(PWR_4G_GPIO_Port, PWR_4G_Pin, GPIO_PIN_RESET);
}

/* ========== SIM 卡检查 ========== */
bool FourG_CheckSIMReady(void)
{
    char buf[50];
    if (!AT_sendFourg("AT+CPIN?\r\n", buf, sizeof(buf), 2000))
        return false;
    return (strstr(buf, "READY") != NULL);
}

/* ========== 网络注册状态 ========== */
bool FourG_IsNetworkRegistered(void)
{
    char buf[50];
    if (!AT_sendFourg("AT+CEREG?\r\n", buf, sizeof(buf), 2000))
        return false;

    int n, stat;
    if (sscanf(buf, " +CEREG: %d,%d", &n, &stat) == 2) {
        return (stat == 1 || stat == 5);
    }
    return false;
}

/* ========== 附着 GPRS（网络注册失败时调用）========== */
bool FourG_AttachGPRS(void)
{
    char buf[20];
    if (!AT_sendFourg("AT+CGATT=1\r\n", buf, sizeof(buf), 5000))
        return false;
    return (strstr(buf, "OK") != NULL);
}

/* ========== 定义 APN ========== */
bool FourG_DefineAPN(const char *apn)
{
    char cmd[80];
    snprintf(cmd, sizeof(cmd), "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", apn);
    char buf[20];
    if (!AT_sendFourg(cmd, buf, sizeof(buf), 3000))
        return false;
    return (strstr(buf, "OK") != NULL);
}

/* ========== 激活 PDP 上下文 ========== */
bool FourG_ActivatePDP(uint8_t cid)
{
    char cmd[20];
    snprintf(cmd, sizeof(cmd), "AT+CGACT=1,%d\r\n", cid);
    char buf[20];
    if (!AT_sendFourg(cmd, buf, sizeof(buf), 5000))
        return false;
    return (strstr(buf, "OK") != NULL);
}

/* ========== 获取 IP 地址 ========== */
bool FourG_GetIPAddress(char *ip_out, uint16_t buf_size)
{
    char buf[100];
    if (!AT_sendFourg("AT+CGPADDR=1\r\n", buf, sizeof(buf), 3000))
        return false;

    char *p = strstr(buf, "+CGPADDR:");
    if (!p) return false;
    if (sscanf(p, "+CGPADDR: 1,\"%[^\"]\"", ip_out) == 1) {
        return true;
    }
    return false;
}

/* ========== 信号强度 ========== */
int FourG_GetSignalQuality(void)
{
    char buf[50];
    if (!AT_sendFourg("AT\r\n", buf, sizeof(buf), 2000))
        return -1;
    if (!AT_sendFourg("AT+CSQ\r\n", buf, sizeof(buf), 2000))
        return -1;

    int rssi = 0, ber = 0;
    if (sscanf(buf, " +CSQ: %d,%d", &rssi, &ber) == 2)
        return rssi;
    return -1;
}

/* ========== Ping 测试 ========== */
bool FourG_Ping(const char *host, uint8_t count, uint16_t size)
{
    char cmd[80];
    snprintf(cmd, sizeof(cmd), "AT+MPING=\"%s\",%d,%d\r\n", host, count, size);
    char buf[200];
    if (!AT_sendFourg(cmd, buf, sizeof(buf), 10000))
        return false;
    return (strstr(buf, "+MPING: \"statistics\"") != NULL);
}

/* ========== 打开 TCP Socket ========== */
bool FourG_OpenTCPSocket(uint8_t socket_id, const char *remote_ip, uint16_t remote_port)
{
    char cmd[120];
    snprintf(cmd, sizeof(cmd), "AT+MIPOPEN=%d,\"TCP\",\"%s\",%d\r\n",
             socket_id, remote_ip, remote_port);
    char buf[30];
    if (!AT_sendFourg(cmd, buf, sizeof(buf), 5000))
        return false;
    return (strstr(buf, "OK") != NULL || strstr(buf, "+MIPOPEN: 0,0") != NULL);
}

/* ========== 发送 TCP 数据（含 Ctrl+Z 结束） ========== */
bool FourG_SendData(uint8_t socket_id, const uint8_t *data, uint16_t len)
{
    char cmd[30];
    snprintf(cmd, sizeof(cmd), "AT+MIPSEND=%d,%d\r\n", socket_id, len);
    char buf[20];
    if (!AT_sendFourg(cmd, buf, sizeof(buf), 2000))
        return false;
    // 等待 > 提示后发送数据
    HAL_UART_Transmit(&UART_4g, data, len, 2000);
    uint8_t eof = 0x1A;     // Ctrl+Z
    HAL_UART_Transmit(&UART_4g, &eof, 1, 2000);

    // 等待 +MIPSEND: <sid>,<len> 和 OK
    if (!AT_sendFourg("", buf, sizeof(buf), 5000))  // 仅等待接收
        return false;
    return (strstr(buf, "+MIPSEND:") != NULL);
}

/* ========== 关闭 Socket ========== */
bool FourG_CloseSocket(uint8_t socket_id)
{
    char cmd[20];
    snprintf(cmd, sizeof(cmd), "AT+MIPCLOSE=%d\r\n", socket_id);
    char buf[20];
    if (!AT_sendFourg(cmd, buf, sizeof(buf), 3000))
        return false;
    return (strstr(buf, "OK") != NULL);
}
