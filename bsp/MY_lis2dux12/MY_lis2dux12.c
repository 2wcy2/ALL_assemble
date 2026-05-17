#include "MY_lis2dux12/MY_lis2dux12.h"

LIS2DUX12_Object_t obj;

int register_io() {
    LIS2DUX12_IO_t io = {
        .Init      = BSP_I2C1_Init,
        .DeInit    = BSP_I2C1_DeInit,
        .BusType   = LIS2DUX12_I2C_BUS,
        .Address   = 0x32,                   // 根据实际地址修改
        .WriteReg  = BSP_I2C1_WriteReg,
        .ReadReg   = BSP_I2C1_ReadReg,
        .GetTick   = BSP_GetTick,
        .Delay     = HAL_Delay
    };
    LIS2DUX12_RegisterBusIO(&obj, &io);
}

int init_lis2dux12() {

    // ------------------- 唤醒与基础配置 -------------------
    // 1) 退出深掉电（I2C 虚拟读）
    uint8_t id;
    lis2dux12_device_id_get(&obj.Ctx, &id);
    obj.Ctx.mdelay(100);                     // 等待 25ms 完成启动

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


    // 8) 配置计步器
    lis2dux12_stpcnt_mode_t stpcnt = {
        .step_counter_enable  = PROPERTY_ENABLE,
        .false_step_rej       = PROPERTY_ENABLE,   // 需要 MLC
        .step_counter_in_fifo = PROPERTY_DISABLE
    };
    if (lis2dux12_stpcnt_mode_set(&obj.Ctx, stpcnt) != 0) {
        HAL_UART_Transmit(&test_uart, "step init error\r\n", 18, 100);
    }

    // 验证配置
    uint8_t c4, c5, emb_a, emb_b_check;
    lis2dux12_read_reg(&obj.Ctx, LIS2DUX12_CTRL4, &c4, 1);
    lis2dux12_read_reg(&obj.Ctx, LIS2DUX12_CTRL5, &c5, 1);
    lis2dux12_mem_bank_set(&obj.Ctx, LIS2DUX12_EMBED_FUNC_MEM_BANK);
    lis2dux12_read_reg(&obj.Ctx, LIS2DUX12_EMB_FUNC_EN_A, &emb_a, 1);
    lis2dux12_read_reg(&obj.Ctx, LIS2DUX12_EMB_FUNC_EN_B, &emb_b_check, 1);
    lis2dux12_mem_bank_set(&obj.Ctx, LIS2DUX12_MAIN_MEM_BANK);
    char msg[64];
    sprintf(msg, "CTRL4=0x%02X, CTRL5=0x%02X, EN_A=0x%02X, EN_B=0x%02X\r\n",
            c4, c5, emb_a, emb_b_check);
    HAL_UART_Transmit(&test_uart, (uint8_t*)msg, strlen(msg), 100);
}

int read_step(uint16_t* step) {
    return lis2dux12_stpcnt_steps_get(&obj.Ctx, step);
}

int clear_step(){
    lis2dux12_stpcnt_rst_step_set(&obj.Ctx);
}
//已初始化返回1
int is_lis2dux12_initialized(void)
{
    lis2dux12_stpcnt_mode_t mode;
    int32_t ret;

    // 调用底层读取函数，获取当前计步器配置
    ret = lis2dux12_stpcnt_mode_get(&obj.Ctx, &mode);
    if (ret != 0) {
        // I2C 通信失败，认为未初始化
        return 0;
    }

    // 检查计步器使能位是否为 1
    if (mode.step_counter_enable == PROPERTY_ENABLE) {
        return 1;
    } else {
        return 0;
    }
}