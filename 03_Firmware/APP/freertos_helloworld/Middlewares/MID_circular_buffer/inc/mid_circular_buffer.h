/******************************************************************************
 * @file mid_circular_buffer.h
 * 
 * @par dependencies 
 * - stdint.h
 * 
 * @author Ethan-Hang
 * 
 * @brief Circular buffer implementation for data storage and retrieval.
 *        This module provides a thread-safe circular buffer with overflow
 *        protection and dynamic memory allocation.
 * 
 * Processing flow: 
 * 1. Create a circular buffer using create_empty_circular_buffer()
 * 2. Insert data using insert_data()
 * 3. Retrieve data using get_data()
 * 4. Check buffer status with buffer_is_empty() and buffer_is_full()
 * 
 * @version V1.0 2025-10-28
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

#ifndef __MID_CIRCULAR_BUFFER_H__
#define __MID_CIRCULAR_BUFFER_H__

//******************************** Includes *********************************//
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "elog.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** @brief Maximum size of the circular buffer */
#define CIRCULAR_BUFFER_SIZE             64

/** @brief Data type stored in the circular buffer */
typedef uint8_t                data_type_t;

/**
 * @brief Circular buffer structure
 * 
 * @param data  Array to store buffer data
 * @param head  Write pointer (index where next data will be written)
 * @param tail  Read pointer (index where next data will be read)
 */
typedef struct
{
    data_type_t data[CIRCULAR_BUFFER_SIZE];
    uint8_t                           head;
    uint8_t                           tail;
} circular_buffer_t;

/**
 * @brief Create an empty circular buffer.
 * 
 * @return circular_buffer_t* : Pointer to the created buffer or NULL if failed.
 */
circular_buffer_t* create_empty_circular_buffer          (void);

/**
 * @brief Check if the circular buffer is empty.
 * 
 * @param[in] p_buffer : Pointer to the circular buffer handler.
 * @return uint8_t : 0xFF=Error(NULL), 0x00=Empty, 0x01=Not empty.
 */
uint8_t          buffer_is_empty  (circular_buffer_t *p_buffer);

/**
 * @brief Check if the circular buffer is full.
 * 
 * @param[in] p_buffer : Pointer to the circular buffer handler.
 * @return uint8_t : 0xFF=Error(NULL), 0x00=Full, 0x01=Not full.
 */
uint8_t          buffer_is_full   (circular_buffer_t *p_buffer);

/**
 * @brief Insert data into the circular buffer.
 * 
 * @param[in] p_buffer : Pointer to the circular buffer handler.
 * @param[in] data     : Data to be inserted.
 * @return uint8_t : 0xFF=Error(NULL), 0xFE=Buffer full, 0x00=Success.
 */
uint8_t          insert_data      (circular_buffer_t *p_buffer,\
                                         data_type_t data     );

/**
 * @brief Get data from the circular buffer.
 * 
 * @param[in]  p_buffer : Pointer to the circular buffer handler.
 * @param[out] p_data   : Pointer to store the retrieved data.
 * @return uint8_t : 0xFF=Error(NULL), 0xFE=Buffer empty, 0x00=Success.
 */
uint8_t          get_data         (circular_buffer_t *p_buffer,\
                                         data_type_t *p_data  );

/**
 * @brief Get the current head position.
 * 
 * @param[in]  p_buffer : Pointer to the circular buffer handler.
 * @param[out] p_head   : Pointer to store the head position.
 * @return uint8_t : 0xFF=Error(NULL), 0x00=Success.
 */
uint8_t          get_head_pos     (circular_buffer_t *p_buffer,\
                                            uint32_t *p_head  );

/**
 * @brief Increase the head position by a specified value.
 * 
 * @param[in] p_buffer     : Pointer to the circular buffer handler.
 * @param[in] increse_num  : Pointer to the increment value.
 * @return uint8_t : 0xFF=Error(NULL), 0x00=Success.
 */
uint8_t          head_pos_increse (circular_buffer_t *p_buffer,\
                                            uint32_t *increse_num);
//******************************** Defines **********************************//
                       
#endif
