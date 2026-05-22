//
// Created by wangc on 2026/2/12.
//

#ifndef LOCATION_TOSTAR_ATSEND_H
#define LOCATION_TOSTAR_ATSEND_H
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "usart.h"


void ATsendcmd(const uint8_t *data, uint16_t data_len);
int ATwake();

#endif //LOCATION_TOSTAR_ATSEND_H