#include "m24c64.h"

void M24C64_ON() {
    HAL_GPIO_WritePin(EEPROM_PWR_GPIO_Port, EEPROM_PWR_Pin, GPIO_PIN_SET);
}
void M24C64_OFF() {
    HAL_GPIO_WritePin(EEPROM_PWR_GPIO_Port, EEPROM_PWR_Pin, GPIO_PIN_RESET);
}
void M24C64_abble_write() {
    HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin,GPIO_PIN_RESET);
}
void M24C64_disable_write() {
    HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin,GPIO_PIN_SET);
}

/**
 * @brief  等待 EEPROM 内部写完成（ACK 轮询）
 * @retval HAL_OK: 就绪, HAL_BUSY: 超时
 */
HAL_StatusTypeDef M24C64_WaitReady(void)
{
    uint32_t tickstart = HAL_GetTick();
    while (HAL_I2C_IsDeviceReady(&M24C64_I2C, M24C64_ADDR_W, 1, 5) != HAL_OK)
    {
        if ((HAL_GetTick() - tickstart) > M24C64_TIMEOUT)
            return HAL_BUSY;
    }
    return HAL_OK;
}

/**
 * @brief  写一个字节
 * @param  addr: 写入地址 (0x0000~0x1FFF)
 * @param  data: 数据
 * @retval HAL_OK / HAL_ERROR / HAL_BUSY
 */
uint8_t M24C64_WriteByte(uint16_t addr, uint8_t data)
{
    if (HAL_I2C_Mem_Write(&M24C64_I2C, M24C64_ADDR_W, addr,
                          I2C_MEMADD_SIZE_16BIT, &data, 1, 100) != HAL_OK)
        return HAL_ERROR;
    return M24C64_WaitReady();
}

/**
 * @brief  页写入（最多 32 字节，自动处理跨页）
 * @param  addr: 起始地址
 * @param  pData: 数据指针
 * @param  len: 写入长度
 * @retval HAL_OK / HAL_ERROR / HAL_BUSY
 */
uint8_t M24C64_WritePage(uint16_t addr, uint8_t *pData, uint16_t len)
{
    uint16_t remain = len;
    uint16_t offset = 0;

    while (remain > 0)
    {
        uint16_t space = M24C64_PAGE_SIZE - (addr % M24C64_PAGE_SIZE);
        uint16_t chunk = (remain < space) ? remain : space;

        if (HAL_I2C_Mem_Write(&M24C64_I2C, M24C64_ADDR_W, addr,
                              I2C_MEMADD_SIZE_16BIT, pData + offset, chunk, 200) != HAL_OK)
            return HAL_ERROR;

        if (M24C64_WaitReady() != HAL_OK)
            return HAL_ERROR;

        addr   += chunk;
        offset += chunk;
        remain -= chunk;
    }
    return HAL_OK;
}

/**
 * @brief  随机读一个字节
 */
uint8_t M24C64_ReadByte(uint16_t addr, uint8_t *data)
{
    return HAL_I2C_Mem_Read(&M24C64_I2C, M24C64_ADDR_R, addr,
                            I2C_MEMADD_SIZE_16BIT, data, 1, 100);
}

/**
 * @brief  连续读取多个字节
 */
uint8_t M24C64_ReadSeq(uint16_t addr, uint8_t *pData, uint16_t len)
{
    return HAL_I2C_Mem_Read(&M24C64_I2C, M24C64_ADDR_R, addr,
                            I2C_MEMADD_SIZE_16BIT, pData, len, 200);
}