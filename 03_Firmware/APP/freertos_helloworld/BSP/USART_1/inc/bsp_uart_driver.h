#ifndef __BSP_UART_DRIVER_H__
#define __BSP_UART_DRIVER_H__

#define FRONT_SEND_TO_END  0xB1B2B3B4
#include "mid_circular_buffer.h"

void uart_driver_func(void *argument);
circular_buffer_t *get_circular_buffer(void);

void dma_half_irq_callback (uint32_t number_of_data);
void dma_full_irq_callback (uint32_t number_of_data);
void uart_idle_irq_callback(uint32_t number_of_data);


#endif
