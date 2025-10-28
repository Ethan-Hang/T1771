/******************************************************************************
 * @file bsp_uart_driver.h
 * 
 * @par dependencies 
 * - usart.h
 * - FreeRTOS.h
 * - mid_circular_buffer.h
 * - elog.h
 * 
 * @author Ethan-Hang
 * 
 * @brief UART driver with DMA and circular buffer support for FreeRTOS.
 *        This module provides UART communication with DMA-based data reception,
 *        circular buffer management, and interrupt-driven data handling.
 * 
 * Processing flow: 
 * 1. Initialize UART driver task using uart_driver_func()
 * 2. DMA receives data into circular buffer automatically
 * 3. Interrupt callbacks update buffer pointers (half/full/idle)
 * 4. Application retrieves data from circular buffer
 * 
 * @version V1.0 2025-10-28
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

#ifndef __BSP_UART_DRIVER_H__
#define __BSP_UART_DRIVER_H__

//******************************** Includes *********************************//
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "mid_circular_buffer.h"
#include "elog.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** @brief Signal code sent from UART thread to end receiver */
#define FRONT_SEND_TO_END  0xB1B2B3B4

/**
 * @brief UART driver task function for FreeRTOS.
 * 
 * This function initializes the UART driver with DMA reception and 
 * circular buffer management. It runs as a FreeRTOS task and handles
 * incoming data notifications from interrupt callbacks.
 * 
 * @param[in] argument : FreeRTOS task argument (unused).
 * 
 * @return None
 */
void uart_driver_func(void *argument);

/**
 * @brief Get the circular buffer pointer used by UART driver.
 * 
 * Returns the pointer to the circular buffer that stores received UART data.
 * This allows other modules to access the received data.
 * 
 * @return circular_buffer_t* : Pointer to the circular buffer or NULL if not initialized.
 */
circular_buffer_t *get_circular_buffer(void);

/**
 * @brief DMA half-transfer complete interrupt callback.
 * 
 * Called when DMA has filled the first half of the circular buffer.
 * Updates the buffer's head pointer and notifies the driver task.
 * 
 * @param[in] number_of_data : Number of bytes received (buffer size / 2).
 * 
 * @return None
 */
void dma_half_irq_callback (uint32_t number_of_data);

/**
 * @brief DMA full-transfer complete interrupt callback.
 * 
 * Called when DMA has filled the entire circular buffer.
 * Updates the buffer's head pointer and notifies the driver task.
 * 
 * @param[in] number_of_data : Number of bytes received (full buffer size).
 * 
 * @return None
 */
void dma_full_irq_callback (uint32_t number_of_data);

/**
 * @brief UART idle line interrupt callback.
 * 
 * Called when UART idle condition is detected (no data received for a period).
 * Updates the buffer's head pointer based on actual received data count
 * and notifies the driver task.
 * 
 * @param[in] number_of_data : Actual number of bytes received before idle.
 * 
 * @return None
 */
void uart_idle_irq_callback(uint32_t number_of_data);
//******************************** Defines **********************************//

#endif
