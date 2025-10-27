#include "uart_parse_task.h"
#include "mid_circular_buffer.h"
#include "bsp_uart_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "elog.h"
#include <string.h>
#include "queue.h"

uint8_t g_buffer1[1] = {0};
uint8_t g_buffer2[1] = {0};

QueueHandle_t             queue_irq_rec_A         = NULL;
static circular_buffer_t *g_circular_buffer_rec_A = NULL;

void uart_rec_A_func(void *argument)
{
    /* USER CODE BEGIN uart_rec_A_func */
    log_i("uart_rec_A_func is running...");

    queue_irq_rec_A = xQueueCreate(1, 4);
    if (NULL == queue_irq_rec_A)
    {
        log_e("queue_irq_rec_A create failed");
        return;
    }
    else
    {
        log_i("queue_irq_rec_A create success");
    }

    /* Infinite loop */
    for (;;)
    {
        uint32_t receive_data = 0;
        if (pdPASS == xQueuePeek(          queue_irq_rec_A,
                                             &receive_data,
                                            portMAX_DELAY))
        {
            // log_i("uart_rec_A_func received data [%u]", receive_data);
        }

        if (FRONT_SEND_TO_END == receive_data)
        {
            g_circular_buffer_rec_A = get_circular_buffer();

            while (0x00 != buffer_is_empty(g_circular_buffer_rec_A))
            {
                uint32_t temp_data = 0;
                if (0x00 == get_data(g_circular_buffer_rec_A, &temp_data))
                {
                    // log_d("uart_rec_A_func got data: %c", temp_data);
                }
                osDelay(2);

                static uint8_t data_counter        =                  0;
                static uint8_t temp_data_array[20] =             {0x00};
                static uint32_t status             = FRAME_NOT_DETECTED;
                switch (status)
                {
                case FRAME_NOT_DETECTED:
                    if (FRAME_HEAD_FLAG == temp_data)
                    {
                        status = FRAME_HEAD;
                        log_i("Data packet start");
                    }

                case FRAME_HEAD:
                    if (FRAME_END_FLAG != temp_data)
                    {
                        data_counter++;
                        temp_data_array[data_counter - 1] = temp_data;
                    }
                    else
                    {
                        uint32_t data_sum = temp_data_array[data_counter - 1];
                        uint32_t data_sum_temp = 0;

                        for (uint8_t i = 1; i < (data_counter - 1); i++)
                        {
                            data_sum_temp += temp_data_array[i];
                        }
                        log_i("calculated sum: [%u], received sum: [%u]", 
                                                data_sum_temp, data_sum);

                        if (data_sum_temp == data_sum)
                        {
                            for (uint8_t j = 1; j < (data_counter - 1); j++)
                            {
                                log_i("Valid data received: [%x]", temp_data_array[j]);
                            }
                        }
                        else
                        {
                            log_i("InValid data received!");
                        }

                        memset(temp_data_array, 0, sizeof(temp_data_array));
                        data_counter = 0;
                    }
                    break;

                default:
                    break;
                }
            }
        }    
        osDelay(100);
    }
    /* USER CODE END uart_rec_A_func */
}
