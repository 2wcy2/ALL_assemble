

#ifndef ALL_ASSEMBLE_LIS2DUX12_H
#define ALL_ASSEMBLE_LIS2DUX12_H

#include "lis2dux12.h"
#include "lis2dux12_reg.h"
#include "custom_bus.h"
#include <stdio.h>
#include "usart.h"

int init_lis2dux12();
int read_step(uint16_t* step);
int clear_step();
int is_lis2dux12_initialized(void);
int register_io();

#endif //ALL_ASSEMBLE_LIS2DUX12_H