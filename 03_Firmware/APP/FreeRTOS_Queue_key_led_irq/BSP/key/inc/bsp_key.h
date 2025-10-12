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

//******************** Thread_Func ********************//
extern osThreadId_t key_TaskHandle;
extern const osThreadAttr_t key_Task_attributes;
//******************** Thread_Func ********************//

//******************* Queue_Handler *******************//
extern QueueHandle_t key_queue;
//******************* Queue_Handler *******************//

/**
 * @brief Key operation status enumeration
 * 
 * Defines return status codes for key-related operations
 */
typedef enum
{
    KEY_OK                = 0,           /* Success */
    KEY_ERROR             = 1,           /* General error */
    KEY_ERRORTIMEOUT      = 2,           /* Timeout */
    KEY_ERRORRESOURCE     = 3,           /* Resource unavailable */
    KEY_ERRORPARAMETER    = 4,           /* Invalid parameter */
    KEY_ERRORNOMEMORY     = 5,           /* Out of memory */
    KEY_ERRORISR          = 6,           /* Not allowed in ISR */
    KEY_RESERVED          = 0x7FFFFFFF   /* Reserved */
} key_status_t;

/**
 * @brief Key press status
 */
typedef enum
{
    KEY_PRESSED           = 0,                            /*     Key pressed */
    KEY_NOT_PRESSED       = 1,                            /* Key not pressed */
    KEY_SHORT_PRESSED     = 2,                            /*     Short press */
    KEY_LONG_PRESSED      = 3,                            /*      Long press */
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
 * @return key_status_t : Status (KEY_OK or KEY_ERRORTIMEOUT)
 * 
 * @note Blocks up to 1000 iterations
 */
key_status_t key_scan_short_long_press          (key_press_status_t *key_value,
                                                    uint32_t short_press_time);

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
 * @return key_status_t : Status (KEY_OK or KEY_ERRORTIMEOUT)
 * 
 * @note Blocks up to 1000 iterations
 */
key_status_t key_scan                          (key_press_status_t *key_value);

/**
 * @brief Key task function
 * 
 * @param[in] argument : Task parameter (unused)
 * 
 * @return void
 * 
 * @note Priority: High, Stack: 512 bytes, Period: 100ms
 */
void key_task_func(void *argument);

//******************************** Declaring ********************************//


#endif //end of __BSP_KEY_H__
