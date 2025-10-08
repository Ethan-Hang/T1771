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
