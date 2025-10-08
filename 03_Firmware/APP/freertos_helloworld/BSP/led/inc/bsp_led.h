/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_led.h
 * 
 * @par dependencies 
 * - bsp_led.h
 * - stdio.h
 * - stdint.h
 * 
 * @author Ethan-Hang
 * 
 * @brief Provide the HAL APIs of Key and corresponding opetions.
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

#ifndef __BSP_LED_H__
#define __BSP_LED_H__

//******************************** Includes *********************************//
#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "queue.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"


//******************************** Includes *********************************//

//******************************** Defines **********************************//

//*********************Thread_Func **********************//
extern osThreadId_t led_TaskHandle;                 /* LED task handle */
extern const osThreadAttr_t led_Task_attributes;    /* LED task attributes configuration */
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
extern QueueHandle_t led_queue;                     /* LED message queue handle for receiving LED control commands */
//*********************Queue_Handler ********************//

/**
 * @brief LED operation status enumeration
 * 
 * Defines return status codes for LED-related operations
 */
typedef enum
{
    LED_OK                = 0,           /* Operation completed successfully */
    LED_ERROR             = 1,           /* Run-time error without specific case matched */
    LED_ERRORTIMEOUT      = 2,           /* Operation failed with timeout */
    LED_ERRORRESOURCE     = 3,           /* Resource not available */
    LED_ERRORPARAMETER    = 4,           /* Parameter error */
    LED_ERRORNOMEMORY     = 5,           /* Out of memory */
    LED_ERRORISR          = 6,           /* Not allowed in ISR context */
    LED_RESERVED          = 0x7FFFFFFF   /* Reserved value */
} led_status_t;

/**
 * @brief LED operation type enumeration
 * 
 * Defines LED control operation types
 */
typedef enum
{
    LED_ON,         /* Turn on LED */
    LED_OFF,        /* Turn off LED */
    LED_TOGGLE      /* Toggle LED state */
} led_operation_t;
//******************************** Defines **********************************//

//******************************** Declaring ********************************//

/**
 * @brief LED on/off control function
 * 
 * This function controls LED on, off, or toggle operations.
 * It directly controls the GPIO pin state corresponding to the LED through HAL library.
 * 
 * Execution steps:
 *  1. Perform corresponding GPIO operation based on the operation type parameter
 *  2. LED_ON: Set GPIO pin to low level (turn on LED)
 *  3. LED_OFF: Set GPIO pin to high level (turn off LED)
 *  4. LED_TOGGLE: Toggle GPIO pin state (switch LED on/off state)
 *  
 * @param[in] led_operation : LED operation type
 *                            - LED_ON: Turn on LED
 *                            - LED_OFF: Turn off LED
 *                            - LED_TOGGLE: Toggle LED state
 * 
 * @return led_status_t : Function execution status
 *                        - LED_OK: Operation successful
 * 
 * @note LED is on at low level and off at high level
 * */
led_status_t led_on_off(led_operation_t led_operation);

/**
 * @brief LED task function
 * 
 * FreeRTOS task function that receives LED control commands from message queue
 * and executes corresponding operations. This task waits for LED control commands
 * from other tasks (e.g., key task) through the queue.
 * 
 * Execution steps:
 *  1. Create LED message queue (10 elements, each element size led_operation_t)
 *  2. Enter infinite loop
 *  3. Receive LED control command from queue (timeout 10 ticks)
 *  4. If command received successfully, call led_on_off() to execute LED operation
 *  5. Delay 1ms and continue to next iteration
 *  
 * @param[in] argument : FreeRTOS task parameter (unused)
 * 
 * @return void : No return value (task function should not return)
 * 
 * @note This function runs as FreeRTOS task with priority osPriorityNormal
 * @note Task stack size is 512 bytes (128*4)
 * */
void                        led_task_func(void *argument);

//******************************** Declaring ********************************//


#endif //end of __BSP_LED_H__
