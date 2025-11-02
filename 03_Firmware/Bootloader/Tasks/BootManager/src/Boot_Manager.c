#include "Boot_Manager.h"

static pFunction jump_to_application;

void             Jump_To_App(void)
{
    uint32_t stack_addr = *(__IO uint32_t *) APP_FLASH_ADDR;

    // 1. check if stack address is valid
    if ((stack_addr & 0x2FFE0000) == 0x20000000)
    {
        // 2. disable all interrupts
        __disable_irq();
        NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t) (0x8000));
        RCC_DeInit();

        // 3. set main stack pointer
        __set_MSP(stack_addr);

        // 4. set jump address to reset handler address
        uint32_t jumpaddr   = *(__IO uint32_t *) (APP_FLASH_ADDR + 4);
        jump_to_application = (pFunction) jumpaddr;
        jump_to_application();
    }
}

int32_t       app_size = 0;
unsigned char IV[16]   = {0x36,
                          0x30,
                          0x32,
                          0x63,
                          0x36,
                          0x64,
                          0x30,
                          0x30,
                          0x39,
                          0x62,
                          0x65,
                          0x65,
                          0x35,
                          0x32,
                          0x33,
                          0x38};
unsigned char Key[32]  = {0x36, 0x30, 0x32, 0x63, 0x36, 0x64, 0x30, 0x30,
                          0x39, 0x62, 0x65, 0x65, 0x35, 0x32, 0x33, 0x38,
                          0x62, 0x35, 0x62, 0x39, 0x34, 0x62, 0x36, 0x34,
                          0x36, 0x37, 0x37, 0x62, 0x65, 0x34, 0x38, 0x38};
int8_t        Back_To_App(int32_t fl_size)
{
    uint32_t AppRunFlashDestination = APP_FLASH_ADDR;
    uint8_t *pu8_IV_IN_OUT          = IV;
    uint8_t *pu8_key256bit          = Key;
    uint8_t *pu8_temp = (uint8_t *) APP_BACK_FLASH_ADDR; //原始数据
    uint8_t  Temp[16];                                   //原密文数据缓存
    uint8_t *pTemp         = Temp;
    uint16_t readTime      = 0,
             readDataCount = 0; //读取数据再解密的次数，每次解密16个字节
    u32 AppSize            = 0; //升级包的大小

    // if (fl_size <= 0)
    // {
    //     return -1;
    // }

    /* */
    if ((app_size > (0x18010 - 1)) || (app_size < 0))
    {
        return -1;
    }

    memcpy(pTemp, pu8_temp, 16);
    pu8_temp += 16;
    Aes_IV_key256bit_Decode(pu8_IV_IN_OUT,
                            pTemp,
                            pu8_key256bit); //解析得到自定义内容+文件大小
    AppSize =
        (pTemp[15] << 24) + (pTemp[14] << 16) + (pTemp[13] << 8) + pTemp[12];

    /*计算需要解密多少次*/
    readDataCount = AppSize / 16;
    if (AppSize % 16 != 0)
    {
        readDataCount += 1;
    }

    //擦除运行区数据
    if (1 == Flash_erase(APP_FLASH_ADDR, AppSize))
    {
        return -1;
    }

    //读数据的总次数
    for (readTime = 0; readTime < readDataCount; readTime++)
    {
        //加密原文读取16个字节到临时区中
        pTemp = Temp;
        memcpy(pTemp, pu8_temp, 16);
        pu8_temp += 16;
        //解密数据
        Aes_IV_key256bit_Decode(pu8_IV_IN_OUT, pTemp, pu8_key256bit); 
        //解密后的数据存入App运行区中
        for (uint8_t j = 0; j < 16; j += 4)
        {
            Flash_Write(AppRunFlashDestination, *(uint32_t *) pTemp);
            if (*(uint32_t *) AppRunFlashDestination != *(uint32_t *) pTemp)
            {
                return -1;
            }
            AppRunFlashDestination += 4;
            pTemp                  += 4;
        }
    }
    return 0;
}

// int8_t Back_To_App(void)
// {
//     uint32_t flash_destiantion =      APP_FLASH_ADDR;
//     uint32_t back_flash_sourse = APP_BACK_FLASH_ADDR;

//     if (app_size > (APP_BACK_FLASH_ADDR - APP_FLASH_ADDR - 1) ||
//        (app_size < 0))
//     {
//         return -1;
//     }

//     for (uint32_t i = 0; i < app_size; i += 4)
//     {
//         Flash_Write(flash_destiantion, *(uint32_t *)back_flash_sourse);

//         if (*(uint32_t *)flash_destiantion !=
//             *(uint32_t *)back_flash_sourse)
//         {
//             return -1;
//         }
//         flash_destiantion += 4;
//         back_flash_sourse += 4;
//     }

//     return 0;
// }
