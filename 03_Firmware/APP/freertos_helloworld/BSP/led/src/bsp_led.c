/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file bsp_led.c
 *
 * @par dependencies
 * - bsp_led.h
 * - stdio.h
 * - stdint.h
 *
 * @author Ethan-Hang
 *
 * @brief Provide the HAL APIs of led and corresponding opetions.
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2025-10-08
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#include "bsp_led.h"

//******************************** Defines **********************************//

//*********************Thread_Func **********************//
osThreadId_t led_TaskHandle;
const osThreadAttr_t led_Task_attributes = {
    .name = "led_Task",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
QueueHandle_t led_queue;

//*********************Queue_Handler ********************//
/**
 * @brief Instantiates the bsp_led_handler_t target.
 *
 * Steps:
 *  1. Adds Core interfaces into bsp_led_driver instance target.
 *
 * @param[in] void
 *
 * @return led_handler_status_t : Status of the function.
 *
 * */
led_status_t led_on_off(led_operation_t led_operation)
{
    led_status_t led_status = LED_OK;
    if (LED_ON == led_operation)
    {
        // 1. Make the LED ON.
        HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
    }
    if (LED_OFF == led_operation)
    {
        // 1. Make the LED OFF.
        HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
    }
    if (LED_TOGGLE == led_operation)
    {
        // 1. Make the LED TOGGLE.
        HAL_GPIO_TogglePin(led_GPIO_Port, led_Pin);
    }
    return led_status;
}

void led_task_func(void *argument)
{
    /* USER CODE BEGIN led_task_func */
    led_status_t    led_ret   = LED_OK;
    led_operation_t led_value = LED_ON;
    led_queue = xQueueCreate(10, sizeof(led_operation_t));

    if (NULL == led_queue)
    {
        printf("led Queue Create Failed!\r\n");
    }
    else
    {
        printf("led Queue Create Success!\r\n");
    }

    for (;;)
    {
        printf("Hello_led_Thread\r\n");

        if (led_queue != NULL)
        {
            if (pdTRUE == xQueueReceive(led_queue, &led_value, 10))
            {
                printf("Received led_queue value: [%lu]\r\n", led_value);
                led_on_off(led_value);
            }
        }
        osDelay(1);
    }
}
/* USER CODE END led_task_func */

//******************************** Defines **********************************//
