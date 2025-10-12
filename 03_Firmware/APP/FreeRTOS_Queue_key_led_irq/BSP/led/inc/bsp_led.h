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

/**
 * @brief LED operation status
 */
typedef enum
{
    LED_OK                = 0,           /* Success              */
    LED_ERROR             = 1,           /* General error        */
    LED_ERRORTIMEOUT      = 2,           /* Timeout              */
    LED_ERRORRESOURCE     = 3,           /* Resource unavailable */
    LED_ERRORPARAMETER    = 4,           /* Invalid parameter    */
    LED_ERRORNOMEMORY     = 5,           /* Out of memory        */
    LED_ERRORISR          = 6,           /* Not allowed in ISR   */
    LED_RESERVED          = 0x7FFFFFFF   /* Reserved             */
} led_status_t;

/**
 * @brief LED operation type
 */
typedef enum
{
    LED_ON                = 0,           /* Turn on LED          */
    LED_OFF               = 1,           /* Turn off LED         */
    LED_TOGGLE            = 2,           /* Toggle LED           */
    LED_BLINK_3_TIMES     = 3,           /* Blink LED 3 times    */
    LED_INITED_VALUE      = 0xFF         /* Initial value        */
} led_operation_t;
//******************************** Defines **********************************//

//******************************** Declaring ********************************//

/**
 * @brief LED on/off control function
 * 
 * @param[in] led_operation : LED operation (ON/OFF/TOGGLE)
 * 
 * @return led_status_t : Status (LED_OK on success)
 * 
 * @note Active-low logic
 */
led_status_t led_on_off(led_operation_t led_operation);

/**
 * @brief LED task function
 * 
 * @param[in] argument : Task parameter (unused)
 * 
 * @return void
 * 
 * @note Priority: Normal, Stack: 512 bytes, Period: 1ms
 */
void led_task_func(void *argument);

//******************************** Declaring ********************************//


#endif //end of __BSP_LED_H__
