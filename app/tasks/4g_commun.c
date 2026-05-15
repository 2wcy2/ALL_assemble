#include "gpio.h"
#include "main.h"
#include "usart.h"
#include <string.h>
int rx_4g_flag=0;
char message_rx_4g[1000]="";
char message_trans_4g[200]="";
uint16_t len_test, len_4g;
void Start_4g_commun() {
    while (1) {
        osDelay(100);
    }
}