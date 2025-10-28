/******************************************************************************
 * @file uart_parse_task.h
 * 
 * @par dependencies 
 * - stdint.h
 * 
 * @author Ethan-Hang
 * 
 * @brief UART data parsing task for protocol frame detection.
 *        Implements frame-based protocol parsing with header, data,
 *        checksum verification, and tail detection.
 * 
 * Processing flow: 
 * 1. Receive data from circular buffer
 * 2. Detect frame header (0xFE)
 * 3. Collect data bytes until frame end (0xFF)
 * 4. Verify checksum and process valid data
 * 
 * @version V1.0 2025-10-28
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

#ifndef __UART_PARSE_TASK_H__
#define __UART_PARSE_TASK_H__

//******************************** Includes *********************************//
#include <stdint.h>
#include <string.h>

#include "main.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "queue.h"

#include "mid_circular_buffer.h"
#include "bsp_uart_driver.h"
#include "elog.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** @brief Frame state: No frame detected yet */
#define FRAME_NOT_DETECTED (0x01)
/** @brief Frame state: Frame header detected */
#define FRAME_HEAD         (0x02)
/** @brief Frame state: Frame end detected */
#define FRAME_END          (0x03)

/** @brief Frame header pattern byte */
#define FRAME_HEAD_FLAG    (0xFE)
/** @brief Frame end pattern byte */
#define FRAME_END_FLAG     (0xFF)

/** @brief Global buffer 1 (1 byte) */
extern uint8_t g_buffer1[1];
/** @brief Global buffer 2 (1 byte) */
extern uint8_t g_buffer2[1];

/**
 * @brief UART receive and parse task function for FreeRTOS.
 * 
 * Receives data from circular buffer and parses protocol frames.
 * Detects frame boundaries, verifies checksum, and processes data.
 * 
 * @param[in] argument : FreeRTOS task argument (unused).
 * 
 * @return None
 */
void uart_rec_A_func(void *argument);
//******************************** Defines **********************************//

#endif
