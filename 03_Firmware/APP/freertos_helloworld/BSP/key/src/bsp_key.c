/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_key.c
 * 
 * @par dependencies 
 * - bsp_key.h
 * - stdio.h
 * - stdint.h
 * 
 * @author Ethan-Hang
 * 
 * @brief Key HAL APIs and operations.
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

#include "bsp_key.h"

/* Key task handle */
osThreadId_t key_TaskHandle;

/* Key message queue handle */
QueueHandle_t key_queue;

/* Key task attributes */
const osThreadAttr_t key_Task_attributes = {
    .name = "key_Task",                         /* Task name                 */
    .stack_size = 128 * 4,                      /* Stack: 512 bytes          */
    .priority = (osPriority_t) osPriorityHigh, /* Priority: High            */
};

/**
 * @brief Key scanning function
 * 
 * @param[out] key_value : Key state (PRESSED/NOT_PRESSED)
 * 
 * @return key_status_t : Status (KEY_OK or KEY_ERRORTIMEOUT)
 * 
 * @note Polls up to 1000 times, active-low logic
 */
key_status_t key_scan(key_press_status_t *key_value)
{
    key_press_status_t current_key_value = KEY_NOT_PRESSED;

    /* Poll key state up to 1000 times */
    if ( GPIO_PIN_RESET == HAL_GPIO_ReadPin(key_GPIO_Port, key_Pin) )
    {
        current_key_value = KEY_PRESSED;    /* Set key state to pressed      */
        *key_value = current_key_value;     /* Return key state              */
        return KEY_OK;                      /* Return success                */
    }
    
    /* Timeout without detecting key press */
    *key_value = current_key_value;         /* Return not pressed state      */

    return KEY_ERRORTIMEOUT;                /* Return timeout error          */
}

/**
 * @brief Key task function
 * 
 * @param[in] argument : Task parameter (unused)
 * 
 * @return void
 * 
 * @note Priority: High, Stack: 512 bytes, Period: 100ms
 */
void key_task_func(void *argument)
{
    /* USER CODE BEGIN key_task_func */
    key_status_t       key_ret   =          KEY_OK;/* Key scan return status */
    key_press_status_t key_value = KEY_NOT_PRESSED;/* Current key state      */

    uint32_t counter_tick = 0;  /* Counter for recording key press count     */
    
    /* Create key message queue, queue length 10, each element size uint32_t */
    key_queue = xQueueCreate ( 10, sizeof(uint32_t) );
    if ( NULL == key_queue )
    {
        /* Queue creation failed */
        printf("Key Queue Create Failed!\r\n");
    }
    else
    {
        /* Queue creation successful */
        printf("Key Queue Create Success!\r\n");
    }

    /* Task main loop */
    for (;;)
    {
        counter_tick++;  /* Increment counter */

        /* Scan key state */
        key_ret = key_scan(&key_value);

        /* If key scan successful */
        if ( KEY_OK == key_ret )
        {
            /* Check if key is pressed */
            if ( KEY_PRESSED == key_value )
            {
                printf("Key Pressed!\r\n");
                
                /* Send counter value to queue, no wait (timeout 0) */
                if ( pdTRUE == xQueueSend(key_queue, &counter_tick, 0) )
                {
                    printf("Key Tick Send Success!\r\n");
                }
            }
        }
        
        /* If key scan failed, print error message */
        if ( KEY_OK != key_ret )
        {
            printf("Key Scan Error: Error code:[%d]\r\n", key_ret);
        }
        
        /* Delay 100ms */
        osDelay(100);
    }
    /* USER CODE END key_task_func */
}

/**
 * @brief Key scanning function
 * 
 * This function detects whether the key is pressed by reading the GPIO pin 
 * state.
 * It uses polling method with a maximum of 1000 attempts.
 * 
 * Execution steps:
 *  1. Loop to read GPIO pin state
 *  2. If key press is detected (GPIO is low level), return success
 *  3. If timeout occurs without detecting key press, return timeout error
 *  
 * @param[out] key_value : Pointer to key state, returns whether key is 
 *                         pressed
 *                         - KEY_PRESSED: Key is pressed
 *                         - KEY_NOT_PRESSED: Key is not pressed
 * 
 * @return key_status_t : Status (KEY_OK or KEY_ERRORTIMEOUT)
 * 
 * @note Blocks up to 1000 iterations
 */
key_status_t key_scan_short_long_press(key_press_status_t       *key_value,
                                                 uint32_t short_press_time)
{
    /*Variables*/
    key_press_status_t key_value_temp = KEY_NOT_PRESSED;
    key_status_t       ret_key_status =          KEY_OK;
    uint32_t           counter_tick   =               0;
    /*Variables*/

    // 1. check if key is pressed
    ret_key_status = key_scan(&key_value_temp);
    // TBD: critical section



    // 1.1 if pressed, check if it is short press
    if ( KEY_OK == ret_key_status )
    {
        if ( KEY_PRESSED == key_value_temp )
        {
            // get the timestamp of the key press
            counter_tick = HAL_GetTick(); 

            // 1.1.1 check the short press time
            while (HAL_GetTick() < counter_tick + short_press_time)
                ;

            // 1.1.2 check if key is still pressed
            ret_key_status = key_scan(&key_value_temp);            

            if ( KEY_NOT_PRESSED == key_value_temp )
            {
                *key_value = KEY_SHORT_PRESSED;
                return KEY_OK;
            }
            else
            {
                *key_value = KEY_LONG_PRESSED;
                // 1.2.1 keep focusing on the status of key to the short press 
                while ( KEY_OK == key_scan(&key_value_temp) )
                    ;
                return KEY_OK;
            }
        }
    }

    return ret_key_status;
}

//******************************** Defines **********************************//
