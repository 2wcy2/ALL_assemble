
#include "cmsis_os2.h"
#include "main.h"
#include "usart.h"
#include "global/animal_state.h"
#include "commun_manager/commun_manager.h"
#define data_len 25

// 辅助函数：将 uint16_t 转为大端序 2 字节
void uint16ToBytes(uint16_t val, uint8_t *buf) {
    buf[0] = (val >> 8) & 0xFF;
    buf[1] = val & 0xFF;
}
// 将uint32_t转换为字节数组（大端序）
void uint32ToBytes(uint32_t value, uint8_t *buffer) {
    buffer[3] = (uint8_t)(value & 0xFF);           // 低字节
    buffer[2] = (uint8_t)((value >> 8) & 0xFF);
    buffer[1] = (uint8_t)((value >> 16) & 0xFF);
    buffer[0] = (uint8_t)((value >> 24) & 0xFF);   // 高字节
}

void framemaker(uint8_t *outstr, animalState *state) {
    // 清零整个缓冲区，同时保证校验和从 0 开始累加
    memset(outstr, 0, 25);  // 根据实际长度调整

    // 字节 0：协议编号
    outstr[0] = 0x02;

    // 字节 1-6：北京时间 (DateTime 需定义)
    outstr[1] = state->beijing_date_time.year - 2000;   // 年（0~99）
    outstr[2] = state->beijing_date_time.month;
    outstr[3] = state->beijing_date_time.day;
    outstr[4] = state->beijing_date_time.hour;
    outstr[5] = state->beijing_date_time.minute;
    outstr[6] = state->beijing_date_time.second;

    // 字节 7-10：纬度（大端 uint32，单位：1e-6 度，偏移 +90°）
    uint32_t lat = (uint32_t)((state->latitude + 90.0) * 1000000.0);
    uint32ToBytes(lat, &outstr[7]);

    // 字节 11-14：经度（大端 uint32，单位：1e-6 度，偏移 +180°）
    uint32_t lon = (uint32_t)((state->longitude + 180.0) * 1000000.0);
    uint32ToBytes(lon, &outstr[11]);

    // 字节 15：温度（偏移 -100℃，范围 0~200，对应 -100~+100℃）
    outstr[15] = (uint8_t)(state->temperature + 100.0f);

    // 字节 16-19：步数（大端 uint32）
    uint32ToBytes(state->steps, &outstr[16]);

    // 字节 20-21：电池电压（大端 uint16，单位：0.01V，如 3.70V -> 370）
    uint16_t vol = (uint16_t)(state->voltage * 100.0f);
    uint16ToBytes(vol, &outstr[20]);

    // 字节 22：标志位（bit0: GPS有效，bit1: 卫星模块正常）
    uint8_t flags = 0;
    if (state->gps_data_valid) flags |= 0x01;
    if (state->star_state)    flags |= 0x02;
    outstr[22] = flags;

    // 字节 23：地面速度（uint8，单位：km/h，最大 255 km/h，野生动物足够）
    outstr[23] = (uint8_t)(state->speed);  // 注意浮点转整数的截断

    // 字节 24：校验和（前面所有字节之和的低 8 位）
    for (int i = 0; i < 24; i++) {
        outstr[24] += outstr[i];
    }
}


void Start_info_assemble_task(void *argument) {
    while (1) {
        uint32_t flags = osThreadFlagsWait(FLAG_SENSOR_READY | FLAG_GNSS_READY,
                                    osFlagsWaitAll,   // 关键：必须两个标志位都置位
                                    osWaitForever);
        uint8_t* message=pvPortMalloc(sizeof(uint8_t)*data_len);
        memset(message,0,sizeof(uint8_t)*data_len);
        framemaker(message,&animal_state);
        SelectCommMethod();
        osMessageQueuePut(info_transHandle,&message,0,osWaitForever);
        HAL_UART_Transmit(&test_uart, message, data_len, 1000);
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    }
}