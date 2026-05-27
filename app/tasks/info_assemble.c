
#include "cmsis_os2.h"
#include "main.h"
#include "rtc.h"
#include "usart.h"
#include "global/animal_state.h"
#include "commun_manager/commun_manager.h"


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
/**
 * @brief  生成 4G 文本格式数据包（动态分配内存）
 * @param  state : 动物状态结构体指针
 * @return 成功返回动态分配的字符串指针，失败返回 NULL
 *         调用者使用完毕后需 vPortFree() 释放
 */
char* Fourg_TextFrameMaker(const animalState *state)
{
    if (state == NULL) return NULL;

    // 预估最大长度：
    // 时间 19 字节 + 经纬度各 12 字节 + 温度 5 + 步数 10 + 电压 5 + 速度 5 + 标志 2 + 逗号 8 个 + 结束符 = 约 80 字节
    // 为安全起见，分配 128 字节
    char *buf = (char *)pvPortMalloc(Fourg_Data_len);
    memset(buf, 0, Fourg_Data_len);
    if (buf == NULL) return NULL;
    // 2. 从 RTC 获取当前北京时间
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);   // 先读时间
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);   // 再读日期
    // 格式化数据：
    // 格式：年-月-日 时:分:秒,纬度,经度,温度,步数,电压,速度,GPS标志,卫星标志
    int written = snprintf(buf, Fourg_Data_len,
        "%04u-%02u-%02u %02u:%02u:%02u,%.6f,%.6f,%.1f,%lu,%.2f,%.1f,%d,%c,%d",
        (unsigned int)(sDate.Year + 2000),   // 从 RTC 读取，偏移量+2000 恢复公元年
        sDate.Month,
        sDate.Date,
        sTime.Hours,
        sTime.Minutes,
        sTime.Seconds,
        state->latitude,
        state->longitude,
        state->temperature,
        (unsigned long)state->steps,
        state->voltage,
        state->speed,
        state->gps_data_valid,
        state->gps_location_valid,
        state->star_state
    );

    // 如果 snprintf 返回的长度超过分配大小（理论上不会，但做保护）
    if (written < 0 || written >= (int)Fourg_Data_len) {
        // 截断或返回错误，这里简单返回 NULL
        vPortFree(buf);
        return NULL;
    }

    return buf;
}
void framemaker(uint8_t *outstr, animalState *state) {
    // 清零整个缓冲区，同时保证校验和从 0 开始累加
    memset(outstr, 0, 25);  // 根据实际长度调整

    // 字节 0：协议编号
    outstr[0] = 0x02;
    // 2. 从 RTC 获取当前北京时间
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);   // 先读时间
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);   // 再读日期
    // 4. 字节 1-6：北京时间（直接取自 RTC）
    outstr[1] = sDate.Year;       // HAL 库 Year 已为偏移量（0~99），无需再减 2000
    outstr[2] = sDate.Month;
    outstr[3] = sDate.Date;
    outstr[4] = sTime.Hours;
    outstr[5] = sTime.Minutes;
    outstr[6] = sTime.Seconds;

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
        osThreadFlagsWait(FLAG_SENSOR_READY /*| FLAG_GNSS_READY*/,
                                    osFlagsWaitAll,   // 关键：必须两个标志位都置位
                                    osWaitForever);
        /* 选择通信通道（内部会尝试初始化 4G，失败时自动断电并返回卫星） */
        CommMethod_t method = SelectCommMethod();

        /* 投递到对应队列，队列满会阻塞直到有空间（osWaitForever） */
        if (method == COMM_4G) {
            FourG_SyncRTC();
            char *text_msg = Fourg_TextFrameMaker(&animal_state);
            if (osMessageQueuePut(info_trans_4gHandle, &text_msg, 0, 2000)!=osOK) {
                vPortFree(text_msg);
            }
        }
        else {
            /* 分配消息内存 */
            uint8_t *message = pvPortMalloc(sizeof(uint8_t) * Sat_Data_len);
            if (message == NULL) {
                // 分配失败，可根据需要记录错误，然后继续等待下一次触发
                continue;
            }
            memset(message, 0, sizeof(uint8_t) * Sat_Data_len);

            /* 组装数据帧 */
            framemaker(message, &animal_state);
            if (osMessageQueuePut(info_trans_sateHandle, &message, 0, 2000)!=osOK) {
                vPortFree(message);
            }
        }
    }
}