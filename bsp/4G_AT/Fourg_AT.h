//
// Created by wangc on 2026/5/23.
//

#ifndef ALL_ASSEMBLE_FOURG_AT_H
#define ALL_ASSEMBLE_FOURG_AT_H

#include <string.h>
#include "main.h"
#include "usart.h"
#include "FreeRTOS.h"
#include <stdbool.h>
/* ----- 基础 AT 命令 ----- */
int AT_sendFourg(const char *tx_message, char *rx_buffer, uint16_t buf_size, uint32_t timeout_ms);

/* ----- 模块上断电 ----- */
void FourG_PowerOnAndWait(void);
void FourG_PowerOffAndWait(void);
/* ----- SIM 卡与网络注册 ----- */
bool FourG_CheckSIMReady(void);
bool FourG_IsNetworkRegistered(void);
bool FourG_AttachGPRS(void);

/* ----- PDP 激活与 IP 获取 ----- */
bool FourG_DefineAPN(const char *apn);
bool FourG_ActivatePDP(uint8_t cid);
bool FourG_GetIPAddress(char *ip_out, uint16_t buf_size);

/* ----- 信号强度查询 ----- */
int FourG_GetSignalQuality(void);   // 返回 RSSI，-1 表示失败

/* ----- 可选：Ping ----- */
bool FourG_Ping(const char *host, uint8_t count, uint16_t size);

/* ----- UDP Socket 操作 ----- */
bool FourG_OpenTCPSocket(uint8_t socket_id, const char *remote_ip, uint16_t remote_port);
bool FourG_SendData(uint8_t socket_id, const uint8_t *data, uint16_t len);
bool FourG_CloseSocket(uint8_t socket_id);




#endif //ALL_ASSEMBLE_FOURG_AT_H