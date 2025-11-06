/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file Ota_task.c
 *
 * @par dependencies
 * - Ota_task.h
 *
 * @author Jack | R&D Dept. | EternalChip 立芯嵌入式
 *
 * @brief Functions related to reading and writing in the chip's flash area.
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2024-09-13
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "SWC_OTA.h"
/* Private typedef -----------------------------------------------------------*/
osThreadId_t         OTA_taskHandle;
const osThreadAttr_t OTA_task_attributes = {
    .name       = "OTA_task",
    .stack_size = 512 * 4,
    .priority   = (osPriority_t)osPriorityNormal1,
};

osThreadId_t         DownloadAppData_taskHandle;
const osThreadAttr_t DownloadAppData_task_attributes = {
    .name       = "DownloadAppData_task",
    .stack_size = 512 * 4,
    .priority   = (osPriority_t)osPriorityNormal1,
};

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static E_Ota_State_t s_e_Ota_State              = WaitReqDownload;
static uint8_t       s_au8_OtaCmd[4]            = {0};
uint8_t              g_au8_YmodemRecAB[2][1030] = {0};
/* extern variables ---------------------------------------------------------*/
extern QueueHandle_t Q_YmodemReclength;
QueueHandle_t        Queue_AppDataBuffer;
SemaphoreHandle_t    Semaphore_ExtFlashState;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static int8_t        Key_Scan(void);
void                 SoftReset(void);
void                 download_appdata_task_runnable(void *argument);
/* extern variables ---------------------------------------------------------*/
extern int32_t       packet_length;
void                 ota_task_runnable(void *argument)
{
    /* USER CODE BEGIN key_task_runnable */
    uint8_t       t_u8_rec_length         = 0;
    const uint8_t t_au8_AckCmd[3]         = {0x44, 0x55, 0x66};
    uint8_t       t_u8_otastate           = 0x00;
    int32_t       t_int32_app_data_length = 0;

    ee_WriteBytes(&t_u8_otastate, 0x00, 1);
    /* Infinite loop */
    for (;;)
    {
        switch (s_e_Ota_State)
        {
            case WaitReqDownload:
                HAL_UARTEx_ReceiveToIdle_DMA(&huart1, 
                                             s_au8_OtaCmd,
                                             4);
                
                xQueueReceive(Q_YmodemReclength, 
                              &t_u8_rec_length,
                              portMAX_DELAY);
                if (3 == t_u8_rec_length)
                {
                    if (0x11 == s_au8_OtaCmd[0] &&
                        0x22 == s_au8_OtaCmd[1] &&
                        0x33 == s_au8_OtaCmd[2])
                    {
                        s_e_Ota_State = OtaDownload;
                        DownloadAppData_taskHandle =
                            osThreadNew(download_appdata_task_runnable, NULL,
                                        &DownloadAppData_task_attributes);
                        Queue_AppDataBuffer = xQueueCreate(1, 
                                                           sizeof(uint32_t *));
                        Semaphore_ExtFlashState = xSemaphoreCreateMutex();

                        t_u8_otastate = 0x01;
                        ee_WriteBytes(&t_u8_otastate, 0x00, 1);
                    }
                    else
                    {
                        memset(s_au8_OtaCmd, 0, 4);
                    }
                }
                else
                {
                    memset(s_au8_OtaCmd, 0, 4);
                }
                break;
            case OtaDownload:
            {

                t_int32_app_data_length =Ymodem_Receive(g_au8_YmodemRecAB);
                if (t_int32_app_data_length > 0)
                {
                    s_e_Ota_State = WaitReqUpdate;

                    HAL_UART_Transmit(&huart1, 
                                      (uint8_t *)t_au8_AckCmd,
                                      3,
                                      HAL_MAX_DELAY);
                }
                else
                {
                    s_e_Ota_State = WaitReqDownload;
                    memset(s_au8_OtaCmd, 0, 4);
                }

                xSemaphoreTake(Semaphore_ExtFlashState, portMAX_DELAY);
                xSemaphoreGive(Semaphore_ExtFlashState);

                W25Q64_WriteData_End();

                
                vTaskDelete(DownloadAppData_taskHandle);
                vQueueDelete(Queue_AppDataBuffer);
                vSemaphoreDelete(Semaphore_ExtFlashState);

                t_u8_otastate = 0x02;
                ee_WriteBytes(&t_u8_otastate, 0x00, 1);

                // Test
                // uint8_t t_u8_readstate = 0;
                // if (0 == ee_ReadBytes(&t_u8_readstate, 0x00, 1))
                // {
                //     HAL_UART_Transmit(&huart1, "error", 6, 1000);
                // }
                // HAL_UART_Transmit(&huart1, &t_u8_readstate, 1, 1000);

                ee_WriteBytes((uint8_t *)&t_int32_app_data_length,
                              0x01,
                              sizeof(t_int32_app_data_length));

                // Test
                // uint8_t t_au8_readlength[4] = {0};
                // if (0 == ee_ReadBytes(t_au8_readlength, 0x01, 4))
                // {
                //     HAL_UART_Transmit(&huart1, "error", 6, 1000);
                // }
                // HAL_UART_Transmit(&huart1, t_au8_readlength, 4, 1000);

                break;
            }
            case WaitReqUpdate:
                HAL_UARTEx_ReceiveToIdle_DMA(&huart1,
                                             s_au8_OtaCmd,
                                             4);
                
                xQueueReceive(Q_YmodemReclength,
                              &t_u8_rec_length,
                              portMAX_DELAY);
                if (3 == t_u8_rec_length)
                {
                    if (0x77 == s_au8_OtaCmd[0] &&
                        0x88 == s_au8_OtaCmd[1] &&
                        0x99 == s_au8_OtaCmd[2])
                    {
                        s_e_Ota_State = OtaEnd;
                    }
                    else
                    {
                        memset(s_au8_OtaCmd, 0, 4);
                    }
                }
                else
                {
                    memset(s_au8_OtaCmd, 0, 4);
                }
                break;
            case OtaEnd:
                if (1 == Key_Scan())
                {
                    SoftReset();
                }
                else
                {
                    s_e_Ota_State = WaitReqDownload;
                }
                break;
            default:
                break;
        }
    }
    /* USER CODE END key_task_runnable */
}

static int8_t Key_Scan(void)
{
    uint16_t t_u16_cnt = 0;

    while (t_u16_cnt < 400)
    {
        if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin))
        {
            osDelay(10);
            if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin))
            {
                return 1;
            }
        }

        osDelay(50);
        t_u16_cnt++;
    }

    return -1;
}

void SoftReset(void)
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}

void download_appdata_task_runnable(void *argument)
{
    uint32_t * pu32_size = NULL;
    uint8_t  * pu8_data  = NULL;

    xQueueReceive(Queue_AppDataBuffer,
                  &pu32_size,
                  portMAX_DELAY);
    
    xSemaphoreGive(Semaphore_ExtFlashState);
    for (;;)
    {
        xQueueReceive(Queue_AppDataBuffer,
                      &pu8_data,
                      portMAX_DELAY);
        xSemaphoreTake(Semaphore_ExtFlashState, portMAX_DELAY);
        if (NULL == pu8_data)
        {
            continue;
        }

        W25Q64_WriteData(pu8_data, (uint32_t)packet_length);

        xSemaphoreGive(Semaphore_ExtFlashState);
    }
}
