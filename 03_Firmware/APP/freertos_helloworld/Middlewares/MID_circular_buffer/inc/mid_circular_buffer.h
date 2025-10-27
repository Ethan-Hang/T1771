#ifndef __MID_CIRCULAR_BUFFER_H__
#define __MID_CIRCULAR_BUFFER_H__

#include <stdint.h>

#define CIRCULAR_BUFFER_SIZE            10

typedef uint32_t               data_type_t;
typedef struct
{
    data_type_t data[CIRCULAR_BUFFER_SIZE];
    uint8_t                           head;
    uint8_t                           tail;
} circular_buffer_t;

circular_buffer_t* create_empty_circular_buffer          (void);

uint8_t          buffer_is_empty  (circular_buffer_t *p_buffer);
uint8_t          buffer_is_full   (circular_buffer_t *p_buffer);
uint8_t          insert_data      (circular_buffer_t *p_buffer,\
                                         data_type_t data     );
uint8_t          get_data         (circular_buffer_t *p_buffer,\
                                         data_type_t *p_data  );
uint8_t          get_head_pos     (circular_buffer_t *p_buffer,\
                                            uint32_t *p_head  );
uint8_t          head_pos_increse (circular_buffer_t *p_buffer,\
                                            uint32_t *increse_num);                                    
#endif
