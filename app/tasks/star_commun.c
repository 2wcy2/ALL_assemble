#include <string.h>
#include "cmsis_os2.h"
#include "usart.h"
#include "main.h"
#include "STAR_AT/atsend.h"
int rx_tianqi_flag=0;

void Start_star_communication_task(void *argument) {
    while (1) {
        int rt=ATwake();
        osDelay(1000);
    }
}
