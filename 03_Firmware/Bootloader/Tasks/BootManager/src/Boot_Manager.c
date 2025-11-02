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

        // 3. set main stack pointer
        __set_MSP(stack_addr);
        
        // 4. set jump address to reset handler address
        uint32_t jumpaddr   = *(__IO uint32_t *) (APP_FLASH_ADDR + 4);
        jump_to_application =                    (pFunction) jumpaddr;
        jump_to_application();
    }
}

int32_t app_size = 0;
int8_t Back_To_App(void)
{
    uint32_t flash_destiantion =      APP_FLASH_ADDR;
    uint32_t back_flash_sourse = APP_BACK_FLASH_ADDR;

    if (app_size > (APP_BACK_FLASH_ADDR - APP_FLASH_ADDR - 1) || 
       (app_size < 0))
    {
        return -1;
    }

    for (uint32_t i = 0; i < app_size; i += 4)
    {
        Flash_Write(flash_destiantion, *(uint32_t *)back_flash_sourse);

        if (*(uint32_t *)flash_destiantion != 
            *(uint32_t *)back_flash_sourse)
        {
            return -1;
        }
        flash_destiantion += 4;
        back_flash_sourse += 4;
    }

    return 0;
}
