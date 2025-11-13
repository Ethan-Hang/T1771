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

led_status_t led_driver_init(bsp_led_driver_t * const self)
{
    led_status_t ret = LED_OK;

    if (NULL == self)
    {
#ifdef DEBUG
        DEBUG_OUT("led init input parameter error\r\n");
#endif // DEBUG
        return LED_ERRORPARAMETER;
    }

    // uint test
    self->p_led_opes_inst->pf_led_off();
    uint32_t time_now = 0;
    self->p_time_base_ms->pf_get_time_ms(&time_now);
    self->p_os_time_delay->pf_os_delay_ms(100);
    
    return ret;
}


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
                                bsp_led_driver_t * const                  self,
                                led_operations_t * const               led_ops,
#ifdef OS_SUPPORTING
                                os_delay_t       * const              os_delay,
#endif // OS_SUPPORTING
                                time_base_ms_t   * const           time_base_ms
                            )
{
    led_status_t ret = LED_OK;
    DEBUG_OUT("led driver inst kick off\r\n");

    /************ 1.Checking input parameters ************/
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
#endif // DEBUG      
        
        return LED_ERRORPARAMETER;
    }

    /************* 2.Checking the Resources **************/
    if ( INITED == self->led_is_init )
    {        
#ifdef DEBUG
        DEBUG_OUT("led errorresource: already inited\r\n");
#endif // DEBUG

        return LED_ERRORRESOURCE;
    }

#ifdef DEBUG
    DEBUG_OUT("led_inst_start\r\n");
#endif // DEBUG

    /************** 3.Adding the interface ***************/
    self->p_led_opes_inst  =        led_ops;
#ifdef OS_SUPPORTING
    self->p_os_time_delay  =       os_delay;
#endif // OS_SUPPORTING
    self->p_time_base_ms   =   time_base_ms;


    /**************** 4.Adding the targets ***************/
    self->blink_times       =               0;
    self->cycle_time_ms     =               0;
    self->proportion_on_off =  PROPORTION_x_x;

    ret = led_driver_init(self);

    if (LED_OK != ret)
    {
#ifdef DEBUG
        DEBUG_OUT("led init error\r\n");
#endif // DEBUG

        self->p_led_opes_inst  = NULL;
#ifdef OS_SUPPORTING
        self->p_os_time_delay  = NULL;
#endif // OS_SUPPORTING
        self->p_time_base_ms   = NULL;

        return ret;
    }

    self->led_is_init = INITED;
    return ret;
}

//******************************** Defines **********************************//

//******************************* Declaring *********************************//


//******************************* Declaring *********************************//
