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
// #include "bsp_key.h"
#include "bsp_key_irq.h"
#include "bsp_led.h"
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
  led_TaskHandle    = osThreadNew(led_task_func, NULL, &led_Task_attributes);

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
    //********************************APP TASK*********************************//
    /**                     Variables (in task stack)                       **/
    key_press_status_t key_press_event =                       KEY_NOT_PRESSED;
    led_operation_t      led_operation =                               LED_OFF;

    /**                     Variables (in task stack)                       **/

    osDelay(500);

#if 0 // Test #1 error fetch
    key_press_event_t key_press_event =                 
    {
        .edge_type = FALLING,
        .trigger_tick = HAL_GetTick()
    };
    if (pdTRUE == xQueueSendToFront(       inter_key_queue, 
                                          &key_press_event, 
                                        ( TickType_t ) 0 ))
    {
        printf("key_result send short press         at" 
                "[%d] tick\r\n",
                HAL_GetTick());
    }
#endif

#if 0 // Test #2 filting fetch and short press
    key_press_event_t key_press_event =                 
    {
        .edge_type = FALLING,
        .trigger_tick = HAL_GetTick()
    };

    if (NULL == inter_key_queue)
    {
        printf("inter_key_queue is not created!     at"
               "[%d] tick \r\n", 
                HAL_GetTick());
    }
    if (pdTRUE == xQueueSendToBack(        inter_key_queue, 
                                          &key_press_event, 
                                        ( TickType_t ) 0 ))
    {
        printf("key_press_event send to inter       at"
                "[%d] tick\r\n",
                HAL_GetTick());
    }

    HAL_Delay(10);

    key_press_event_t key_press_event_2 =                 
    {
        .edge_type = RISING,
        .trigger_tick = HAL_GetTick()
    };
    if (NULL == inter_key_queue)
    {
        printf("inter_key_queue is not created!     at"
                                       "[%d] tick \r\n", 
                                         HAL_GetTick());
    }
    if (pdTRUE == xQueueSendToBack(        inter_key_queue, 
                                        &key_press_event_2, 
                                        ( TickType_t ) 0 ))
    {
        printf("key_press_event send to inter       at" 
                                        "[%d] tick\r\n",
                                         HAL_GetTick());
    }
#endif

#if 0 // Test #3 filting fetch and long press
    key_press_event_t key_press_event =                 
    {
        .edge_type = FALLING,
        .trigger_tick = HAL_GetTick()
    };

    if (NULL == inter_key_queue)
    {
        printf("inter_key_queue is not created!     at"
               "[%d] tick \r\n", 
                HAL_GetTick());
    }
    if (pdTRUE == xQueueSendToBack(        inter_key_queue, 
                                          &key_press_event, 
                                        ( TickType_t ) 0 ))
    {
        printf("key_press_event send to inter       at"
                "[%d] tick\r\n",
                HAL_GetTick());
    }

    HAL_Delay(600);

    key_press_event_t key_press_event_2 =                 
    {
        .edge_type = RISING,
        .trigger_tick = HAL_GetTick()
    };
    if (NULL == inter_key_queue)
    {
        printf("inter_key_queue is not created!     at"
                                       "[%d] tick \r\n", 
                                         HAL_GetTick());
    }
    if (pdTRUE == xQueueSendToBack(        inter_key_queue, 
                                        &key_press_event_2, 
                                        ( TickType_t ) 0 ))
    {
        printf("key_press_event send to inter       at" 
                                        "[%d] tick\r\n",
                                         HAL_GetTick());
    }
#endif

    /* Infinite loop */
    for (;;)
    {
        printf("APP task is living\r\n");

        // Check if the queue was created successfully
        if ( NULL == key_queue )
        {
            printf("key_queue is not created!               at"
                                               "[%d] tick \r\n", 
                                                HAL_GetTick());
        }

        // 1. If the key is pressed with short time
        if ( pdTRUE == xQueueReceive(                         key_queue, 
                                                   &( key_press_event ), 
                                                    ( TickType_t ) 0 ) )
        {
            if ( KEY_SHORT_PRESSED == key_press_event )
            {
                // 1.1 toggle the LED
                led_operation = LED_BLINK_1_TIMES;
                if ( pdTRUE == xQueueSendToBack(              led_queue, 
                                                         &led_operation, 
                                                    ( TickType_t ) 0 ) )
                {
                    printf("led_operation send to led_queue     at"
                                                    "[%d] tick\r\n",
                                                    HAL_GetTick());
                }
            }
            // 2. If the key is pressed with long time
            if ( KEY_LONG_PRESSED  == key_press_event )
            {
                // 2.1 led blink with 3 times
                led_operation =  LED_BLINK_10_TIMES;
                if ( pdTRUE == xQueueSendToBack(              led_queue, 
                                                         &led_operation, 
                                                    ( TickType_t ) 0 ) )
                {
                    printf("led_operation send to led_queue     at"
                                                    "[%d] tick\r\n",
                                                    HAL_GetTick());
                }
            }
        }    
    
        osDelay(200);
    }

  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
