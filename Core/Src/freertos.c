/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for gps_locate_task */
osThreadId_t gps_locate_taskHandle;
const osThreadAttr_t gps_locate_task_attributes = {
  .name = "gps_locate_task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for star_commun */
osThreadId_t star_communHandle;
const osThreadAttr_t star_commun_attributes = {
  .name = "star_commun",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for commun_4g */
osThreadId_t commun_4gHandle;
const osThreadAttr_t commun_4g_attributes = {
  .name = "commun_4g",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for info_assemble_t */
osThreadId_t info_assemble_tHandle;
const osThreadAttr_t info_assemble_t_attributes = {
  .name = "info_assemble_t",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for info_trans */
osMessageQueueId_t info_transHandle;
const osMessageQueueAttr_t info_trans_attributes = {
  .name = "info_trans"
};
/* Definitions for i2c1mutex */
osMutexId_t i2c1mutexHandle;
const osMutexAttr_t i2c1mutex_attributes = {
  .name = "i2c1mutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Start_SensorTask(void *argument);
extern void Start_gps_locate_task(void *argument);
extern void Start_star_communication_task(void *argument);
extern void Start_4g_commun(void *argument);
extern void Start_info_assemble_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of i2c1mutex */
  i2c1mutexHandle = osMutexNew(&i2c1mutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of info_trans */
  info_transHandle = osMessageQueueNew (16, sizeof(uint8_t*), &info_trans_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of SensorTask */
  SensorTaskHandle = osThreadNew(Start_SensorTask, NULL, &SensorTask_attributes);

  /* creation of gps_locate_task */
  gps_locate_taskHandle = osThreadNew(Start_gps_locate_task, NULL, &gps_locate_task_attributes);

  /* creation of star_commun */
  star_communHandle = osThreadNew(Start_star_communication_task, NULL, &star_commun_attributes);

  /* creation of commun_4g */
  commun_4gHandle = osThreadNew(Start_4g_commun, NULL, &commun_4g_attributes);

  /* creation of info_assemble_t */
  info_assemble_tHandle = osThreadNew(Start_info_assemble_task, NULL, &info_assemble_t_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_Start_SensorTask */
/**
  * @brief  Function implementing the SensorTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Start_SensorTask */
__weak void Start_SensorTask(void *argument)
{
  /* USER CODE BEGIN Start_SensorTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_SensorTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

