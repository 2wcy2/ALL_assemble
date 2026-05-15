#ifndef __M24C64_H
#define __M24C64_H

#include "custom_bus.h"
#include "gpio.h"

/* ----------------- 硬件相关配置（按实际修改） ----------------- */
#define M24C64_I2C       hi2c1               // 使用的 I2C 句柄
#define M24C64_ADDR_W    (0xA0)              // 8位写地址（全接地）
#define M24C64_ADDR_R    (0xA1)              // 8位读地址（全接地）
#define M24C64_TIMEOUT   10                  // 超时时间 (ms)
#define M24C64_PAGE_SIZE 32                  // 页大小

/* ----------------- 函数声明 ----------------- */
void M24C64_ON();
void M24C64_OFF();
void M24C64_abble_write();
void M24C64_disable_write();
HAL_StatusTypeDef M24C64_WaitReady(void);
uint8_t M24C64_WriteByte(uint16_t addr, uint8_t data);
uint8_t M24C64_WritePage(uint16_t addr, uint8_t *pData, uint16_t len);
uint8_t M24C64_ReadByte(uint16_t addr, uint8_t *data);
uint8_t M24C64_ReadSeq(uint16_t addr, uint8_t *pData, uint16_t len);

#endif