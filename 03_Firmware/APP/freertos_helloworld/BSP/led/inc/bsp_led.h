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
extern osThreadId_t led_TaskHandle;
extern const osThreadAttr_t led_Task_attributes;
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
extern QueueHandle_t led_queue;
//*********************Queue_Handler ********************//

typedef enum
{
    LED_OK                = 0,           /* Operation completed successfully.  */
    LED_ERROR             = 1,           /* Run-time error without case matched*/
    LED_ERRORTIMEOUT      = 2,           /* Operation failed with timeout      */
    LED_ERRORRESOURCE     = 3,           /* Resource not available.            */
    LED_ERRORPARAMETER    = 4,           /* Parameter error.                   */
    LED_ERRORNOMEMORY     = 5,           /* Out of memory.                     */
    LED_ERRORISR          = 6,           /* Not allowed in ISR context         */
    LED_RESERVED          = 0x7FFFFFFF   /* Reserved                           */
} led_status_t;

typedef enum
{
    LED_ON,
    LED_OFF,
    LED_TOGGLE
} led_operation_t;
//******************************** Defines **********************************//

//******************************** Declaring ********************************//

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
led_status_t led_on_off(led_operation_t led_operation);

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
void                        led_task_func(void *argument);

//******************************** Declaring ********************************//


#endif //end of __BSP_LED_H__
