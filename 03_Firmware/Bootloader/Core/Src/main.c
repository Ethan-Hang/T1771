/*Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "gpio.h"
#include "main.h"
#include "tim.h"

#include "Boot_Manager.h"
#include "Debug.h"
#include "SEGGER_RTT.h"
#include "elog.h"

/*Private typedef -----------------------------------------------------------*/

/*Private define ------------------------------------------------------------*/

/*Private macro -------------------------------------------------------------*/

/*Private variables ---------------------------------------------------------*/
static __IO uint32_t uwTimingDelay;
RCC_ClocksTypeDef    RCC_Clocks;

/*Private function prototypes -----------------------------------------------*/

/*Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void)
{
    //    SCB->VTOR = 0x08000000 | 0x00000000;

    /* Enable Clock Security System(CSS): this will generate an NMI exception
     when HSE clock fails *****************************************************/
    RCC_ClockSecuritySystemCmd(ENABLE);

    /* SysTick end of count event each 1ms */
    SystemCoreClockUpdate();
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

    /* Add your application code here */
    /* Insert 50 ms delay */
    delay_ms(50);

    //    GPIO_Config();
    //    TIM_Config();
    app_elog_init();

    Jump_To_App();

    /* Infinite loop */
    while (1)
    {
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

#ifdef __GUNC__
#define PUTCHAR_PORTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_POROTOTYPE int fputc(int ch, FILE *f)
#endif

/******************************************************************
 *@brief  Retargets the C library printf function to the USART.
 *@param  None
 *@retval None
 ******************************************************************/
PUTCHAR_POROTOTYPE
{
    // SEGGER_RTT_PutChar(0, ch);
    return ch;
}
