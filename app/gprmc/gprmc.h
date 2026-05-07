//
// Created by wangc on 2026/2/3.
//

#ifndef GPSTEST_GPRMC_H
#define GPSTEST_GPRMC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int microsecond;  // 微秒部分
} DateTime;

typedef struct {
    char time[12];           // UTC时间 (HHMMSS.SSS)
    char status;            // 状态: A=有效, V=无效
    double latitude;        // 纬度 (度)
    char lat_direction;     // 纬度方向: N/S
    double longitude;       // 经度 (度)
    char lon_direction;     // 经度方向: E/W
    double speed;           // 地面速度 (km/h)
    char date[12];          // UTC日期 (DDMMYY)
    int checksum;           // 校验和
    int valid;              // 数据有效标志: 1=有效, 0=无效
    char beijing_time[12];
    char beijing_date[12];
    DateTime beijing_date_time;
} GPRMC_DATA;




int parse_gprmc(const char *nmea_str, GPRMC_DATA *data);


#endif //GPSTEST_GPRMC_H