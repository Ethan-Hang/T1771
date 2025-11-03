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
uint8_t       Mem_Read_buffer[4096];
int8_t        Back_To_App(int32_t fl_size)
{
    uint8_t  Temp[16];              //原密文数据缓存
    uint8_t  wirteTime         = 0; //一个解析包写入次数
    uint16_t readTime          = 0,
             readDataCount     = 0; //读取数据再解密的次数（每次解密16个字节）
    uint32_t AppSize           = 0; //升级包的大小
    //uint32_t FlashDestination=APP_FLASH_ADDR;
    uint16_t Read_Memory_Size  = 0;
    uint32_t Read_Memory_index = 0;
    uint8_t *pu8_IV_IN_OUT     = IV;
    uint8_t *pu8_key256bit     = Key;
    uint32_t RamSource         = 0;
    uint32_t AppRunFlashDestination = APP_FLASH_ADDR;
    if (fl_size <= 0)
    {
        return -1;
    }
    if ((app_size > (0x18010 - 1)) || (app_size < 0))
    {
        return -1;
    }
    //先读一帧，用来解析头文件格式
    W25Q64_ReadData(Mem_Read_buffer, &Read_Memory_Size);
    if (Read_Memory_Size >= 16)
    {
        memcpy(Temp, Mem_Read_buffer, 16);
        Aes_IV_key256bit_Decode(pu8_IV_IN_OUT,
                                Temp,
                                pu8_key256bit); //解析得到自定义内容+文件大小
        AppSize =
            (Temp[15] << 24) + (Temp[14] << 16) + (Temp[13] << 8) + Temp[12];
        log_d("AppSize=%d", AppSize);

        //计算升级包读取次数
        readDataCount = AppSize / 16;
        if (AppSize % 16 != 0)
        {
            readDataCount += 1;
        }
        Read_Memory_index += 16;
    }

    //数据帧
    //将待写入区的内容擦除
    uint8_t flash_erase_state = Flash_erase(AppRunFlashDestination, AppSize);
    if (flash_erase_state == 0)
    {
        for (readTime = 0; readTime < readDataCount; readTime++)
        {
            //判断下当前buffer下的数据是否读取完毕
            if (Read_Memory_index == Read_Memory_Size)
            {
                if (2 == W25Q64_ReadData(Mem_Read_buffer, &Read_Memory_Size))
                {
                    log_d("Write_Flash_After_AES_Decode read extern buffer "
                          "error");
                    return -1;
                }
                Read_Memory_index = 0;
            }
            //拷贝16个数据
            memcpy(Temp, Mem_Read_buffer + Read_Memory_index, 16);
            Read_Memory_index += 16;
            //解析16个数据
            Aes_IV_key256bit_Decode(pu8_IV_IN_OUT, Temp, pu8_key256bit); //解析

            RamSource = (uint32_t) Temp;
            for (wirteTime = 0; wirteTime < 4; wirteTime++)
            {
                Flash_Write(AppRunFlashDestination, *(uint32_t *) RamSource);
                AppRunFlashDestination += 4;
                RamSource              += 4;
            }
        }
        log_d("Write_Flash_After_AES_Decode end");
        return 0;
    }
    else
    {
        log_d("Write_Flash_After_AES_Decode Error");
        return -1;
    }
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
