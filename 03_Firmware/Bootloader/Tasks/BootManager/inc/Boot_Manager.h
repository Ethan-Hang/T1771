#ifndef BOOT_MANAGER_H
#define BOOT_MANAGER_H

#include "gpio.h"
#include "elog.h"
#include "flash.h"
#include "main.h"
#include "AES.h"
#include "w25qxx_Handler.h"
#include "AT24Cxx_Driver.h"
#include "ymodem.h"
// #include ""

typedef void (*pFunction)(void);
#define APP_FLASH_ADDR        0x8008000
#define APP_BACK_FLASH_ADDR   0x8020000
#define NO_APP_UPDATE         0x00
#define APP_DOWNLOADING       0x01
#define APP_DOWNLOAD_COMPLETE 0x02

void   Jump_To_App(void);
int8_t Back_To_App(int32_t fl_size);
// int8_t Back_To_App(void);
void   OTA_StateManager(void);
int8_t App_To_ExA(int32_t fl_size);
int8_t ExB_To_App(void);
int8_t ExA_To_App(void);
int8_t ExA_To_ExB_AES(int32_t fl_size);


#endif /* BOOT_MANAGER_H */
