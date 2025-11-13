/******************************************************************************
 * @file bsp_led_driver.c
 *
 * @par dependencies
 * - bsp_led_driver.h
 * 
 * @author Ethan-Hang
 *
 * @brief
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2025-11-13
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_led_driver.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
/**
 * @brief Initialize LED driver instance with dependency injection
 * 
 * @param[in,out] self          LED driver instance to initialize
 * @param[in]     led_ops       LED hardware operations (on/off)
 * @param[in]     os_delay      OS delay interface (if OS_SUPPORTING)
 * @param[in]     time_base_ms  Millisecond time base interface
 * 
 * @return led_status_t
 *         - LED_OK              : Success
 *         - LED_ERRORPARAMETER  : Invalid parameter (NULL pointer)
 *         - LED_ERROR           : Initialization error
 * 
 * @note Must be called before using LED control functions
 * */
led_status_t led_driver_inst(
                                      bsp_led_driver_t * const            self,
                                const led_operations_t * const         led_ops,
#ifdef OS_SUPPORTING
                                const os_delay_t       * const        os_delay,
#endif // OS_SUPPORTING
                                const time_base_ms_t   * const     time_base_ms
                            )
{
    led_status_t ret = LED_OK;
    DEBUG_OUT("led driver inst start\r\n");

    if ( NULL == self         ||
         NULL == led_ops      ||
         NULL == time_base_ms
#ifdef OS_SUPPORTING
      || NULL == os_delay
#endif // OS_SUPPORTING
        )
    {
#ifdef DEBUG
        DEBUG_OUT("input parameter error\r\n");
        return LED_ERRORPARAMETER;
#endif // DEBUG        
    }

    if ( INITED == self->led_is_init )
    {
#ifdef DEBUG
        DEBUG_OUT("led errorresource: already inited\r\n");
        return LED_ERRORRESOURCE;
#endif // DEBUG
    }

#ifdef DEBUG
    DEBUG_OUT("led_inst_start\r\n");
#endif // DEBUG


    return ret;
}

//******************************** Defines **********************************//

//******************************* Declaring *********************************//


//******************************* Declaring *********************************//
