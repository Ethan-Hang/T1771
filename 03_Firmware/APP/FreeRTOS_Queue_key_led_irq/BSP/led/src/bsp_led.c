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
 * @brief LED HAL APIs and operations.
 *
 * Processing flow: call directly.
 *
 * @version V1.0 2025-10-08
 *
 * @note 1 tab == 4 spaces!
 *
 ******************************************************************************/

#include "bsp_led.h"

//*************************** Defines *******************************//

//********************* Thread_Func ***********************//
/* LED task handle */
osThreadId_t led_TaskHandle;

/* LED task attributes */
const osThreadAttr_t led_Task_attributes = {
    .name = "led_Task",                          /* Task name */
    .stack_size = 128 * 4,                       /* Stack: 512 bytes */
    .priority = (osPriority_t) osPriorityNormal, /* Priority: Normal */
};
//******************** Thread_Func **********************//

//******************** Queue_Handler ********************//
/* LED message queue handle */
QueueHandle_t led_queue;

//********************* Queue_Handler *********************//
/**
 * @brief LED control function
 * 
 * @param[in] led_operation : LED operation type
 *                            - LED_ON: Turn on LED
 *                            - LED_OFF: Turn off LED
 *                            - LED_TOGGLE: Toggle LED state
 * 
 * @return led_status_t : Operation status (LED_OK on success)
 * 
 * @note LED uses active-low logic
 */
led_status_t led_on_off(led_operation_t led_operation)
{
    led_status_t led_status = LED_OK;
    
    if ( LED_ON            == led_operation )
    {
        /* Turn on LED */
        HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
    }
    if ( LED_OFF           == led_operation )
    {
        /* Turn off LED */
        HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
    }
    if ( LED_TOGGLE        == led_operation )
    {
        /* Toggle LED state */
        HAL_GPIO_TogglePin(led_GPIO_Port, led_Pin);
    }
    if ( LED_BLINK_3_TIMES == led_operation )
    {
        // 1. Make the LED blink 3 times
        for (int i = 0; i < 3; i++)
        {
            HAL_GPIO_TogglePin(led_GPIO_Port, led_Pin); // Toggle LED state
            osDelay(300);                               // Delay 300ms
            HAL_GPIO_TogglePin(led_GPIO_Port, led_Pin); // Toggle LED state
            osDelay(300);                               // Delay 300ms
        }
    }
    return led_status;
}

/**
 * @brief LED task function
 * 
 * @param[in] argument : Task parameter (unused)
 * 
 * @return void
 * 
 * @note Priority: osPriorityNormal, Stack: 512 bytes, Period: 1ms
 */
void led_task_func(void *argument)
{
    /* USER CODE BEGIN led_task_func */
    led_status_t    led_ret   = LED_OK;
    led_operation_t led_value = LED_ON;
    
    /* Create LED queue: length 10 */
    led_queue = xQueueCreate(10, sizeof(led_operation_t));

    if ( NULL == led_queue )
    {
        printf("led Queue Create Failed!\r\n");
    }
    else
    {
        printf("led Queue Create Success!\r\n");
    }

    /* Task main loop */
    for (;;)
    {
        printf("Hello  led  Thread\r\n");

        if ( led_queue != NULL )
        {
            /* Receive command from queue, timeout: 10 ticks */
            if ( pdTRUE == xQueueReceive(led_queue, &led_value, 10) )
            {
                printf("Received led_queue value: [%d]    at [%d] tick\r\n",
                                                                  led_value, 
                                                             HAL_GetTick());
                
                /* Execute LED operation */
                led_ret = led_on_off(led_value);
                if ( LED_OK == led_ret )
                {
                    printf("LED operation success            at [%d] tick\r\n", 
                                                                HAL_GetTick());
                }
            }
        }
        
        osDelay(100);
    }
}
/* USER CODE END led_task_func */

//*************************** Defines *******************************//
