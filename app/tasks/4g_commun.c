#include "gpio.h"
#include "main.h"
#include "usart.h"
#include <string.h>
#include "4G_AT/Fourg_AT.h"
int rx_4g_flag=0;
uint16_t len_test, len_4g;
void Start_4g_commun() {
    while (1) {
        uint8_t* msg;
        osMessageQueueGet(info_trans_4gHandle,&msg,0,osWaitForever);
        FourG_SendData(0,msg,strlen(msg));
        FourG_PowerOffAndWait();
        //FourG_SendData(0,"hello",5);
        vPortFree(msg);
    }
}