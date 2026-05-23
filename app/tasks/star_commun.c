#include "FreeRTOS.h"
#include <string.h>
#include "cmsis_os2.h"
#include "usart.h"
#include "main.h"
#include "../../bsp/STAR_AT/atsend.h"
#include "global/animal_state.h"
int rx_tianqi_flag=0;

void Start_star_communication_task(void *argument) {
    while (1) {
        uint8_t* satmessage;
        osMessageQueueGet(info_trans_sateHandle,&satmessage,0,osWaitForever);
        int rt=ATwake();
        if (ATwake()==1) {
            animal_state.star_state=1;
        }
        ATsendcmd(satmessage,Sat_Data_len);
        vPortFree(satmessage);
    }
}
