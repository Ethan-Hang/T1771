/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Boot_Manager.h"
#include "Debug.h"
#include "Ymodem.h"
#include "elog.h"
#include "flash.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "spi.h"
#include "w25qxx_Handler.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t uwTimingDelay;
RCC_ClocksTypeDef    RCC_Clocks;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int32_t              fil_size = 0;
uint8_t              au8_test[1024];
int                  main(void)
{
    /* Enable Clock Security System(CSS): this will generate an NMI exception
     when HSE clock fails *****************************************************/
    RCC_ClockSecuritySystemCmd(ENABLE);

    /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files before to branch to application main.
       To reconfigure the default setting of SystemInit() function, 
       refer to system_stm32f4xx.c file */

    /* SysTick end of count event each 1ms */
    SystemCoreClockUpdate();
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

    /* Add your application code here */
    /* Insert 50 ms delay */
    // delay_ms(50);
    Key_IO_Init();
    Led_IO_Init();
    //TIM_Config();
    USART1_Init();
    app_elog_init();
    log_a("This is bootloader!");

    SPI1_Init();
    W25Q64_Init();

    if (Key_Scan())
    {
        // donwload to backup area
        fil_size = Ymodem_Receive(au8_test);
        // copy backup area data to A area
        if (0 == Back_To_App(fil_size))
        {
            Jump_To_App();
        }
        else
        {
            log_i("Back to app failed!");
        }
    }
    else
    {
        Jump_To_App();
    }

    /* Infinite loop */
    while (1)
    {
        log_e("No Valid App,Please press key and download new App!");
        if (Key_Scan())
        {
            //按下
            /*1.下载到备份区*/
            fil_size = Ymodem_Receive(au8_test);
            /*2.备份区数据拷贝到A区中*/
            if (0 == Back_To_App(fil_size))
            {
                Jump_To_App();
            }
            else
            {
                //
            }
        }
        delay_ms(50);
    }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay_ms(__IO uint32_t nTime)
{
    uwTimingDelay = nTime;

    while (uwTimingDelay != 0)
        ;
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
    if (uwTimingDelay != 0x00)
    {
        uwTimingDelay--;
    }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
