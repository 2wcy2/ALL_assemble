#include <gprmc/gprmc.h>
#include <string.h>
#include <stdlib.h>

#include "global/animal_state.h"

// 校验和计算
static int calculate_checksum(const char *data) {
    int checksum = 0;
    // 跳过'$'字符
    if (*data == '$') data++;

    while (*data && *data != '*') {
        checksum ^= *data;
        data++;
    }
    return checksum & 0xFF;
}

// 将度分格式转换为十进制度
static double nmea_to_degrees(const char *nmea_coord, char direction) {
    double degrees = 0.0;
    double minutes = 0.0;
    char temp[16] = {0};

    // 复制字符串进行处理
    strncpy(temp, nmea_coord, sizeof(temp) - 1);

    // 找到小数点的位置
    char *dot_pos = strchr(temp, '.');
    if (!dot_pos) return 0.0;

    // 度部分至少2位（经度3位，纬度2位）
    int degree_digits = (strlen(nmea_coord) > 10) ? 3 : 2;

    // 提取度和分
    char degree_str[4] = {0};
    strncpy(degree_str, temp, degree_digits);
    degrees = atof(degree_str);

    minutes = atof(temp + degree_digits);

    // 转换为度
    double result = degrees + minutes / 60.0;

    // 处理方向
    if (direction == 'S' || direction == 'W') {
        result = -result;
    }

    return result;
}

// 判断是否为闰年
static int is_leap_year(int year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

// 获取月份的天数
static int days_in_month(int year, int month) {
    static const int month_days[12] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};

    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return month_days[month - 1];
}

// 解析UTC时间字符串为DateTime结构
static int parse_utc_time(const char *time_str, DateTime *dt) {
    if (strlen(time_str) < 6) return -1;

    char hour_str[3] = {0};
    char minute_str[3] = {0};
    char second_str[3] = {0};

    strncpy(hour_str, time_str, 2);
    strncpy(minute_str, time_str + 2, 2);
    strncpy(second_str, time_str + 4, 2);

    dt->hour = atoi(hour_str);
    dt->minute = atoi(minute_str);
    dt->second = atoi(second_str);

    // 解析微秒部分
    char *dot = strchr(time_str, '.');
    if (dot) {
        // 处理小数秒，最多6位
        char micro_str[7] = {0};
        strncpy(micro_str, dot + 1, 6);
        int len = strlen(micro_str);

        // 转换为微秒（小数部分，例如0.12秒 -> 120000微秒）
        dt->microsecond = atoi(micro_str);
        for (int i = len; i < 6; i++) {
            dt->microsecond *= 10;
        }
    } else {
        dt->microsecond = 0;
    }

    return 0;
}

// 解析UTC日期字符串为DateTime结构
static int parse_utc_date(const char *date_str, DateTime *dt) {
    if (strlen(date_str) != 6) return -1;

    char day_str[3] = {0};
    char month_str[3] = {0};
    char year_str[3] = {0};

    strncpy(day_str, date_str, 2);
    strncpy(month_str, date_str + 2, 2);
    strncpy(year_str, date_str + 4, 2);

    dt->day = atoi(day_str);
    dt->month = atoi(month_str);
    dt->year = 2000 + atoi(year_str);  // 假设是2000年之后的年份

    // 处理2000年之前的日期
    if (dt->year > 2050) {
        dt->year -= 100;
    }

    return 0;
}

// UTC时间转为北京时间（UTC+8）
static void utc_to_beijing(DateTime *utc_dt, DateTime *beijing_dt) {
    *beijing_dt = *utc_dt;

    // 加上8小时
    beijing_dt->hour += 8;

    // 处理小时溢出
    if (beijing_dt->hour >= 24) {
        beijing_dt->hour -= 24;
        beijing_dt->day += 1;

        // 处理日期溢出
        if (beijing_dt->day > days_in_month(beijing_dt->year, beijing_dt->month)) {
            beijing_dt->day = 1;
            beijing_dt->month += 1;

            // 处理月份溢出
            if (beijing_dt->month > 12) {
                beijing_dt->month = 1;
                beijing_dt->year += 1;
            }
        }
    }
}

// 格式化时间为字符串
static void format_time_string(const DateTime *dt, char *time_str, int include_micro) {
    if (include_micro && dt->microsecond > 0) {
        // 计算小数部分
        double seconds = dt->second + dt->microsecond / 1000000.0;
        snprintf(time_str, 15, "%02d:%02d:%06.3f",
                dt->hour, dt->minute, seconds);
    } else {
        snprintf(time_str, 12, "%02d:%02d:%02d",
                dt->hour, dt->minute, dt->second);
    }
}

// 格式化日期为字符串
static void format_date_string(const DateTime *dt, char *date_str) {
    snprintf(date_str, 12, "%04d/%02d/%02d",
            dt->year, dt->month, dt->day);
}

// 使用手动分割替代strtok，保留空字段
static int split_gprmc_fields(char *buffer, char *tokens[], int max_tokens) {
    int token_count = 0;
    char *start = buffer;
    char *end;

    // 处理第一个字段（可能包含$）
    if (*start == '$') {
        start++;
    }

    while (token_count < max_tokens) {
        // 找到下一个逗号或字符串结束
        end = strchr(start, ',');
        if (end) {
            // 保存当前位置作为字段的结束
            *end = '\0';
            tokens[token_count++] = start;
            start = end + 1;  // 移动到下一个字段的开始
        } else {
            // 最后一个字段
            tokens[token_count++] = start;
            break;
        }
    }

    return token_count;
}


// 主解析函数
int parse_gprmc(const char *nmea_str, GPRMC_DATA *data) {
    if (!nmea_str || !data) {
        return -1;
    }

    // 验证基本格式
    if (strlen(nmea_str) < 10 || nmea_str[0] != '$') {
        return -2;
    }

    // 检查是否为GPRMC语句
    if (strncmp(nmea_str, "$GPRMC", 6) != 0 &&
        strncmp(nmea_str, "$GNRMC", 6) != 0) {
        return -3;
    }

    // 查找校验和分隔符
    char *checksum_start = strchr(nmea_str, '*');
    if (!checksum_start) {
        return -4;
    }

    // 验证校验和
    int expected_checksum = strtol(checksum_start + 1, NULL, 16);
    int actual_checksum = calculate_checksum(nmea_str);

    if (expected_checksum != actual_checksum) {
        data->valid = 0;
        // 可以选择继续解析或返回错误
        // 这里继续解析，因为校验和错误可能只是传输问题
    }

    data->checksum = expected_checksum;

    // 复制字符串用于分割（避免修改原字符串）
    char buffer[120] = {0};
    // 复制到*之前的所有字符（包含字段部分，不包含校验和）
    size_t copy_len = checksum_start - nmea_str;
    if (copy_len >= sizeof(buffer)) {
        copy_len = sizeof(buffer) - 1;
    }
    strncpy(buffer, nmea_str, copy_len);
    buffer[copy_len] = '\0';

    // 分割字段 - 使用新的分割函数
    char *tokens[13];
    int token_count = 0;

    token_count = split_gprmc_fields(buffer, tokens, 20);

    DateTime utc_dt = {0};
    DateTime beijing_dt = {0};

    // 解析各个字段
    // 注意：现在索引是固定的，空字段也会占用索引位置

    // 1. 状态（索引2）
    if (token_count > 2 && strlen(tokens[2]) > 0) {
        data->status = tokens[2][0];
        data->valid = (data->status == 'A') ? 1 : 0;
    }

    // 2. 时间（索引1）
    if (token_count > 1 && strlen(tokens[1]) > 0) {
        if (parse_utc_time(tokens[1], &utc_dt) == 0) {
            format_time_string(&utc_dt, data->time, 1);
        }
    }

    // 3. 纬度（索引3）和纬度半球（索引4）
    if (token_count > 4 && strlen(tokens[3]) > 0 && strlen(tokens[4]) > 0) {
        data->lat_direction = tokens[4][0];
        data->latitude = nmea_to_degrees(tokens[3], data->lat_direction);
    }

    // 4. 经度（索引5）和经度半球（索引6）
    if (token_count > 6 && strlen(tokens[5]) > 0 && strlen(tokens[6]) > 0) {
        data->lon_direction = tokens[6][0];
        data->longitude = nmea_to_degrees(tokens[5], data->lon_direction);
    }
    //5.速度
    if (token_count > 7 && strlen(tokens[7]) > 0) {
        float temp = atof(tokens[7]);
        temp*=1.852;
        data->speed = temp;
    }

    // 6. 日期（索引9）
    if (token_count > 9 && strlen(tokens[9]) > 0) {
        if (parse_utc_date(tokens[9], &utc_dt) == 0) {
            format_date_string(&utc_dt, data->date);
        }
    }

    // 7. 将UTC转换为北京时间
    if (utc_dt.year > 0) {
        utc_to_beijing(&utc_dt, &beijing_dt);
        format_time_string(&beijing_dt, data->beijing_time, 1);
        format_date_string(&beijing_dt, data->beijing_date);
        data->beijing_date_time = beijing_dt;//供组包使用
    }

    return 0;
}