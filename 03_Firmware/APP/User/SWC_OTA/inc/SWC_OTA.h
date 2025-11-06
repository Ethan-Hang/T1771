/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file Ota_task.h
 *
 * @par dependencies
 * - Ota_task.h
 *
 * @author Jack | R&D Dept. | EternalChip 立芯嵌入式
 *
 * @brief Execute the Ota upgrade process
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SWC_OTA_H
#define __SWC_OTA_H
/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>

#include "main.h"
#include "gpio.h"

#include "cmsis_os.h"
#include "Freertos.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "ymodem.h"
#include "W25Q_Handler.h"
#include "AT24Cxx_Driver.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
    WaitReqDownload = 0,
    OtaDownload     = 1,
    WaitReqUpdate   = 2,
    OtaEnd          = 3,
} E_Ota_State_t;
/* Exported variable --------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define BUFFER_SIZE 1030
/* Exported functions ------------------------------------------------------- */
extern void ota_task_runnable(void *argument);
#endif
