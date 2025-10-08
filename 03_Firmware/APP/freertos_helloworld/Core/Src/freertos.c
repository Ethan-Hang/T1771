/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "bsp_key.h"
#include "queue.h"
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
//*********************Thread_Func **********************//
osThreadId_t key_TaskHandle;
const osThreadAttr_t key_Task_attributes = {
  .name = "key_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
QueueHandle_t key_queue;

//*********************Queue_Handler ********************//

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void key_task_func(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

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
  /* USER CODE BEGIN RTOS_THREADS */
  key_TaskHandle    = osThreadNew(key_task_func, NULL, &key_Task_attributes);

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
  uint32_t received_value = 0;
  /* Infinite loop */
  for(;;)
  {
    // printf("StartDefaultTask\r\n");
    if (key_queue != NULL)
    {
      if ( xQueueReceive( key_queue, &received_value, 10 ) == pdPASS )
      {
        printf("Key Tick Received: %lu\r\n", received_value);
      }
    }
    osDelay(1);
  }
  
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void key_task_func(void *argument)
{
    /* USER CODE BEGIN key_task_func */
    key_status_t       key_ret   =          KEY_OK;
    key_press_status_t key_value = KEY_NOT_PRESSED;

    uint32_t counter_tick = 0;
    key_queue = xQueueCreate ( 10, sizeof(uint32_t) );
    if (NULL == key_queue)
    {
        printf("Key Queue Create Failed!\r\n");
    }
    else
    {
        printf("Key Queue Create Success!\r\n");
    }

    for (;;)
    {
        counter_tick++;

        key_ret = key_scan(&key_value);

        if ( KEY_OK == key_ret )
        {
            if ( KEY_PRESSED == key_value )
            {
                printf("Key Pressed!\r\n");
                if (pdTRUE == xQueueSend(key_queue, &counter_tick, 0))
                {
                    printf("Key Tick Send Success!\r\n");
                }
            }
        }
        if ( KEY_OK != key_ret )
        {
            printf("Key Scan Error: Error code:[%d]\r\n", key_ret);
        }
        osDelay(100);
    }
    /* USER CODE END key_task_func */
}

/* USER CODE END Application */

