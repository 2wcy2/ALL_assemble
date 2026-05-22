#include "adc.h"
#include"main.h"
#include "global/animal_state.h"

TaskHandle_t xBatteryTaskHandle = NULL;
void HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef *hadc) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hadc->Instance == ADC1) {  // 确保是电池电压对应的ADC
        // 发送任务通知，直接让等待该通知的任务解除阻塞
        vTaskNotifyGiveFromISR(xBatteryTaskHandle, &xHigherPriorityTaskWoken);
        // 如果需要立即切换任务，则请求调度
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void Start_battery_voltage_task(void *argument) {
    xBatteryTaskHandle = xTaskGetCurrentTaskHandle();
    HAL_ADC_Start_IT(&hadc1);
    while (1) {
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (ulNotificationValue > 0) {
            // 读取ADC转换结果
            uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
            // 转换为电压值（假设参考电压3.3V，12位ADC）
            float voltage = (adc_value * 3.3f) / 4096.0f;
            HAL_ADC_Start_IT(&hadc1);
            animal_state.voltage = 2.0*voltage;
            osDelay(5000);
            // 在此处理电池电压逻辑...
        }

    }
}
