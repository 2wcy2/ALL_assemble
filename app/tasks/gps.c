#include "cmsis_os2.h"
int rx_gps_flag=0;
void Start_gps_locate_tast(void *argument) {
    while (1) {
        osDelay(100);
    }
}
