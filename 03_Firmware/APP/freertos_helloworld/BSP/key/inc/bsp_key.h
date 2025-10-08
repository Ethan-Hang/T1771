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
extern osThreadId_t key_TaskHandle;
extern const osThreadAttr_t key_Task_attributes;
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
extern QueueHandle_t key_queue;
//*********************Queue_Handler ********************//

typedef enum
{
    KEY_OK                = 0,           /* Operation completed successfully.  */
    KEY_ERROR             = 1,           /* Run-time error without case matched*/
    KEY_ERRORTIMEOUT      = 2,           /* Operation failed with timeout      */
    KEY_ERRORRESOURCE     = 3,           /* Resource not available.            */
    KEY_ERRORPARAMETER    = 4,           /* Parameter error.                   */
    KEY_ERRORNOMEMORY     = 5,           /* Out of memory.                     */
    KEY_ERRORISR          = 6,           /* Not allowed in ISR context         */
    KEY_RESERVED          = 0x7FFFFFFF   /* Reserved                           */
} key_status_t;

typedef enum
{
    KEY_PRESSED           = 0,           /* Operation completed successfully.  */
    KEY_NOT_PRESSED       = 1,           /* Run-time error without case matched*/
} key_press_status_t;
//******************************** Defines **********************************//

//******************************** Declaring ********************************//

/**
 * @brief Instantiates the bsp_led_handler_t target.
 * 
 * Steps:
 *  1. Adds Core interfaces into bsp_led_driver instance target.
 *  
 * @param[in] self        : Pointer to the target of handler.
 * @return led_handler_status_t : Status of the function.
 * 
 * */
key_status_t key_scan(key_press_status_t *key_value);

/**
 * @brief Instantiates the bsp_led_handler_t target.
 * 
 * Steps:
 *  1. Adds Core interfaces into bsp_led_driver instance target.
 *  
 * @param[in] self        : Pointer to the target of handler.
 * @return led_handler_status_t : Status of the function.
 * 
 * */
void key_task_func(void *argument);

//******************************** Declaring ********************************//


#endif //end of __BSP_KEY_H__
