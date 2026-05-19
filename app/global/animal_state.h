//
// Created by wangc on 2026/4/25.
//

#ifndef ALL_ASSEMBLE_ANIMAL_STATE_H
#define ALL_ASSEMBLE_ANIMAL_STATE_H

#include <gprmc/gprmc.h>

typedef struct {
    int gps_data_valid;      //gps数据状态（1：有效；0：无效）
    float temperature;       // 温度（单位：摄氏度）
    double latitude;        // 纬度 (度)
    double longitude;       // 经度 (度)
    double speed;           // 地面速度 (km/h)
    char beijing_time[12];
    char beijing_date[12];
    DateTime beijing_date_time;
    int star_state;//卫星模块状态(1:正常，0：异常)
} animalState;

extern animalState animal_state;


#endif //ALL_ASSEMBLE_ANIMAL_STATE_H