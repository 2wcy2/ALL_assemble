//
// Created by wangc on 2026/5/22.
//

#ifndef ALL_ASSEMBLE_COMMUN_MANAGER_H
#define ALL_ASSEMBLE_COMMUN_MANAGER_H

#include <stdint.h>
#include "main.h"
#include "usart.h"
#include <string.h>
typedef enum {
    COMM_4G = 0,
    COMM_SATELLITE
} CommMethod_t;

CommMethod_t SelectCommMethod(void);


#endif //ALL_ASSEMBLE_COMMUN_MANAGER_H