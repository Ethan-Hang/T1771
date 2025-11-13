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

#include "bsp_led_driver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// uint test of led operations
led_status_t led_on_myown (void)
{
    printf("LED ON\r\n");
    return LED_OK;
}
led_status_t led_off_myown (void)
{
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
    printf("Hello World!\r\n");
    bsp_led_driver_t led1;
    led_driver_inst(&led1, 
                    &led_operations_myown, 
                    &os_delay_myown, 
                    &time_base_ms_myown);
    led1.pf_led_controller(&led1, 10, 10, PROPORTION_1_1);
    printf("Hello World2!\r\n");

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
