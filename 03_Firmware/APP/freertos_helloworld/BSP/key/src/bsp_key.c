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

osThreadId_t key_TaskHandle;
QueueHandle_t key_queue;

const osThreadAttr_t key_Task_attributes = {
    .name = "key_Task",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityHigh,
};

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
key_status_t key_scan(key_press_status_t *key_value)
{
    uint32_t counter = 0;
    key_press_status_t current_key_value = KEY_NOT_PRESSED;

    while (counter < 1000)
    {
        if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(key_GPIO_Port, key_Pin))
        {
            current_key_value = KEY_PRESSED;
            *key_value = current_key_value;
            return KEY_OK;
        }
        counter++;
    }
    *key_value = current_key_value;

    return KEY_ERRORTIMEOUT;

}

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
void key_task_func(void *argument)
{
    /* USER CODE BEGIN key_task_func */
    key_status_t       key_ret   =          KEY_OK;
    key_press_status_t key_value = KEY_NOT_PRESSED;

    uint32_t counter_tick = 0;
    key_queue = xQueueCreate ( 10, sizeof(uint32_t) );
    if (NULL == key_queue)
    {
        printf("Key Queue Create Failed!\r\n");
    }
    else
    {
        printf("Key Queue Create Success!\r\n");
    }

    for (;;)
    {
        counter_tick++;

        key_ret = key_scan(&key_value);

        if ( KEY_OK == key_ret )
        {
            if ( KEY_PRESSED == key_value )
            {
                printf("Key Pressed!\r\n");
                if (pdTRUE == xQueueSend(key_queue, &counter_tick, 0))
                {
                    printf("Key Tick Send Success!\r\n");
                }
            }
        }
        if ( KEY_OK != key_ret )
        {
            printf("Key Scan Error: Error code:[%d]\r\n", key_ret);
        }
        osDelay(100);
    }
    /* USER CODE END key_task_func */
}

//******************************** Defines **********************************//
