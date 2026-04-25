#include "cmsis_os2.h"

void Start_store_task(void *argument) {
    while (1) {
        osDelay(100);
    }
}
