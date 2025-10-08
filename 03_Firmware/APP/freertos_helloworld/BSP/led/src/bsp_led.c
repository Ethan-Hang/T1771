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
/* LED task handle */
osThreadId_t led_TaskHandle;

/* LED task attributes configuration */
const osThreadAttr_t led_Task_attributes = {
    .name = "led_Task",                             /* Task name */
    .stack_size = 128 * 4,                          /* Stack size: 512 bytes */
    .priority = (osPriority_t) osPriorityNormal,    /* Task priority: Normal priority */
};
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
/* LED message queue handle for receiving LED control commands from other tasks */
QueueHandle_t led_queue;

//*********************Queue_Handler ********************//
/**
 * @brief LED on/off control function
 * 
 * This function controls LED on, off, or toggle operations.
 * It directly operates the GPIO pin corresponding to the LED through HAL library.
 * 
 * Execution steps:
 *  1. Determine the operation to execute based on operation type parameter
 *  2. LED_ON: Set GPIO pin to low level (RESET), turn on LED
 *  3. LED_OFF: Set GPIO pin to high level (SET), turn off LED
 *  4. LED_TOGGLE: Toggle GPIO pin state, switch LED on/off
 *  
 * @param[in] led_operation : LED operation type
 *                            - LED_ON: Turn on LED
 *                            - LED_OFF: Turn off LED
 *                            - LED_TOGGLE: Toggle LED state
 * 
 * @return led_status_t : Function execution status
 *                        - LED_OK: Operation completed successfully
 * 
 * @note LED uses active-low logic (low level on, high level off)
 * */
led_status_t led_on_off(led_operation_t led_operation)
{
    led_status_t led_status = LED_OK;   /* Return status, default success */
    
    if (LED_ON == led_operation)
    {
        /* Turn on LED: Set GPIO pin to low level */
        HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
    }
    if (LED_OFF == led_operation)
    {
        /* Turn off LED: Set GPIO pin to high level */
        HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
    }
    if (LED_TOGGLE == led_operation)
    {
        /* Toggle LED state: Switch GPIO pin level */
        HAL_GPIO_TogglePin(led_GPIO_Port, led_Pin);
    }
    return led_status;  /* Return operation status */
}

/**
 * @brief LED task function
 * 
 * FreeRTOS task function that receives LED control commands from message queue
 * and executes corresponding operations. This task continuously monitors the message
 * queue and calls led_on_off() function when LED control command is received.
 * 
 * Execution steps:
 *  1. Initialize LED status and operation variables
 *  2. Create LED message queue (capacity 10 elements, element size led_operation_t)
 *  3. Enter infinite loop:
 *     a. Print task running information
 *     b. Receive LED control command from queue (timeout 10 ticks)
 *     c. If command received successfully, print command value and call led_on_off()
 *     d. Delay 1ms and continue to next iteration
 *  
 * @param[in] argument : FreeRTOS task parameter (currently unused)
 * 
 * @return void : No return value (task function should not return)
 * 
 * @note This function runs as FreeRTOS task with priority osPriorityNormal
 * @note Task stack size is 512 bytes (128*4)
 * @note Task loop period is 1ms
 * */
void led_task_func(void *argument)
{
    /* USER CODE BEGIN led_task_func */
    led_status_t    led_ret   = LED_OK;     /* LED operation return status */
    led_operation_t led_value = LED_ON;     /* LED operation command, default turn on */
    
    /* Create LED message queue, queue length 10, each element size led_operation_t */
    led_queue = xQueueCreate(10, sizeof(led_operation_t));

    if (NULL == led_queue)
    {
        /* Queue creation failed */
        printf("led Queue Create Failed!\r\n");
    }
    else
    {
        /* Queue creation successful */
        printf("led Queue Create Success!\r\n");
    }

    /* Task main loop */
    for (;;)
    {
        printf("Hello_led_Thread\r\n");  /* Print task running information */

        /* Check if queue is valid */
        if (led_queue != NULL)
        {
            /* Receive LED control command from queue, wait time 10 ticks */
            if (pdTRUE == xQueueReceive(led_queue, &led_value, 10))
            {
                /* Command received successfully, print command value */
                printf("Received led_queue value: [%lu]\r\n", led_value);
                
                /* Execute LED operation */
                led_on_off(led_value);
            }
        }
        
        /* Delay 1ms */
        osDelay(1);
    }
}
/* USER CODE END led_task_func */

//******************************** Defines **********************************//
