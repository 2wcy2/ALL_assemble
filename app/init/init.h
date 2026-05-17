//
// Created by wangc on 2026/5/15.
//

#ifndef ALL_ASSEMBLE_TASKS_H
#define ALL_ASSEMBLE_TASKS_H


#include <stdlib.h>
#include <string.h>
#include "usart.h"
#include "main.h"
#include "M24c64/m24c64.h"

int gap_init(void);
int gap_de_init();
uint8_t read_config_from_eeprom(uint8_t *hour, uint8_t *minute);

#endif //ALL_ASSEMBLE_TASKS_H