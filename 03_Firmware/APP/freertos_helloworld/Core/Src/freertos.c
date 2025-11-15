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
#include <stdio.h>

#include "queue.h"

#include "bsp_led_driver.h"
#include "bsp_led_handler.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define HANLDER_1_DEBUG           1
// uint test of led operations
led_status_t led_on_myown (void)
{
    HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
    printf("LED ON\r\n");
    return LED_OK;
}
led_status_t led_off_myown (void)
{
    HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
    printf("LED OFF\r\n");
    return LED_OK;
}
led_operations_t led_operations_myown = {
    .pf_led_on  = led_on_myown,
    .pf_led_off = led_off_myown,
};

led_status_t pf_get_time_ms_myown ( uint32_t * const time_stamp)
{
    printf("get time now [%lu]ms\r\n", HAL_GetTick());
    *time_stamp = HAL_GetTick();
    return LED_OK;
}
time_base_ms_t time_base_ms_myown = {
    .pf_get_time_ms = pf_get_time_ms_myown,
};

led_status_t pf_os_delay_ms_myown ( const uint32_t delay )
{
    // for ( uint32_t i = 0; i < 1000 * delay; i++ )
    // {

    // }
    osDelay(delay);
    printf("delay [%d]ms finished\r\n", delay);
    return LED_OK;
}
os_delay_t os_delay_myown = {
    .pf_os_delay_ms = pf_os_delay_ms_myown,
};

// Self test :: driver-layer-test
void Test1()
{
    printf("Hello World!\r\n");
    bsp_led_driver_t led1;
    led_driver_inst(&led1, 
                    &led_operations_myown, 
                    &os_delay_myown, 
                    &time_base_ms_myown);
    led1.pf_led_controller(&led1, 1000, 10, PROPORTION_1_1);
    printf("Hello World2!\r\n");
}

led_status_t os_delay_ms_handler_1 ( const uint32_t delay_time )
{
    osDelay(delay_time);
#if HANLDER_1_DEBUG
    printf("delay [%d]ms finished finished\r\n", delay_time);
#endif // HANLDER_1_DEBUG
    return LED_OK;
}
os_delay_t handler_1_os_delay = {
    .pf_os_delay_ms = os_delay_ms_handler_1,
};

led_handler_status_t os_queue_create_handler_1 (
                                                   uint32_t const     item_num,
                                                   uint32_t const    item_size,
                                                   void **  const queue_handler
                                               )
{
#if HANLDER_1_DEBUG
    printf("os_queue_create_handler_1 kick off\r\n");
#endif // HANLDER_1_DEBUG
    QueueHandle_t temp_queue_handler = NULL;

    temp_queue_handler = xQueueCreate(item_num, item_size);
    if (NULL == temp_queue_handler)
    {
        return HANDLER_ERRORRESOURCE;
    }
    else
    {
        *queue_handler = (void *)temp_queue_handler;
        return HANDLER_OK;
    }
}

led_handler_status_t os_queue_put_handler_1 (
                                                void *  const queue_handler,
                                                void *  const          item,
                                                uint32_t             timeout
                                            )
{
#if HANLDER_1_DEBUG
    printf("os_queue_put_handler_1 kick off\r\n");
#endif // HANLDER_1_DEBUG
    led_handler_status_t ret = HANDLER_OK;
    if (
        NULL == queue_handler ||
        NULL == item          ||
        timeout > portMAX_DELAY
       )
    {
        return HANDLER_ERRORPARAMETER;
    }
    else
    {
        if ( pdTRUE != xQueueSend((QueueHandle_t)queue_handler, 
                                                         item, 
                                                     timeout) )
        {
            ret = HANDLER_ERROR;
            return ret;
        }
    }
    return ret;
}

led_handler_status_t os_queue_get_handler_1 (
                                                void *  const queue_handler,
                                                void *  const           msg,
                                                uint32_t             timeout
                                             )
{
#if HANLDER_1_DEBUG
    printf("os_queue_get_handler_1 kick off\r\n");
#endif // HANLDER_1_DEBUG
    led_handler_status_t ret = HANDLER_OK;
    if (
        NULL == queue_handler ||
        NULL == msg          ||
        timeout > portMAX_DELAY
       )
    {
        return HANDLER_ERRORPARAMETER;
    }
    else
    {
        if ( pdTRUE != xQueueReceive((QueueHandle_t)queue_handler, 
                                                              msg, 
                                                        timeout) )
        {
            ret = HANDLER_ERROR;
            return ret;
        }
    }    
    return ret;
}

led_handler_status_t os_queue_delete_hanler_1 (void *  const  queue_handler)
{
#if HANLDER_1_DEBUG
    printf("os_queue_delete_hanler_1 kick off\r\n");
#endif // HANLDER_1_DEBUG   
    if (NULL == queue_handler)
    {
        return HANDLER_ERRORPARAMETER;
    }

    vQueueDelete((QueueHandle_t)queue_handler);
    return HANDLER_OK;
}

handler_os_queue_t handler1_os_queue = {
    .pf_os_queue_create = os_queue_create_handler_1,
    .pf_os_queue_put    = os_queue_put_handler_1,
    .pf_os_queue_get    = os_queue_get_handler_1,
    .pf_os_queue_delete = os_queue_delete_hanler_1,
};

led_handler_status_t os_critical_enter_handler_1 ( void )
{
    vPortEnterCritical();
    return HANDLER_OK;
}
led_handler_status_t os_critical_exit_handler_1 ( void )
{
    vPortExitCritical();
    return HANDLER_OK;
}
handler_os_critical_t handler1_os_critical = {
    .pf_os_critical_enter = os_critical_enter_handler_1,
    .pf_os_critical_exit  = os_critical_exit_handler_1,
};

led_handler_status_t get_time_ms_handler_1 ( uint32_t * const os_tick)
{
    if (NULL == os_tick)
    {
#if HANLDER_1_DEBUG
        printf("get_time_ms_handler_1 input parameter error\r\n");
#endif // HANLDER_1_DEBUG
        return HANDLER_ERRORPARAMETER;
    }
    *os_tick = HAL_GetTick();
    return HANDLER_OK;
}
handler_time_base_ms_t time_base_ms_handler_1 = {
    .pf_get_time_ms = get_time_ms_handler_1,
};

// Self test :: driver-layer-test
void Test2()
{
    printf("Hello World!\r\n");
//******************************** Handler **********************************//
    led_handler_status_t ret = HANDLER_OK;
    bsp_led_handler_t handler_1;
    ret = led_handler_inst(
                           &handler_1,
                           &handler_1_os_delay,
                           &handler1_os_queue,
                           &handler1_os_critical,
                           &time_base_ms_handler_1
                          );
    
//******************************** Driver ***********************************//
    bsp_led_driver_t led1;
    led_driver_inst(&led1, 
                    &led_operations_myown, 
                    &os_delay_myown, 
                    &time_base_ms_myown);

    bsp_led_driver_t led2;
    led_driver_inst(&led2, 
                    &led_operations_myown, 
                    &os_delay_myown, 
                    &time_base_ms_myown);
    led1.pf_led_controller(&led1, 5, 30, PROPORTION_1_1);
    led2.pf_led_controller(&led2, 2, 10, PROPORTION_1_3);

//*************************** Intergrated Test ******************************//
    led_index_t handler_1_led_index = LED_NOT_INITIALIZED;
    ret = handler_1.pf_led_register(
                                &handler_1,
                                &led1,
                                &handler_1_led_index
                                  );
    printf("The return of handler_1.pf_led_register is [%d]\r\n", ret);
    printf("LED registered with index [%lu]\r\n", handler_1_led_index + 1);

    ret = handler_1.pf_led_register(
                                &handler_1,
                                &led2,
                                &handler_1_led_index
                                  );
    printf("The return of handler_1.pf_led_register is [%d]\r\n", ret);
    printf("LED registered with index [%lu]\r\n", handler_1_led_index + 1);


    printf("Hello World2!\r\n");
}

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
    .priority = (osPriority_t)osPriorityNormal,
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
void MX_FREERTOS_Init(void)
{
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
    defaultTaskHandle   = osThreadNew(StartDefaultTask,
                                      NULL,
                                      &defaultTask_attributes);

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
    // Self test
    Test2();

    /* Infinite loop */
    for (;;)
    {

        osDelay(100);
    }
    /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
