#include "cmsis_os2.h"
#include "main.h"
int rx_test_flag = 0;

void Start_dubug_task(void *argument) {
    osDelay(1000);

    while (1) {

        osDelay(1000);
    }
}
