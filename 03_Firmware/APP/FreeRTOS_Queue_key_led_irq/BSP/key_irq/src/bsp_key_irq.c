/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_key.c
 * 
 * @par dependencies 
 * - bsp_key_irq.h
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
 * @version V1.0 2025-10-09
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

#include "bsp_key_irq.h"

//******************************** Defines **********************************//
#define FALLING_TYPE 0
#define RISING_TYPE  1
//******************************** Defines **********************************//

/* Key task handle */
osThreadId_t key_TaskHandle;

/* Key message queue handle */
QueueHandle_t       key_queue = NULL;
QueueHandle_t inter_key_queue = NULL;

/* Key task attributes */
const osThreadAttr_t key_Task_attributes = {
    .name = "key_Task",                         /* Task name                 */
    .stack_size = 128 * 4,                      /* Stack: 512 bytes          */
    .priority = (osPriority_t) osPriorityNormal,  /* Priority: High          */
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
    if ( GPIO_PIN_RESET == HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) )
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
    /**                     Variables (in task stack)                       **/
    uint32_t               event_index =                                     0;
    uint32_t        first_trigger_tick =                                     0;
    uint32_t          short_press_time =                      SHORT_PRESS_TIME;
    key_press_event_t  key_press_event =                  
    {
        .edge_type = RISING,
        .trigger_tick = 0
    };
    /**                     Variables (in task stack)                       **/

    
    /**                      Variables (in OS heap)                         **/
    key_queue       =  xQueueCreate ( 10, sizeof( key_press_status_t ) );
    inter_key_queue =  xQueueCreate ( 10, sizeof( key_press_event_t  ) );

    /**                      Variables (in OS heap)                         **/

    /* Check if queue creation was successful */
    if ( NULL == key_queue && NULL == inter_key_queue )
    {
        /* Queue creation failed */
        printf("Key Queue Create Failed!\r\n");
        return ;
    }
    else
    {
        /* Queue creation successful */
        printf("Key Queue Create Success!\r\n");
    }

    /* Task main loop */
    for (;;)
    {
        printf("key_task_func running               at"
                                        "[%d] tick\r\n",
                                         HAL_GetTick());

        // 1. check if there is new data about key press in the queue
        if ( pdTRUE == xQueueReceive(       inter_key_queue, 
                                       &( key_press_event ), 
                                        ( TickType_t ) 0 ) )
        {
            printf("key_press_event.trigger_tick          "
                                                 "[%d]\r\n",
                              key_press_event.trigger_tick);

            printf("inter_key_queue receive key event   at"
                                            "[%d] tick\r\n", 
                                             HAL_GetTick());
        // 1. process the new data about key press
            // 1.1 if there is a new data about the key
            // then update it in state machine
            if ( RISING  == key_press_event.edge_type &&
                 0       ==                event_index )
            {
                printf("Key Rising fetched Error!\r\n");
            }
            if ( FALLING == key_press_event.edge_type &&
                 0       ==                event_index )
            {
                printf("Key first FALLING fetched\r\n");

                // change the index for chaing state machine
                event_index += 1;

                // Mark the first tick when event coming
                first_trigger_tick = key_press_event.trigger_tick;
            }
            if ( RISING  == key_press_event.edge_type &&
                 1       ==                event_index )
            {
                printf( "Key Rising after falling\r\n" );

                // 1.1.1 if the interval between two key presses less than 10ms
                if ( key_press_event.trigger_tick - first_trigger_tick < 10)
                {
                    // 1.1.1.1 the new key press is invalid
                    printf("Invaled key fetched                    at"
                                                       "[%d] tick\r\n", 
                                                        HAL_GetTick());
                    continue;
                }
                // 1.1.2 if the interval between two key presses more than 10ms
                // 1.1.2.1 if the interval is less than short press time
                if (key_press_event.trigger_tick - first_trigger_tick < \
                                                        short_press_time)
                {
                    // send the short press message to key_queue
                    key_press_status_t key_result = KEY_SHORT_PRESSED;

                    if (pdTRUE == xQueueSendToFront(       key_queue, 
                                                         &key_result, 
                                                  ( TickType_t ) 0 ))
                    {
                        printf("key_result send short press         at" 
                                                        "[%d] tick\r\n",
                                                         HAL_GetTick());
                        event_index = 0;
                    }
                    else
                    {
                        printf("key_result send short press failed  at" 
                                                        "[%d] tick\r\n",
                                                         HAL_GetTick());
                    }
                }
                    
                    // 1.1.2.2 if the interval is more than short press time
                    // send the long  press message to key_queue
                if (key_press_event.trigger_tick - first_trigger_tick >\
                                                       short_press_time)
                {
                    // send the short press message to key_queue
                    key_press_status_t key_result =    KEY_LONG_PRESSED;

                    if (pdTRUE == xQueueSendToFront(          key_queue, 
                                                            &key_result, 
                                                    ( TickType_t ) 0 ) )
                    {
                        printf("key_result send long press          at"
                                                        "[%d] tick\r\n",
                                                         HAL_GetTick());
                        event_index = 0;
                    } 
                    else
                    {
                        printf("key_result send long press failed   at" 
                                                        "[%d] tick\r\n",
                                                         HAL_GetTick());
                    }
                }
                    
            }
        }
        
        osDelay(200);
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

/**
 * @brief Key interrupt callback function
 * 
 * Steps:
 *  1. if trigger first with falling type, \
 *     send the event to the inter_key_queue \
 *     changing the interruption type to rising
 * 
 *  2. if trigger then with rising type, \
 *     send the event to the inter_key_queue \
 *    changing the interruption type back to falling
 * @param[in] void 
 * 
 * @return void
 * 
 */
KEY_CALLBACK
{
    static uint8_t irq_type = FALLING_TYPE;
    /* 
    *  1. if trigger first with falling type, \
    *     send the event to the inter_key_queue \
    *     changing the interruption type to rising
    */
    BaseType_t xHigherPrioritTaskWoken;

    if ( FALLING_TYPE == irq_type )
    {
        key_press_event_t key_press_event =                 
        {
            .edge_type = FALLING,
            .trigger_tick = HAL_GetTick()
        };
        if (pdTRUE == xQueueSendToBackFromISR(            inter_key_queue,
                                                         &key_press_event, 
                                                &xHigherPrioritTaskWoken))
        {
            printf("key_press_event send falling in irq at" 
                    "[%d] tick\r\n",
                     HAL_GetTick());
        }
        /* 
        *  1.1 change the irq_type to rising
        */
        irq_type = RISING_TYPE;

        /* 
        *  1.2 change the interruption type to rising
        */
        GPIO_InitTypeDef GPIO_InitStruct =                  {0};
        GPIO_InitStruct.Pin              =              KEY_Pin;
        GPIO_InitStruct.Mode             =  GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull             =          GPIO_PULLUP;
        HAL_GPIO_Init(KEY_GPIO_Port, &GPIO_InitStruct);
    }
    /*
    *2. if trigger then with rising type, \
    *send the event to the inter_key_queue \
    *changing the interruption type back to falling
    */
    else if ( RISING_TYPE == irq_type )
    {
        key_press_event_t key_press_event_2 =                 
        {
            .edge_type = RISING,
            .trigger_tick = HAL_GetTick()
        };
        if (pdTRUE == xQueueSendToBackFromISR(            inter_key_queue,
                                                       &key_press_event_2,
                                                &xHigherPrioritTaskWoken))
        {
            printf("key_press_event send rising in irq  at"
                    "[%d] tick\r\n",
                     HAL_GetTick());
        }
        /* 
        *  2.1 change the irq_type to falling
        */
        irq_type = FALLING_TYPE;

        /* 
        *  2.2 change the interruption type to falling
        */
        GPIO_InitTypeDef GPIO_InitStruct =                  {0};
        GPIO_InitStruct.Pin              =              KEY_Pin;
        GPIO_InitStruct.Mode             = GPIO_MODE_IT_FALLING;
        GPIO_InitStruct.Pull             =          GPIO_PULLUP;
        HAL_GPIO_Init(KEY_GPIO_Port, &GPIO_InitStruct);  
    }    
}

//******************************** Defines **********************************//
