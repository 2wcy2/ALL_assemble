#include "cmsis_os2.h"
#include "lis2dux12.h"
#include "lis2dux12_reg.h"
#include "custom_bus.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

void my_delay(uint32_t Delay) {
    osDelay(Delay);
}
void StartT_steps_cal_task(void *argument)
{
    // 1. 初始化 I2C 总线（获得互斥锁后执行）
    osMutexAcquire(i2c1mutexHandle, osWaitForever);

    LIS2DUX12_IO_t io = {
        .Init      = BSP_I2C1_Init,
        .DeInit    = BSP_I2C1_DeInit,
        .BusType   = LIS2DUX12_I2C_BUS,
        .Address   = 0x32,                   // 根据实际地址修改
        .WriteReg  = BSP_I2C1_WriteReg,
        .ReadReg   = BSP_I2C1_ReadReg,
        .GetTick   = BSP_GetTick,
        .Delay     = my_delay
    };

    LIS2DUX12_Object_t obj;
    LIS2DUX12_RegisterBusIO(&obj, &io);

    // ------------------- 唤醒与基础配置 -------------------
    // 1) 退出深掉电（I2C 虚拟读）
    uint8_t id;
    lis2dux12_device_id_get(&obj.Ctx, &id);
    obj.Ctx.mdelay(25);                     // 等待 25ms 完成启动

    // 2) 确保在主内存页
    lis2dux12_mem_bank_set(&obj.Ctx, LIS2DUX12_MAIN_MEM_BANK);

    // 3) 基本寄存器：地址自增 + BDU，先关闭嵌入式功能
    lis2dux12_ctrl1_t ctrl1 = {0};
    lis2dux12_ctrl4_t ctrl4 = {0};
    ctrl1.if_add_inc = PROPERTY_ENABLE;
    ctrl4.bdu = PROPERTY_ENABLE;
    ctrl4.emb_func_en = PROPERTY_DISABLE;
    lis2dux12_write_reg(&obj.Ctx, LIS2DUX12_CTRL1, (uint8_t*)&ctrl1, 1);
    lis2dux12_write_reg(&obj.Ctx, LIS2DUX12_CTRL4, (uint8_t*)&ctrl4, 1);

    // 4) 进入 Power-down 模式，然后选择低功耗模式（HP_EN = 0）
    lis2dux12_md_t pd = { .odr = LIS2DUX12_OFF };
    lis2dux12_mode_set(&obj.Ctx, &pd);
    lis2dux12_ctrl3_t ctrl3;
    lis2dux12_read_reg(&obj.Ctx, LIS2DUX12_CTRL3, (uint8_t*)&ctrl3, 1);
    ctrl3.hp_en = 0;                       // 0 = Low‑Power, 1 = High‑Performance
    lis2dux12_write_reg(&obj.Ctx, LIS2DUX12_CTRL3, (uint8_t*)&ctrl3, 1);

    // 5) 设置 ODR = 25Hz 低功耗模式，FS = ±2g，带宽 ODR/2
    lis2dux12_md_t mode = {
        .odr = LIS2DUX12_25Hz_LP,
        .fs  = LIS2DUX12_2g,
        .bw  = LIS2DUX12_ODR_div_2
    };
    if (lis2dux12_mode_set(&obj.Ctx, &mode) != 0) {
        // 设置失败，可打印错误（根据你的环境处理）
        while(1);
    }

    // 6) 使能嵌入式功能
    ctrl4.emb_func_en = PROPERTY_ENABLE;
    lis2dux12_write_reg(&obj.Ctx, LIS2DUX12_CTRL4, (uint8_t*)&ctrl4, 1);

    // 7) 使能 MLC（用于误判抑制）
    lis2dux12_mlc_set(&obj.Ctx, LIS2DUX12_MLC_ON);

    // 8) 配置计步器（步数计数 + 误判抑制，不存入 FIFO）
    lis2dux12_stpcnt_mode_t stpcnt = {
        .step_counter_enable  = PROPERTY_ENABLE,
        .false_step_rej       = PROPERTY_ENABLE,
        .step_counter_in_fifo = PROPERTY_DISABLE
    };
    if (lis2dux12_stpcnt_mode_set(&obj.Ctx, stpcnt) != 0) {
        while(1);
    }

    // ------------------- 验证配置（可选，可删除） -------------------
    uint8_t c4, c5, emb_a, emb_b;
    lis2dux12_read_reg(&obj.Ctx, LIS2DUX12_CTRL4, &c4, 1);
    lis2dux12_read_reg(&obj.Ctx, LIS2DUX12_CTRL5, &c5, 1);
    lis2dux12_mem_bank_set(&obj.Ctx, LIS2DUX12_EMBED_FUNC_MEM_BANK);
    lis2dux12_read_reg(&obj.Ctx, LIS2DUX12_EMB_FUNC_EN_A, &emb_a, 1);
    lis2dux12_read_reg(&obj.Ctx, LIS2DUX12_EMB_FUNC_EN_B, &emb_b, 1);
    lis2dux12_mem_bank_set(&obj.Ctx, LIS2DUX12_MAIN_MEM_BANK);
    // 期望值: CTRL4=0x32, CTRL5=0x60, EN_A=0x08或0x88, EN_B=0x10

    osMutexRelease(i2c1mutexHandle);
    // 等待传感器稳定（计步器算法需要几个周期）
    osDelay(2000);

    // ------------------- 主循环 -------------------
    char msg[64];
    uint16_t last_steps = 0;
    uint16_t steps;

    while (1) {
        osMutexAcquire(i2c1mutexHandle, osWaitForever);

        // 读取步数
        if (lis2dux12_stpcnt_steps_get(&obj.Ctx, &steps) == 0) {
            if (1) {
                sprintf(msg, "Steps: %u\r\n", steps);
                HAL_UART_Transmit(&test_uart, (uint8_t*)msg, strlen(msg), 100);
                last_steps = steps;
            }
        } else {
            HAL_UART_Transmit(&test_uart, (uint8_t*)"Read steps error\r\n", 18, 100);
        }

        osMutexRelease(i2c1mutexHandle);
        osDelay(500);   // 每 500ms 读取一次
    }
}