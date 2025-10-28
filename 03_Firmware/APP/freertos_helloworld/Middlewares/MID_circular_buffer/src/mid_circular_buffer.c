/******************************************************************************
 * @file mid_circular_buffer.c
 * 
 * @par dependencies 
 * - mid_circular_buffer.h
 * - stdlib.h
 * - string.h
 * - elog.h
 * 
 * @author Ethan-Hang
 * 
 * @brief 
 * 
 * Processing flow: 
 * 
 * call directly.
 * 
 * @version V1.0 2025-10-28
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

//******************************** Includes *********************************//
#include "mid_circular_buffer.h"
//******************************** Includes *********************************//

//************************** Function Implementations ***********************//
/**
 * @brief Create an empty circular buffer.
 * 
 * Allocates memory for a new circular buffer structure and initializes 
 * all fields to zero.
 * 
 * @param[in] None
 * 
 * @return      circular_buffer_t* : 
 *                      Pointer to the newly created circular buffer;
 *                      NULL if memory allocation failed.
 * 
 * */
circular_buffer_t* create_empty_circular_buffer          (void)
{
    circular_buffer_t* p_buffer = (circular_buffer_t *)            \
                                  malloc(sizeof(circular_buffer_t));
    if (NULL == p_buffer)
    {
        log_e("malloc failed in create_empty_circular_buffer");
        return NULL;
    }
    
    memset ( p_buffer, 0, sizeof(circular_buffer_t) );

    return p_buffer;
}

/**
 * @brief Check if the circular buffer is empty.
 * 
 * Determines whether the buffer contains any data by comparing 
 * head and tail pointers.
 * 
 * @param[in] p_buffer : Pointer to the circular buffer handler.
 * 
 * @return      uint8_t : 
 *                      0xFF: Error, the p_buffer is NULL;
 *                      0x00: Buffer is empty;
 *                      0x01: Buffer is not empty.
 * 
 * */
uint8_t            buffer_is_empty(circular_buffer_t *p_buffer)
{
    if (NULL == p_buffer)
    {
        return 0xFF;
    }

    if (p_buffer->head == p_buffer->tail)
    {
        return 0x00;
    }
    else
    {
        return 0x01;
    }
}

/**
 * @brief Check if the circular buffer is full.
 * 
 * Determines whether the buffer has reached its maximum capacity.
 * The buffer is considered full when head+1 equals tail position.
 * 
 * @param[in] p_buffer : Pointer to the circular buffer handler.
 * 
 * @return      uint8_t : 
 *                      0xFF: Error, the p_buffer is NULL;
 *                      0x00: Buffer is full;
 *                      0x01: Buffer is not full.
 * 
 * */
uint8_t            buffer_is_full (circular_buffer_t *p_buffer)
{
    if (NULL == p_buffer)
    {
        return 0xFF;
    }

    if (
        ((p_buffer->head + 1) % CIRCULAR_BUFFER_SIZE) ==
        ((p_buffer->tail    ) % CIRCULAR_BUFFER_SIZE)
       )
    {
        return 0x00;
    }
    else
    {
        return 0x01;
    }
}

/**
 * @brief Insert data into the circular buffer.
 * 
 * Adds a new data element to the buffer at the head position.
 * The head pointer is automatically incremented after insertion.
 * 
 * @param[in] p_buffer : Pointer to the circular buffer handler.
 * @param[in] data     : Data to be inserted into the buffer.
 * 
 * @return      uint8_t : 
 *                      0xFF: Error, the p_buffer is NULL;
 *                      0xFE: Error, buffer is full;
 *                      0x00: Success.
 * 
 * */
uint8_t            insert_data    (circular_buffer_t *p_buffer,\
                                         data_type_t data      )
{
    if (NULL == p_buffer)
    {
        return 0xFF;
    }
    if (0x00 == buffer_is_full(p_buffer))
    {
        return 0xFE;
    }


    p_buffer->data[(p_buffer->head) % CIRCULAR_BUFFER_SIZE] = data;
    p_buffer->head++;

    return 0x00;
}

/**
 * @brief Get data from the circular buffer.
 * 
 * Retrieves one data element from the buffer at the tail position.
 * The tail pointer is automatically incremented after retrieval.
 * 
 * @param[in]  p_buffer : Pointer to the circular buffer handler.
 * @param[out] p_data   : Pointer to store the retrieved data.
 * 
 * @return      uint8_t : 
 *                      0xFF: Error, the p_buffer is NULL;
 *                      0xFE: Error, buffer is empty;
 *                      0x00: Success.
 * 
 * */
uint8_t            get_data       (circular_buffer_t *p_buffer,\
                                         data_type_t *p_data   )
{
    if (NULL == p_buffer)
    {
        return 0xFF;
    }
    if (0x00 == buffer_is_empty(p_buffer))
    {
        return 0xFE;
    }

    *p_data = p_buffer->data[p_buffer->tail % CIRCULAR_BUFFER_SIZE];
    p_buffer->tail++;

    return 0x00;
}

/**
 * @brief Get the current head position of the circular buffer.
 * 
 * Retrieves the current head pointer value which indicates 
 * where the next data will be inserted.
 * 
 * @param[in]  p_buffer : Pointer to the circular buffer handler.
 * @param[out] p_head   : Pointer to store the head position value.
 * 
 * @return      uint8_t : 
 *                      0xFF: Error, the p_buffer is NULL;
 *                      0x00: Success.
 * 
 * */
uint8_t            get_head_pos   (circular_buffer_t *p_buffer,\
                                            uint32_t    *p_head)
{
    if (NULL == p_buffer)
    {
        return 0xFF;
    }

    *p_head = p_buffer->head;

    return 0x00;
}

/**
 * @brief Increase the head position by a specified value.
 * 
 * Manually increments the head pointer by the given number.
 * This is useful for batch operations or direct buffer manipulation.
 * 
 * @param[in] p_buffer     : Pointer to the circular buffer handler.
 * @param[in] increse_num  : Pointer to the increment value.
 * 
 * @return      uint8_t : 
 *                      0xFF: Error, the p_buffer is NULL;
 *                      0x00: Success.
 * 
 * */
uint8_t          head_pos_increse (circular_buffer_t *p_buffer,  \
                                            uint32_t *increse_num)
{
    if (NULL == p_buffer)
    {
        return 0xFF;
    }
    
    p_buffer->head += (*increse_num);

    return 0x00;
}
//************************** Function Implementations ***********************//
