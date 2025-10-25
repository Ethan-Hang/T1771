/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "elog.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define  BUFFER_SIZE                                      1
#define  DMA_ADC_CPLT_INT                              0xA1
#define  BUFFER1_READY                                 0x01
#define  BUFFER2_READY                                 0x02
uint32_t *gp_buffer1            =                     NULL;
uint32_t *gp_buffer2            =                     NULL;
uint32_t g_DMA_Point            =                        0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
QueueHandle_t xQueue1;
QueueHandle_t xQueue2;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osThreadId_t adc_output_TaskHandle;
const osThreadAttr_t adc_output_attributes = {
    .name = "adc_outputTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
void adc_output_Task(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* creation of defaultTask */
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
    defaultTaskHandle = osThreadNew(adc_output_Task, NULL, &adc_output_attributes);
    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
    /* USER CODE BEGIN StartDefaultTask */
    /* Infinite loop */
    gp_buffer1 = (uint32_t *)malloc(BUFFER_SIZE * sizeof(uint32_t));
    gp_buffer2 = (uint32_t *)malloc(BUFFER_SIZE * sizeof(uint32_t));
    if (NULL == gp_buffer1)
    {
        printf("Failed to Create buffer1\r\n");
        return;
    }
    if (NULL == gp_buffer2)
    {
        printf("Failed to Create buffer2\r\n");
        return;
    }
    memset(gp_buffer1, 0xFF, sizeof(uint32_t) * BUFFER_SIZE);
    memset(gp_buffer2, 0xFF, sizeof(uint32_t) * BUFFER_SIZE);

    xQueue1 = xQueueCreate(1, sizeof(uint32_t));
    if (NULL == xQueue1)
    {
        printf("xQueueCreate1 failed\r\n");
        return;
    }
    xQueue2 = xQueueCreate(1, sizeof(uint32_t));
    if (NULL == xQueue2)
    {
        printf("xQueueCreate2 failed\r\n");
        return;
    }
    
    if (HAL_OK == HAL_ADC_Start_DMA(                &hadc1, 
                                                gp_buffer1,
                                              BUFFER_SIZE))
    {
        // printf("HAL_ADC_Start_DMA success\r\n");
    }
    else
    {
        printf("HAL_ADC_Start_DMA error\r\n");
        return;
    }

    uint32_t queue_data2 = 0xff;
    uint32_t queue2_pattern = BUFFER1_READY;

    for (;;)
    {
    if (pdPASS == xQueueReceive(                   xQueue1, 
                                            &(queue_data2), 
                                            portMAX_DELAY))
        {
            // printf("xQueueReceive success, data: [%lu]\r\n", queue_data2);
        }

        if (0 == g_DMA_Point)
        {
            printf("gp_buffer1 data = [%lu]\r\n", gp_buffer1[0]);

            queue2_pattern = BUFFER1_READY;
            if (pdPASS == xQueueSendToBack(        xQueue2, 
                                           &queue2_pattern, 
                                            portMAX_DELAY))
            {
                printf("xQueueSend buffer1 to Queue2 success\r\n");
            }

            if (HAL_OK != HAL_ADC_Start_DMA(        &hadc1, 
                                                gp_buffer2, 
                                              BUFFER_SIZE))
            {
                printf("HAL_ADC_Start_DMA error\r\n");
                return;
            }

            g_DMA_Point = 1;
        }
        else if (1 == g_DMA_Point)
        {
            printf("gp_buffer2 data = [%lu]\r\n", gp_buffer2[0]);

            queue2_pattern = BUFFER2_READY;
            if (pdPASS == xQueueSendToBack(         xQueue2, 
                                            &queue2_pattern, 
                                             portMAX_DELAY))
            {
                // printf("xQueueSend gp_buffer2 to Queue2 success\r\n");
            }

            if (HAL_OK != HAL_ADC_Start_DMA(        &hadc1, 
                                                gp_buffer1, 
                                              BUFFER_SIZE))
            {
                printf("HAL_ADC_Start_DMA error\r\n");
                return;
            }

            g_DMA_Point = 0;
        }
        osDelay(100);
    }
    /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void adc_output_Task(void *argument)
{
    /* USER CODE BEGIN adc_output_Task */
    
    // wait for queue create
    osDelay(10);

    printf("adc_out_put thread \r\n");

    uint32_t queue2_receive = BUFFER1_READY;
    for (;;)
    {
        if (pdPASS == xQueueReceive(               xQueue2, 
                                           &queue2_receive, 
                                            portMAX_DELAY))
        {
            printf("adc_output_Task QueueReceive success");
        }

        float voltage = 0.0f;
        if (queue2_receive == BUFFER1_READY)
        {
            voltage =  (float)*gp_buffer1 * 3.3f / 4095.0f;
            char str[30];
            sprintf(str, "%.3f V\r\n", voltage);
            elog_i("ADC buffer1 Voltage: ", str);
        }
        else if (queue2_receive == BUFFER2_READY)
        {
            voltage =  (float)*gp_buffer2 * 3.3f / 4095.0f;
            char str[30];
            sprintf(str, " %.3f V\r\n", voltage);
            elog_i("ADC buffer2 Voltage:", str);
        }
        osDelay(100);
    }
    /* USER CODE END adc_output_Task */
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hadc);
    /* NOTE : This function Should not be modified, when the callback is needed,
              the HAL_ADC_ConvCpltCallback could be implemented in the user file
     */

    // printf("Buffer1: [%lu]\r\n", gp_buffer1[0]);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint32_t dma_adc_pattern_cplt = DMA_ADC_CPLT_INT;

    if (pdPASS == xQueueSendToBackFromISR(         xQueue1,
                                     &dma_adc_pattern_cplt, 
                                &xHigherPriorityTaskWoken))
    {
        // printf("xQueueSend in irq success\r\n");
    }
    else
    {
        // printf("xQueueSend in irq failed\r\n");
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hadc);
    /* NOTE : This function Should not be modified, when the callback is needed,
              the HAL_ADC_ErrorCallback could be implemented in the user file
     */
    printf("ADC transfer error!\r\n");
}
/* USER CODE END Application */
