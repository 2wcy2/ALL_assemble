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
int conf_init();
int gap_de_init();
uint8_t read_config_from_eeprom(uint8_t *hour, uint8_t *minute);
int read_network_config_from_eeprom(uint8_t *ip, uint16_t *port);
extern uint8_t g_server_ip[4];
extern uint16_t g_server_port;

#endif //ALL_ASSEMBLE_TASKS_H