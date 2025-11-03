#ifndef BOOT_MANAGER_H
#define BOOT_MANAGER_H

#include "elog.h"
#include "flash.h"
#include "main.h"
#include "AES.h"
#include "w25qxx_Handler.h"

typedef void (*pFunction)(void);
#define APP_FLASH_ADDR      0x8008000
#define APP_BACK_FLASH_ADDR 0x8020000

void   Jump_To_App(void);
int8_t Back_To_App(int32_t fl_size);
// int8_t Back_To_App(void);


#endif /* BOOT_MANAGER_H */
