#include "Boot_Manager.h"

static pFunction jump_to_application;
uint8_t          au8_test[1024];
uint8_t          Mem_Read_buffer[4096];
void             Jump_To_App(void)
{
    uint32_t stack_addr = *(__IO uint32_t *)APP_FLASH_ADDR;

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
        uint32_t jumpaddr   = *(__IO uint32_t *)(APP_FLASH_ADDR + 4);
        jump_to_application = (pFunction)jumpaddr;
        jump_to_application();
    }
}

int32_t       app_size = 0;
unsigned char IV[16]   = {0x31, 0X32, 0x31, 0X32, 0x31, 0X32, 0x31, 0X32,
                          0x31, 0X32, 0x31, 0X32, 0x31, 0X32, 0x31, 0X32};
unsigned char Key[32]  = {0x31, 0X32, 0x31, 0X32, 0x31, 0X32, 0x31, 0X32,
                          0x31, 0X32, 0x31, 0X32, 0x31, 0X32, 0x31, 0X32,
                          0x31, 0X32, 0x31, 0X32, 0x31, 0X32, 0x31, 0X32,
                          0x31, 0X32, 0x31, 0X32, 0x31, 0X32, 0x31, 0X32};
int8_t        Back_To_App(int32_t fl_size)
{
    uint8_t  Temp[16];               // 原密文数据缓存
    uint8_t  wirteTime          = 0; // 一个解析包写入次数
    uint16_t readTime           = 0,
             readDataCount      = 0; // 读取数据再解密的次数（每次解密16个字节）
    uint32_t AppSize            = 0; // 升级包的大小
    // uint32_t FlashDestination=APP_FLASH_ADDR;
    uint16_t Read_Memory_Size   = 0;
    uint32_t Read_Memory_index  = 0;
    uint8_t *puint8_t_IV_IN_OUT = IV;
    uint8_t *puint8_t_key256bit = Key;
    uint32_t RamSource          = 0;
    uint32_t AppRunFlashDestination = APP_FLASH_ADDR;
    if (fl_size <= 0)
    {
        return -1;
    }
    if ((app_size > (0x18010 - 1)) || (app_size < 0))
    {
        return -1;
    }
    // 先读一帧，用来解析头文件格式
    W25Q64_ReadData(BLOCK_2, Mem_Read_buffer, &Read_Memory_Size);
    if (Read_Memory_Size >= 16)
    {
        memcpy(Temp, Mem_Read_buffer, 16);
        Aes_IV_key256bit_Decode(
            puint8_t_IV_IN_OUT, Temp,
            puint8_t_key256bit); // 解析得到自定义内容+文件大小
        AppSize =
            (Temp[15] << 24) + (Temp[14] << 16) + (Temp[13] << 8) + Temp[12];
        log_d("AppSize=%d", AppSize);

        // 计算升级包读取次数
        readDataCount = AppSize / 16;
        if (AppSize % 16 != 0)
        {
            readDataCount += 1;
        }
        Read_Memory_index += 16;
    }

    // 数据帧
    // 将待写入区的内容擦除
    uint8_t flash_erase_state = Flash_erase(AppRunFlashDestination, AppSize);
    if (flash_erase_state == 0)
    {
        for (readTime = 0; readTime < readDataCount; readTime++)
        {
            // 判断下当前buffer下的数据是否读取完毕
            if (Read_Memory_index == Read_Memory_Size)
            {
                if (2 == W25Q64_ReadData(BLOCK_2, Mem_Read_buffer,
                                         &Read_Memory_Size))
                {
                    log_d("Write_Flash_After_AES_Decode read extern buffer "
                          "error");
                    return -1;
                }
                Read_Memory_index = 0;
            }
            // 拷贝16个数据
            memcpy(Temp, Mem_Read_buffer + Read_Memory_index, 16);
            Read_Memory_index += 16;
            // 解析16个数据
            Aes_IV_key256bit_Decode(puint8_t_IV_IN_OUT, Temp,
                                    puint8_t_key256bit); // 解析

            RamSource = (uint32_t)Temp;
            for (wirteTime = 0; wirteTime < 4; wirteTime++)
            {
                Flash_Write(AppRunFlashDestination, *(uint32_t *)RamSource);
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

int8_t ExA_To_ExB_AES(int32_t fl_size)
{
    uint8_t  Temp[16]; // 原密文数据缓存
    // uint8_t  wirteTime          = 0; // 一个解析包写入次数
    uint16_t readTime           = 0,
             readDataCount      = 0; // 读取数据再解密的次数（每次解密16个字节）
    uint32_t AppSize            = 0; // 升级包的大小
    // uint32_t FlashDestination=ApplicationAddress;
    uint16_t Read_Memory_Size   = 0;
    uint32_t Read_Memory_index  = 0;
    uint8_t *puint8_t_IV_IN_OUT = IV;
    uint8_t *puint8_t_key256bit = Key;
    // uint32_t RamSource          = 0;
    if (fl_size <= 0)
    {
        return -1;
    }
    if ((app_size > (0x18010 - 1)) || (app_size < 0))
    {
        return -1;
    }
    // 先读一帧，用来解析头文件格式
    W25Q64_ReadData(BLOCK_1, Mem_Read_buffer, &Read_Memory_Size);
    if (Read_Memory_Size >= 16)
    {
        memcpy(Temp, Mem_Read_buffer, 16);
        Aes_IV_key256bit_Decode(
            puint8_t_IV_IN_OUT, Temp,
            puint8_t_key256bit); // 解析得到自定义内容+文件大小
        AppSize =
            (Temp[15] << 24) + (Temp[14] << 16) + (Temp[13] << 8) + Temp[12];
        log_d("AppSize=%d", AppSize);

        // 计算升级包读取次数
        readDataCount = AppSize / 16;
        if (AppSize % 16 != 0)
        {
            readDataCount += 1;
        }
        Read_Memory_index += 16;
    }
    else
    {
        return -1;
    }

    // 擦除外部flash比较耗时
    for (readTime = 0; readTime < readDataCount; readTime++)
    {
        // 判断下当前buffer下的数据是否读取完毕
        if (Read_Memory_index == Read_Memory_Size)
        {
            if (2 ==
                W25Q64_ReadData(BLOCK_1, Mem_Read_buffer, &Read_Memory_Size))
            {
                log_d("Write_Flash_After_AES_Decode read extern buffer error");
                return -1;
            }
            Read_Memory_index = 0;
        }
        // 拷贝16个数据
        memcpy(Temp, Mem_Read_buffer + Read_Memory_index, 16);
        Read_Memory_index += 16;
        // 解析16个数据
        Aes_IV_key256bit_Decode(puint8_t_IV_IN_OUT, Temp,
                                puint8_t_key256bit); // 解析

        W25Q64_WriteData(BLOCK_2, Temp, 16);
    }
    W25Q64_WriteData_End(BLOCK_2);
    log_d("Write_Flash_After_AES_Decode end");
    return 0;
}

int8_t ExA_To_App(void)
{
    uint32_t FlashDes        = APP_FLASH_ADDR;
    uint32_t flashsize       = 0;
    uint8_t  Read_dataState  = 0;
    uint16_t Read_Memorysize = 0;
    uint32_t RamSource       = 0;
    uint16_t writeTime       = 0;
    /*擦除本地flash数据*/
    flashsize                = Read_BlockSize(BLOCK_1);
    if (1 == Flash_erase(FlashDes, flashsize))
    {
        return -1;
    }
    else
    {
        for (;;)
        {
            Read_dataState =
                W25Q64_ReadData(BLOCK_1, Mem_Read_buffer, &Read_Memorysize);
            if (1 == Read_dataState)
            {
                // 数据读完退出
                break;
            }
            else if (2 == Read_dataState)
            {
                // 外部flash数据读取有问题
                break;
            }
            else
            {
                RamSource = (uint32_t)Mem_Read_buffer;
                // 循环搬运flash数据
                for (writeTime = 0; writeTime < (Read_Memorysize / 4);
                     writeTime++)
                {
                    Flash_Write(FlashDes, RamSource);
                    FlashDes  += 4;
                    RamSource += 4;
                }
            }
        }
        return flashsize;
    }
}

int8_t ExB_To_App(void)
{
    uint32_t FlashDes        = APP_FLASH_ADDR;
    uint32_t flashsize       = 0;
    uint8_t  Read_dataState  = 0;
    uint16_t Read_Memorysize = 0;
    uint32_t RamSource       = 0;
    uint16_t writeTime       = 0;
    /*擦除本地flash数据*/
    flashsize                = Read_BlockSize(BLOCK_2);
    if (1 == Flash_erase(FlashDes, flashsize))
    {
        return -1;
    }
    else
    {
        for (;;)
        {
            Read_dataState =
                W25Q64_ReadData(BLOCK_2, Mem_Read_buffer, &Read_Memorysize);
            if (1 == Read_dataState)
            {
                // 数据读完退出
                break;
            }
            else if (2 == Read_dataState)
            {
                // 外部flash数据读取有问题
                break;
            }
            else
            {
                RamSource = (uint32_t)Mem_Read_buffer;
                // 循环搬运flash数据
                for (writeTime = 0; writeTime < (Read_Memorysize / 4);
                     writeTime++)
                {
                    Flash_Write(FlashDes, *(uint32_t *)RamSource);
                    FlashDes  += 4;
                    RamSource += 4;
                }
            }
        }
        return flashsize;
    }
}

int8_t App_To_ExA(int32_t fl_size)
{
    uint32_t FlashDes = APP_FLASH_ADDR;
    if ((fl_size > (0x18010 - 1)) || (fl_size < 0))
    {
        return -1;
    }

    Erase_Flash_Block(BLOCK_1);
    W25Q64_WriteData(BLOCK_1, (uint8_t *)FlashDes, fl_size);
    W25Q64_WriteData_End(BLOCK_1);
    return 0;
}

void System_SoftwareReset(void)
{
    __NVIC_SystemReset();
}

void OTA_StateManager(void)
{
    uint8_t  t_u8_ota_state = NO_APP_UPDATE;
    uint32_t t_u32_appsize  = 0;
    uint32_t file_size      = 0;

    ee_ReadBytes(&t_u8_ota_state, 0x00, 1);

    switch (t_u8_ota_state)
    {
        case NO_APP_UPDATE:
            if (Key_Scan())
            {
                file_size = Ymodem_Receive(au8_test);

                if (0 == ExA_To_ExB_AES(file_size))
                {
                    /*3.当前App的数据搬运到外部flash的A区
                     * ，增加一个代码回退功能*/
                    // 读当前App的大小
                    ee_ReadBytes((uint8_t *)&t_u32_appsize, 0x05, 4);
                    App_To_ExA(t_u32_appsize);
                    ExB_To_App();
					
					t_u8_ota_state = APP_FIRST_CHECK_START;
					ee_WriteBytes(&t_u8_ota_state, 0x00, 1);
					// 执行软件复位
					System_SoftwareReset();
                }
            }
            else
            {
                log_a("Boot download failed");
                Jump_To_App();
            }
            break;

        case APP_DOWNLOADING:
            log_a("App dowload failed");
            Jump_To_App();
            // 1.跳转失败，下载新App
            log_a("No Valid App");

            file_size = Ymodem_Receive(au8_test);
            /*2.解密数据拷贝到外部flashB区中*/
            if (0 == ExA_To_ExB_AES(file_size))
            {
                /*3.当前App的数据搬运到外部flash的A区 ，增加一个代码回退功能*/
                // 读当前App的大小
                ee_ReadBytes((uint8_t *)&t_u32_appsize, 0x05, 4);
                App_To_ExA(t_u32_appsize);
                /*4.外部flashB区的数据搬运到内部App区域中*/
                ExB_To_App();

                t_u8_ota_state = APP_FIRST_CHECK_START;
                ee_WriteBytes(&t_u8_ota_state, 0x00, 1);
                // 执行软件复位
                System_SoftwareReset();
                /*5.执行跳转逻辑*/
                // Jump_To_App();
                /*6.如果运行到这一步，说明数据无效，把外部A区的数据搬运到App中*/
                // ExA_To_App();
                /*再执行一次跳转*/
                // Jump_To_App();
            }
            else
            {
                log_a("Boot dowload failed");
                Jump_To_App();
            }

        case APP_DOWNLOAD_COMPLETE:
            // 1.读取当前需要更新的App的大小
            ee_ReadBytes((uint8_t *)&t_u32_appsize, 0x01, 4);
            // 2.App大小更新到内部flash存储数据量的管理结构体中
            SetBlockParmeter(BLOCK_1, t_u32_appsize);
            /*2.解密数据拷贝到外部flashB区中*/
            if (0 == ExA_To_ExB_AES(t_u32_appsize))
            {
                /*3.当前App的数据搬运到外部flash的A区 ，增加一个代码回退功能*/
                // 读当前App的大小
                ee_ReadBytes((uint8_t *)&t_u32_appsize, 0x05, 4);
                App_To_ExA(t_u32_appsize);
                /*4.外部flashB区的数据搬运到内部App区域中*/
                ExB_To_App();
                t_u8_ota_state = APP_FIRST_CHECK_START;
                ee_WriteBytes(&t_u8_ota_state, 0x00, 1);
                // 执行软件复位
                /*软件复位*/
                System_SoftwareReset();
            }
            else
            {
                log_a("Boot dowload failed");
                Jump_To_App();
                t_u8_ota_state = NO_APP_UPDATE;
                ee_WriteBytes(&t_u8_ota_state, 0x00, 1);
                /*软件复位*/
                System_SoftwareReset();
            }
            break;

        case APP_FIRST_CHECK_START:
            t_u8_ota_state = APP_FIRST_CHECKING;
            ee_WriteBytes(&t_u8_ota_state, 0x00, 1);
            /*开看门狗*/
            IWDG_Init(IWDG_Prescaler_64, 3000);
            /*5.执行跳转逻辑*/
            Jump_To_App();
            break;

        case APP_FIRST_CHECKING:
            // App校验无效
            t_u8_ota_state = NO_APP_UPDATE;
            ee_WriteBytes(&t_u8_ota_state, 0x00, 1);
            /*软件复位*/
            System_SoftwareReset();
            /*如果运行到这一步，说明数据无效，把外部A区的数据搬运到App中*/
            // ExA_To_App();
            /*再执行一次跳转*/
            // Jump_To_App();
            break;
        default:
            break;
    }
}
