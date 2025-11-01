#include "Boot_Manager.h"

static pFunction jump_to_application;

void Jump_To_App(void)
{
    uint32_t stack_addr = *(__IO uint32_t *) APP_FLASH_ADDR;

    // 1. check if stack address is valid
    if ((stack_addr & 0x2FFE0000) == 0x20000000)
    {
        // 2. disable all interrupts
        __disable_irq();
        NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)(0x8000));
        RCC_DeInit();

        // 3. set jump address to reset handler address
        uint32_t jumpaddr   = *(__IO uint32_t *) (APP_FLASH_ADDR + 4);
        jump_to_application =                    (pFunction) jumpaddr;

        // 4. set main stack pointer
        __set_MSP(stack_addr);
        jump_to_application();
    }
}
