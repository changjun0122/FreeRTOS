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
#include <stdio.h>
#include <string.h>
#include "usart.h"
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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LogTask */
osThreadId_t LogTaskHandle;
const osThreadAttr_t LogTask_attributes = {
  .name = "LogTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for AlarmTask */
osThreadId_t AlarmTaskHandle;
const osThreadAttr_t AlarmTask_attributes = {
  .name = "AlarmTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for SerialMutex */
osMutexId_t SerialMutexHandle;
const osMutexAttr_t SerialMutex_attributes = {
  .name = "SerialMutex"
};
/* Definitions for SerialSem */
osSemaphoreId_t SerialSemHandle;
const osSemaphoreAttr_t SerialSem_attributes = {
  .name = "SerialSem"
};
/* Definitions for DangerSem */
osSemaphoreId_t DangerSemHandle;
const osSemaphoreAttr_t DangerSem_attributes = {
  .name = "DangerSem"
};
/* Definitions for TestSem */
osSemaphoreId_t TestSemHandle;
const osSemaphoreAttr_t TestSem_attributes = {
  .name = "TestSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartLogTask(void *argument);
void StartAlarmTask(void *argument);

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
  /* creation of SerialMutex */
  SerialMutexHandle = osMutexNew(&SerialMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of SerialSem */
  SerialSemHandle = osSemaphoreNew(1, 1, &SerialSem_attributes);

  /* creation of DangerSem */
  DangerSemHandle = osSemaphoreNew(1, 0, &DangerSem_attributes);

  /* creation of TestSem */
  TestSemHandle = osSemaphoreNew(1, 0, &TestSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of LogTask */
  LogTaskHandle = osThreadNew(StartLogTask, NULL, &LogTask_attributes);

  /* creation of AlarmTask */
  AlarmTaskHandle = osThreadNew(StartAlarmTask, NULL, &AlarmTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osSemaphoreAcquire(TestSemHandle, osWaitForever);   /* 平时阻塞，按键后唤醒 */
    HAL_Delay(3000);                                      /* 忙等 3 秒，抢占 CPU */
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartLogTask */
/**
* @brief Function implementing the LogTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLogTask */
void StartLogTask(void *argument)
{
  /* USER CODE BEGIN StartLogTask */
  int counter = 0;
  char msg[30];
  /* Infinite loop */
  for(;;)
  {
    osSemaphoreAcquire(SerialSemHandle, osWaitForever);
    // osMutexAcquire(SerialMutexHandle, osWaitForever);

    counter++;
    sprintf(msg, "日志：%d", counter);
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    osDelay(500);
    osSemaphoreRelease(SerialSemHandle);
    // osMutexRelease(SerialMutexHandle);
  }
  /* USER CODE END StartLogTask */
}

/* USER CODE BEGIN Header_StartAlarmTask */
/**
* @brief Function implementing the AlarmTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartAlarmTask */
void StartAlarmTask(void *argument)
{
  /* USER CODE BEGIN StartAlarmTask */
  char msg[] = "🚨 警报 警报";
  /* Infinite loop */
  for(;;)
  {
    osSemaphoreAcquire(DangerSemHandle, osWaitForever);

    osSemaphoreAcquire(SerialSemHandle, osWaitForever);
    // osMutexAcquire(SerialMutexHandle, osWaitForever);

    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY); 

    osSemaphoreRelease(SerialSemHandle);
    // osMutexRelease(SerialMutexHandle);
  }
  /* USER CODE END StartAlarmTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

