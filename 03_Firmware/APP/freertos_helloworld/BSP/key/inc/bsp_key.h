/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_key.h
 * 
 * @par dependencies 
 * - bsp_key.h
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

#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

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
extern osThreadId_t key_TaskHandle;                 /* Key task handle */
extern const osThreadAttr_t key_Task_attributes;    /* Key task attributes configuration */
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
extern QueueHandle_t key_queue;                     /* Key message queue handle for inter-task communication */
//*********************Queue_Handler ********************//

/**
 * @brief Key operation status enumeration
 * 
 * Defines return status codes for key-related operations
 */
typedef enum
{
    KEY_OK                = 0,           /* Operation completed successfully */
    KEY_ERROR             = 1,           /* Run-time error without specific case matched */
    KEY_ERRORTIMEOUT      = 2,           /* Operation failed with timeout */
    KEY_ERRORRESOURCE     = 3,           /* Resource not available */
    KEY_ERRORPARAMETER    = 4,           /* Parameter error */
    KEY_ERRORNOMEMORY     = 5,           /* Out of memory */
    KEY_ERRORISR          = 6,           /* Not allowed in ISR context */
    KEY_RESERVED          = 0x7FFFFFFF   /* Reserved value */
} key_status_t;

/**
 * @brief Key press status enumeration
 * 
 * Defines the physical state of the key
 */
typedef enum
{
    KEY_PRESSED           = 0,           /* Key is pressed */
    KEY_NOT_PRESSED       = 1,           /* Key is not pressed */
} key_press_status_t;
//******************************** Defines **********************************//

//******************************** Declaring ********************************//

/**
 * @brief Key scanning function
 * 
 * This function detects whether the key is pressed by reading the GPIO pin state.
 * It uses polling method with a maximum of 1000 attempts.
 * 
 * Execution steps:
 *  1. Loop to read GPIO pin state
 *  2. If key press is detected (GPIO is low level), return success
 *  3. If timeout occurs without detecting key press, return timeout error
 *  
 * @param[out] key_value : Pointer to key state, returns whether key is pressed
 *                         - KEY_PRESSED: Key is pressed
 *                         - KEY_NOT_PRESSED: Key is not pressed
 * 
 * @return key_status_t : Function execution status
 *                        - KEY_OK: Key press detected
 *                        - KEY_ERRORTIMEOUT: Timeout without detecting key press
 * 
 * @note This function blocks execution for up to 1000 loop iterations
 * */
key_status_t key_scan(key_press_status_t *key_value);

/**
 * @brief Key task function
 * 
 * FreeRTOS task function that periodically scans key state and handles key events.
 * When a key press is detected, it sends the counter value to the message queue
 * for other tasks to use.
 * 
 * Execution steps:
 *  1. Create key message queue (10 elements, each element size is uint32_t)
 *  2. Enter infinite loop
 *  3. Call key_scan() to scan key state
 *  4. If key press is detected, send counter value to queue
 *  5. Delay 100ms and continue to next iteration
 *  
 * @param[in] argument : FreeRTOS task parameter (unused)
 * 
 * @return void : No return value (task function should not return)
 * 
 * @note This function runs as FreeRTOS task with priority osPriorityHigh
 * @note Task stack size is 512 bytes (128*4)
 * */
void key_task_func(void *argument);

//******************************** Declaring ********************************//


#endif //end of __BSP_KEY_H__
