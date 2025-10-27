#include "mid_circular_buffer.h"
#include <stdlib.h>
#include <string.h>
#include "elog.h"

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

    *p_data = p_buffer->data[p_buffer->tail];
    p_buffer->tail++;

    return 0x00;
}

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
                                    