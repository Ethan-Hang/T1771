#include "bsp_uart_driver.h"

#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "mid_circular_buffer.h"
#include "elog.h"


#define IRQ_SEND_TO_THREAD 0xA1A2A3A4

// #define USE_AB_BUFFER
#define USE_CIRCULAR_BUFFER

#if defined (USE_AB_BUFFER)
#define BUFFER_A 0
#define BUFFER_B 1
uint8_t flag_AB      =  0 ;
uint8_t g_bufferA[1] = {0};
uint8_t g_bufferB[1] = {0};
#endif

#if defined (USE_CIRCULAR_BUFFER)

extern QueueHandle_t       queue_irq_rec_A                    ;
static circular_buffer_t * g_circular_buffer_irq_thread = NULL;
static QueueHandle_t       queue_uart_irq_thread        = NULL;
uint8_t g_buffer[1] = {0};

#endif


void uart_driver_func(void *argument)
{
    log_i("uart_driver_func is running...");

#if defined (USE_AB_BUFFER)
    flag_AB = BUFFER_A;
    if (HAL_OK != HAL_UART_Receive_IT(&huart1, (uint8_t *)g_bufferA, 1))
    {
        log_e("HAL_UART_Receive_IT error");
    }
    else
    {
        log_i("HAL_UART_Receive_IT success");
    }
#endif // AB buffer method

#if defined (USE_CIRCULAR_BUFFER)
    queue_uart_irq_thread = xQueueCreate (1, 4);
    if (NULL == queue_uart_irq_thread)
    {
        log_e("queue_uart_irq_thread create failed");
        return;
    }

    circular_buffer_t * p_circular_buffer = create_empty_circular_buffer();
    if ( NULL == p_circular_buffer )
    {
        log_e("circular_buffer create failed");
    }
    g_circular_buffer_irq_thread = p_circular_buffer;
    log_i("circular_buffer create Success");

    if (HAL_OK != HAL_UARTEx_ReceiveToIdle_DMA(                        &huart1,
                                            (uint8_t *)p_circular_buffer->data, 
                                                        CIRCULAR_BUFFER_SIZE) )
    // if (HAL_OK != HAL_UART_Receive_IT(&huart1, g_buffer, 1))
    {
        log_e("HAL_UART_Receive_IT error");
    }
    else
    {
        log_i("HAL_UART_Receive_IT success");
    }
#endif // circular buffer method

    uint32_t receive_data = 0;
    /* Infinite loop */
    for (;;)
    {
        if (pdPASS == xQueuePeek(   queue_uart_irq_thread, 
                                            &receive_data, 
                                           portMAX_DELAY))
        {
        }
        log_i("front receive cmd from irq [%u]", receive_data);
        
        if (IRQ_SEND_TO_THREAD == receive_data)
        {
            uint32_t send_to_end = FRONT_SEND_TO_END;
            if (pdPASS == xQueueOverwrite( queue_irq_rec_A,
                                             &send_to_end))
            {

            }

        }

    }
    /* USER CODE END  */
}

circular_buffer_t *get_circular_buffer(void)
{
    if (NULL == g_circular_buffer_irq_thread)
    {
        return NULL;
    }
    return g_circular_buffer_irq_thread;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_UART_RxCpltCallback could be implemented in the user file
    */
#if defined (USE_AB_BUFFER)
    log_d("HAL_UART_RxCpltCallback AB buffer method is called");
    if (huart->Instance == USART1)
    {
        if (BUFFER_A == flag_AB)
        {
            log_d("Received data: %c", g_bufferA[0]);
            if (HAL_OK != HAL_UART_Receive_IT(&huart1, (uint8_t*)g_bufferB, 1))
            {
                log_e("HAL_UART_Receive_IT error");
            }
            flag_AB = BUFFER_B;
        }
        else if (BUFFER_B == flag_AB)
        {
            log_d("Received data: %c", g_bufferB[0]);
            if (HAL_OK != HAL_UART_Receive_IT(&huart1, (uint8_t*)g_bufferA, 1))
            {
                log_e("HAL_UART_Receive_IT error");
            }
            flag_AB = BUFFER_A;
        }
    }
#endif // AB buffer method

#if defined (USE_CIRCULAR_BUFFER)
    log_d("HAL_UART_RxCpltCallback circular buffer method is called");
    
    if (NULL == g_circular_buffer_irq_thread)
    {
        log_e("g_circular_buffer_irq_thread is NULL");
        return;
    }

    if (0x00 == insert_data(g_circular_buffer_irq_thread, *g_buffer))
    {
        // uint8_t temp_get_data;
        // if (0x00 == get_data(g_circular_buffer_irq_thread, &temp_get_data))
        // {
        //     log_d("Received data: %c", temp_get_data);
        // }
    }

    // insert complete send queue
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    xQueueOverwriteFromISR(          queue_uart_irq_thread,
                                           &send_to_thread,
                                &xHigherPriorityTaskWoken);

    if (HAL_OK != HAL_UART_Receive_IT(&huart1, g_buffer, 1))
    {
        log_e("HAL_UART_Receive_IT error");
    }
    else
    {
        log_i("HAL_UART_Receive_IT success");
    }

#endif // circular buffer method

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    UNUSED(Size);

    /* NOTE : This function should not be modified, when the callback is needed,
              the HAL_UARTEx_RxEventCallback can be implemented in the user file.
    */
    // log_d("HAL_UARTEx_RxEventCallback is called size = [%u]", Size);
}


void dma_half_irq_callback (uint32_t number_of_data)
{
    // log_d("dma_half_irq = [%u]", number_of_data);
    uint32_t head_pos = 0;
    uint8_t  ret      = 0;

    ret = get_head_pos(g_circular_buffer_irq_thread, &head_pos);
    if (0x00 != ret)
    {

        return;    
    }

    uint32_t currunt_data_pos =         (CIRCULAR_BUFFER_SIZE / 2) - 1;

    uint32_t pos_in_buffer    =  head_pos % (CIRCULAR_BUFFER_SIZE / 2);

    uint32_t increse_pos      =       currunt_data_pos - pos_in_buffer;

    ret = head_pos_increse(g_circular_buffer_irq_thread, &increse_pos);
    if (0x00 != ret)
    {
        log_e("head_pos_increse failed");
        return;    
    }

    //test
    ret = get_head_pos(g_circular_buffer_irq_thread, &head_pos);
    if (0x00 != ret)
    {
        log_d("get head pos error");
    }
    log_d("head_pos = [%d]", head_pos);

    pos_in_buffer = head_pos % (CIRCULAR_BUFFER_SIZE / 2);
    log_d("pos_in_half_buffer = [%d]", pos_in_buffer);
}

void dma_full_irq_callback (uint32_t number_of_data)
{
    // log_d("dma_full_irq = [%u]", number_of_data);

    uint32_t head_pos = 0;
    uint8_t  ret      = 0;

    ret = get_head_pos(g_circular_buffer_irq_thread, &head_pos);
    if (0x00 != ret)
    {

        return;    
    }

    uint32_t currunt_data_pos =             (CIRCULAR_BUFFER_SIZE) - 1;

    uint32_t pos_in_buffer    =      head_pos % (CIRCULAR_BUFFER_SIZE);

    uint32_t increse_pos      =       currunt_data_pos - pos_in_buffer;

    ret = head_pos_increse(g_circular_buffer_irq_thread, &increse_pos);
    if (0x00 != ret)
    {
        log_e("head_pos_increse failed");
        return;    
    }

    ret = get_head_pos(g_circular_buffer_irq_thread, &head_pos);
    if (0x00 != ret)
    {
        log_d("get head pos error");
    }
    log_d("head_pos = [%d]", head_pos);

    pos_in_buffer = head_pos % (CIRCULAR_BUFFER_SIZE);
    log_d("pos_in_full_buffer = [%d]", pos_in_buffer); 
}

void uart_idle_irq_callback(uint32_t number_of_data)
{
    // log_d("uart_idle_irq = [%u]", number_of_data);

    uint32_t head_pos = 0;
    uint8_t  ret      = 0;

    ret = get_head_pos(g_circular_buffer_irq_thread, &head_pos);
    if (0x00 != ret)
    {

        return;    
    }

    uint32_t currunt_data_pos =                     number_of_data - 1;

    uint32_t pos_in_buffer    =      head_pos % (CIRCULAR_BUFFER_SIZE);

    uint32_t increse_pos      =                                      0;
    if (currunt_data_pos >= pos_in_buffer)
    {
        increse_pos           =       currunt_data_pos - pos_in_buffer;
    }
    else
    {
        increse_pos           = currunt_data_pos + CIRCULAR_BUFFER_SIZE
                                                       - pos_in_buffer;
    }

    ret = head_pos_increse(g_circular_buffer_irq_thread, &increse_pos);
    if (0x00 != ret)
    {
        log_e("head_pos_increse failed");
        return;    
    }

    //test
    ret = get_head_pos(g_circular_buffer_irq_thread, &head_pos);
    if (0x00 != ret)
    {
        log_d("get head pos error");
    }
    log_d("head_pos = [%d]", head_pos);

    pos_in_buffer = head_pos % (CIRCULAR_BUFFER_SIZE);
    log_d("pos_in_idle_buffer = [%d]", pos_in_buffer);    
}
