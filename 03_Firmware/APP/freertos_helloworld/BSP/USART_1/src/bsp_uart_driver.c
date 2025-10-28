/******************************************************************************
 * @file bsp_uart_driver.c
 * 
 * @par dependencies 
 * - bsp_uart_driver.h
 * - usart.h
 * - FreeRTOS.h
 * - mid_circular_buffer.h
 * - elog.h
 * 
 * @author Ethan-Hang
 * 
 * @brief UART driver implementation with DMA and circular buffer.
 *        Supports two reception modes: AB buffer switching and 
 *        circular buffer. Circular buffer mode is recommended.
 * 
 * Processing flow: 
 * 1. uart_driver_func() creates buffer and starts DMA reception
 * 2. DMA writes data directly to circular buffer
 * 3. Interrupt callbacks (half/full/idle) update head pointer
 * 4. Application reads data from buffer using tail pointer
 * 
 * @version V1.0 2025-10-28
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_uart_driver.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** @brief Signal code sent from interrupt to thread */
#define IRQ_SEND_TO_THREAD 0xA1A2A3A4

// #define USE_AB_BUFFER         /**< Enable AB buffer switching mode */
#define USE_CIRCULAR_BUFFER      /**< Enable circular buffer mode     */

#if defined (USE_AB_BUFFER)
/** @brief Buffer A identifier */
#define BUFFER_A 0
/** @brief Buffer B identifier */
#define BUFFER_B 1
/** @brief Current active buffer flag (A or B) */
uint8_t flag_AB      =  0 ;
/** @brief Reception buffer A (1 byte) */
uint8_t g_bufferA[1] = {0};
/** @brief Reception buffer B (1 byte) */
uint8_t g_bufferB[1] = {0};
#endif

#if defined (USE_CIRCULAR_BUFFER)
/** @brief External queue from another task */
extern QueueHandle_t       queue_irq_rec_A                    ;
/** @brief Circular buffer pointer for IRQ/thread communication */
static circular_buffer_t * g_circular_buffer_irq_thread = NULL;
/** @brief Queue for UART IRQ to thread communication */
static QueueHandle_t       queue_uart_irq_thread        = NULL;
/** @brief Temporary buffer for single byte reception (IT mode) */
uint8_t g_buffer[1] = {0};

#endif
//******************************** Defines **********************************//

//************************** Function Implementations ***********************//
/**
 * @brief UART driver task function for FreeRTOS.
 * 
 * Initializes UART reception with AB buffer or circular buffer 
 * method. Creates necessary queues and buffers, then enters an 
 * infinite loop to process received data notifications from ISR.
 * 
 * @param[in] argument : FreeRTOS task argument (unused).
 * 
 * @return None
 * 
 * */
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
            // log_i("front receive cmd from irq [%u]", receive_data);
        }
        
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

/**
 * @brief Get the circular buffer pointer used by UART driver.
 * 
 * Provides access to the circular buffer that stores received 
 * UART data. Other tasks can use this to read the received data.
 * 
 * @return circular_buffer_t* : Pointer to circular buffer or 
 *                               NULL if not initialized.
 * 
 * */
circular_buffer_t *get_circular_buffer(void)
{
    if (NULL == g_circular_buffer_irq_thread)
    {
        return NULL;
    }
    return g_circular_buffer_irq_thread;
}

/**
 * @brief UART receive complete callback (HAL library callback).
 * 
 * Called by HAL library when UART reception is complete.
 * Handles both AB buffer switching mode and circular buffer mode.
 * In circular buffer mode, restarts reception for next byte.
 * 
 * @param[in] huart : Pointer to UART handle structure.
 * 
 * @return None
 * 
 * */
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
    if (pdPASS == xQueueOverwriteFromISR(        queue_uart_irq_thread,
                                                       &send_to_thread,
                                            &xHigherPriorityTaskWoken))
    {

    }

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

/**
 * @brief UART receive event callback (HAL library extended callback).
 * 
 * Called when UART receive event occurs (e.g., idle line detection).
 * Currently not actively used but available for future extensions.
 * 
 * @param[in] huart : Pointer to UART handle structure.
 * @param[in] Size  : Number of bytes received.
 * 
 * @return None
 * 
 * */
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

/**
 * @brief DMA half-transfer complete interrupt callback.
 * 
 * Called when DMA has filled the first half of the circular 
 * buffer. Calculates the number of new bytes received and updates 
 * the buffer's head pointer. Notifies the driver task via queue.
 * 
 * Algorithm:
 * 1. Get current head position
 * 2. Calculate position in first half buffer
 * 3. Calculate increment needed to reach half-buffer boundary
 * 4. Update head pointer and notify task
 * 
 * @param[in] number_of_data : Number of bytes received 
 *                             (should be buffer_size/2).
 * 
 * @return None
 * 
 * */
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

    uint32_t currunt_data_pos =             (CIRCULAR_BUFFER_SIZE / 2);

    uint32_t pos_in_buffer    =  head_pos % (CIRCULAR_BUFFER_SIZE / 2);

    uint32_t increse_pos      =       currunt_data_pos - pos_in_buffer;

    ret = head_pos_increse(g_circular_buffer_irq_thread, &increse_pos);
    if (0x00 != ret)
    {
        log_e("head_pos_increse failed");
        return;    
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    if (pdPASS == xQueueOverwriteFromISR(        queue_uart_irq_thread,
                                                       &send_to_thread,
                                            &xHigherPriorityTaskWoken))
    {
        
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

/**
 * @brief DMA full-transfer complete interrupt callback.
 * 
 * Called when DMA has filled the entire circular buffer and wraps 
 * around. Calculates the number of new bytes received and updates 
 * the buffer's head pointer. Notifies the driver task via queue.
 * 
 * Algorithm:
 * 1. Get current head position
 * 2. Calculate position in full buffer
 * 3. Calculate increment needed to reach full-buffer boundary
 * 4. Update head pointer and notify task
 * 
 * @param[in] number_of_data : Number of bytes received 
 *                             (should be full buffer_size).
 * 
 * @return None
 * 
 * */
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

    uint32_t currunt_data_pos =                 (CIRCULAR_BUFFER_SIZE);

    uint32_t pos_in_buffer    =      head_pos % (CIRCULAR_BUFFER_SIZE);

    uint32_t increse_pos      =       currunt_data_pos - pos_in_buffer;

    ret = head_pos_increse(g_circular_buffer_irq_thread, &increse_pos);
    if (0x00 != ret)
    {
        log_e("head_pos_increse failed");
        return;    
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    if (pdPASS == xQueueOverwriteFromISR(        queue_uart_irq_thread,
                                                       &send_to_thread,
                                            &xHigherPriorityTaskWoken))
    {
        
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

/**
 * @brief UART idle line interrupt callback.
 * 
 * Called when UART idle condition is detected (no data for a 
 * period). This indicates the end of a data packet. Calculates 
 * the actual number of bytes received and updates the buffer's 
 * head pointer. Handles buffer wrap-around correctly.
 * 
 * Algorithm:
 * 1. Get current head position
 * 2. Calculate current position in buffer
 * 3. Calculate increment (handle wrap-around case)
 * 4. Update head pointer and notify task
 * 
 * @param[in] number_of_data : Actual number of bytes received 
 *                             before idle.
 * 
 * @return None
 * 
 * */
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

    uint32_t currunt_data_pos =                         number_of_data;

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

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    if (pdPASS == xQueueOverwriteFromISR(        queue_uart_irq_thread,
                                                       &send_to_thread,
                                            &xHigherPriorityTaskWoken))
    {
        
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
//************************** Function Implementations ***********************//
