#ifndef BOOT_MANAGER_H
#define BOOT_MANAGER_H

#include "main.h"
#include "elog.h"

typedef void (* pFunction)(void);
#define APP_FLASH_ADDR 0x8008000

void Jump_To_App(void);

#endif /* BOOT_MANAGER_H */
