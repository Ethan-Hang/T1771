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

#include "bsp_key.h"

/* Key task handle */
osThreadId_t key_TaskHandle;

/* Key message queue handle for sending key events to other tasks */
QueueHandle_t key_queue;

/* Key task attributes configuration */
const osThreadAttr_t key_Task_attributes = {
    .name = "key_Task",                         /* Task name */
    .stack_size = 128 * 4,                      /* Stack size: 512 bytes */
    .priority = (osPriority_t) osPriorityHigh,  /* Task priority: High priority */
};

/**
 * @brief Key scanning function
 * 
 * This function detects key press using polling method.
 * It reads GPIO pin state and considers the key pressed if low level is detected.
 * Maximum 1000 polling attempts, returns timeout error if no key press detected.
 * 
 * Execution steps:
 *  1. Initialize counter and key state
 *  2. Loop up to 1000 times to read GPIO pin state
 *  3. If GPIO low level (key pressed) is detected, set state and return success
 *  4. If loop ends without detecting key press, return timeout error
 *  
 * @param[out] key_value : Pointer to key state, returns detection result
 *                         - KEY_PRESSED: Key is pressed
 *                         - KEY_NOT_PRESSED: Key is not pressed
 * 
 * @return key_status_t : Function execution status
 *                        - KEY_OK: Key press successfully detected
 *                        - KEY_ERRORTIMEOUT: Timeout without detecting key press
 * 
 * @note This function blocks execution for up to 1000 loop iterations
 * @note Key is considered pressed when GPIO pin is low level (GPIO_PIN_RESET)
 * */
key_status_t key_scan(key_press_status_t *key_value)
{
    uint32_t counter = 0;                               /* Polling counter */
    key_press_status_t current_key_value = KEY_NOT_PRESSED; /* Current key state, default not pressed */

    /* Poll key state up to 1000 times */
    while (counter < 1000)
    {
        /* Read GPIO pin state, low level means key is pressed */
        if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(key_GPIO_Port, key_Pin))
        {
            current_key_value = KEY_PRESSED;    /* Set key state to pressed */
            *key_value = current_key_value;     /* Return key state */
            return KEY_OK;                      /* Return success */
        }
        counter++;  /* Increment counter */
    }
    
    /* Timeout without detecting key press */
    *key_value = current_key_value;     /* Return not pressed state */
    return KEY_ERRORTIMEOUT;            /* Return timeout error */
}

/**
 * @brief Key task function
 * 
 * FreeRTOS task function that periodically scans key state and handles key events.
 * When key press is detected, it sends the current counter value to the message queue.
 * Other tasks can respond to key events by receiving messages from the queue.
 * 
 * Execution steps:
 *  1. Initialize key state variables
 *  2. Create key message queue (capacity 10 elements, element size uint32_t)
 *  3. Enter infinite loop:
 *     a. Call key_scan() to scan key state
 *     b. If key press detected, send counter value to queue
 *     c. Delay 100ms and continue to next iteration
 *  
 * @param[in] argument : FreeRTOS task parameter (currently unused)
 * 
 * @return void : No return value (task function should not return)
 * 
 * @note This function runs as FreeRTOS task with priority osPriorityHigh
 * @note Task stack size is 512 bytes (128*4)
 * @note Key scanning period is 100ms
 * */
void key_task_func(void *argument)
{
    /* USER CODE BEGIN key_task_func */
    key_status_t       key_ret   =          KEY_OK;         /* Key scan return status */
    key_press_status_t key_value = KEY_NOT_PRESSED;         /* Current key state */

    uint32_t counter_tick = 0;  /* Counter for recording key press count */
    
    /* Create key message queue, queue length 10, each element size uint32_t */
    key_queue = xQueueCreate ( 10, sizeof(uint32_t) );
    if (NULL == key_queue)
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
                if (pdTRUE == xQueueSend(key_queue, &counter_tick, 0))
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

//******************************** Defines **********************************//
