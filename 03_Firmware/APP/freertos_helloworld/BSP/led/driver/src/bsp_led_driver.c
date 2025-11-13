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
 * @brief Perform LED blinking based on configured parameters
 * 
 * Reads blink parameters and executes LED blinking sequence.
 * Calculates ON/OFF timing based on proportion.
 * 
 * @param[in] self : LED driver instance
 *                   - Must be initialized
 *                   - Contains cycle_time_ms, blink_times, proportion
 * 
 * @return led_status_t
 *         - LED_OK              : Success
 *         - LED_ERRORPARAMETER  : NULL or not initialized
 *         - LED_ERRORNOMEMORY   : Invalid proportion
 * 
 * @note Internal function called by led_control()
 *       Blocks during execution
 * */
static led_status_t led_blink(bsp_led_driver_t *const self)
{
    led_status_t ret = LED_OK;
    // 1. Check if the target has been instantiated
    if ( NULL       == self             ||
         NOT_INITED == self->led_is_init )
    {
        // 2. if not instantiated, return error to caller
        // TBD: mutex to upgrade low proiority task to init target ASAP
#if DEBUG
        DEBUG_OUT("led control input parameter error\r\n");
#endif // DEBUG

        return LED_ERRORPARAMETER;
    }

    // 2. analyze the featrues
    {
        // 2.1 define time value for saving featrues
        uint32_t                   cycle_time_local;
        uint32_t                  blink_times_local;
        proportion_t               proportion_local;
        uint32_t                    led_time_toggle;

        cycle_time_local  =     self->cycle_time_ms;
        blink_times_local =       self->blink_times;
        proportion_local  = self->proportion_on_off;

        // 2.2 calculate the time for each led to toggle
        switch ( proportion_local )
        {
            case PROPORTION_1_3 :
                led_time_toggle = cycle_time_local / 4;
                break;
            case PROPORTION_1_2 :
                led_time_toggle = cycle_time_local / 3;
                break;
            case PROPORTION_1_1 :
                led_time_toggle = cycle_time_local / 2;
                break;
            case PROPORTION_x_x :
            default :
#if DEBUG
                DEBUG_OUT("led errormemory\r\n");
#endif // DEBUG
                return LED_ERRORNOMEMORY;
                // break;
        }

        // 3. do the operations of led
        for (uint32_t i = 0; i < blink_times_local; i++)
        {
            // 3.1 Turn LED ON for calculated time
            self->p_led_opes_inst->pf_led_on();
#if OS_SUPPORTING
            self->p_os_time_delay->pf_os_delay_ms(led_time_toggle);
#else
            // Non-OS: use time polling
            uint32_t time_start = 0;
            uint32_t time_now = 0;
            self->p_time_base_ms->pf_get_time_ms(&time_start);
            do 
            {
                self->p_time_base_ms->pf_get_time_ms(&time_now);
            } while ((time_now - time_start) < led_time_toggle);
#endif // OS_SUPPORTING
            
            // 3.2 Turn LED OFF for remaining time
            self->p_led_opes_inst->pf_led_off();
#if OS_SUPPORTING
            self->p_os_time_delay->pf_os_delay_ms(
                cycle_time_local - led_time_toggle
            );
#else
            // Non-OS: use time polling
            self->p_time_base_ms->pf_get_time_ms(&time_start);
            do 
            {
                self->p_time_base_ms->pf_get_time_ms(&time_now);
            } while ((time_now - time_start) < 
                     (cycle_time_local - led_time_toggle));
#endif // OS_SUPPORTING
        }
    }

    return ret;
}

/**
 * @brief Control LED blinking with timing parameters
 * 
 * Main LED control interface. Validates input, updates config,
 * and triggers blinking. Called via pf_led_controller.
 * 
 * @param[in,out] self             : LED driver instance
 * @param[in]     cycle_time_ms    : Blink cycle time (0-10000ms)
 * @param[in]     blink_times      : Repeat count (0-1000)
 * @param[in]     proportion_on_off: ON/OFF ratio
 *                  - PROPORTION_1_3: 1:3 (25% duty)
 *                  - PROPORTION_1_2: 1:2 (33% duty)
 *                  - PROPORTION_1_1: 1:1 (50% duty)
 * 
 * @return led_status_t
 *         - LED_OK             : Success
 *         - LED_ERRORPARAMETER : Invalid parameter
 * 
 * @note Blocks until complete
 *       Time = cycle_time_ms × blink_times
 * 
 * @example
 *       led_control(&inst, 1000, 5, PROPORTION_1_1);
 * */
static led_status_t led_control(
                             bsp_led_driver_t * const self,
                             uint32_t        cycle_time_ms,
                             uint32_t          blink_times,
                             proportion_t proportion_on_off
                               )
{
    led_status_t ret = LED_OK;

    /************ 0.Checking the target status ***********/
    // 1. Check if the target has been instantiated
    if ( NULL       == self             ||
         NOT_INITED == self->led_is_init )
    {
        // 2. if not instantiated, return error to caller
        // TBD: mutex to upgrade low proiority task to init target ASAP
#if DEBUG
        DEBUG_OUT("led control input parameter error\r\n");
#endif // DEBUG
        return LED_ERRORPARAMETER;
    }

    /************ 1.Checking input parameters ************/
    // 1.1 Check cycle_time_ms
    if ( 
        !(
         cycle_time_ms    <= 10000                       &&
         blink_times      <= 1000                        &&
         ((PROPORTION_1_3 == proportion_on_off           ||
           PROPORTION_1_2 == proportion_on_off           || 
           PROPORTION_1_1 == proportion_on_off           ))
         ) // end of !
       )   // end of if condition
    {
#if DEBUG
        DEBUG_OUT("led control input parameter error\r\n");
#endif // DEBUG
        ret = LED_ERRORPARAMETER;
        return ret;
    }

    /************ 2.Adding the data in target ************/
    self->cycle_time_ms     =                cycle_time_ms;
    self->blink_times       =                  blink_times;
    self->proportion_on_off =            proportion_on_off;

    /************ 3.Run the operations of led ************/
    led_blink(self);

    return ret;

}

/**
 * @brief Initialize LED hardware
 * 
 * @param[in,out] self : LED driver instance
 * 
 * @return led_status_t
 *         - LED_OK             : Success
 *         - LED_ERRORPARAMETER : NULL pointer
 * 
 * @note Called internally by led_driver_inst()
 */
static led_status_t led_driver_init(bsp_led_driver_t * const self)
{
    led_status_t ret = LED_OK;

    if (NULL == self)
    {
#if DEBUG
        DEBUG_OUT("led init input parameter error\r\n");
#endif // DEBUG
        return LED_ERRORPARAMETER;
    }

    self->p_led_opes_inst->pf_led_off();
#if OS_SUPPORTING    
    self->p_os_time_delay->pf_os_delay_ms(600);
#endif
    uint32_t time_now = 0;
    self->p_time_base_ms->pf_get_time_ms(&time_now);
    
    return ret;
}


/**
 * @brief Initialize LED driver with dependency injection
 * 
 * @param[in,out] self         : LED driver instance
 * @param[in]     led_ops      : LED operations (on/off)
 * @param[in]     os_delay     : OS delay (if OS_SUPPORTING)
 * @param[in]     time_base_ms : Millisecond timebase
 * 
 * @return led_status_t
 *         - LED_OK             : Success
 *         - LED_ERRORPARAMETER : NULL pointer
 *         - LED_ERROR          : Init error
 * 
 * @note Call before using LED control
 * */
led_status_t led_driver_inst(
                      bsp_led_driver_t * const        self,
                      led_operations_t * const     led_ops,
#if OS_SUPPORTING
                      os_delay_t       * const    os_delay,
#endif // OS_SUPPORTING
                      time_base_ms_t   * const time_base_ms
                            )
{
    led_status_t ret = LED_OK;
#if DEBUG
    DEBUG_OUT("led driver inst kick off\r\n");
#endif // DEBUG
    /************ 1.Checking input parameters ************/
    if ( NULL == self         ||
         NULL == led_ops      ||
         NULL == time_base_ms
#if OS_SUPPORTING
      || NULL == os_delay
#endif // OS_SUPPORTING
        )
    {
#if DEBUG
        DEBUG_OUT("input parameter error\r\n");
#endif // DEBUG      
        
        return LED_ERRORPARAMETER;
    }

    /************* 2.Checking the Resources **************/
    if ( INITED == self->led_is_init )
    {        
#if DEBUG
        DEBUG_OUT("led errorresource: already inited\r\n");
#endif // DEBUG

        return LED_ERRORRESOURCE;
    }

#if DEBUG
    DEBUG_OUT("led_inst_start\r\n");
#endif // DEBUG

    /************** 3.Adding the interface ***************/

    // 3.1 mount external interfaces
    self->p_led_opes_inst   =                      led_ops;
#if OS_SUPPORTING
    self->p_os_time_delay   =                     os_delay;
#endif // OS_SUPPORTING
    self->p_time_base_ms    =                 time_base_ms;

    // 3.2 mount internal interfaces
    self->pf_led_controller =                  led_control;

    /**************** 4.Adding the targets ***************/
    self->blink_times       =                            0;
    self->cycle_time_ms     =                            0;
    self->proportion_on_off =               PROPORTION_x_x;

    ret = led_driver_init(self);

    if (LED_OK != ret)
    {
#if DEBUG
        DEBUG_OUT("led init error\r\n");
#endif // DEBUG

        self->p_led_opes_inst  =  NULL;
#if OS_SUPPORTING
        self->p_os_time_delay  =  NULL;
#endif // OS_SUPPORTING
        self->p_time_base_ms   =  NULL;

        return ret;
    }

    self->led_is_init = INITED;
#if DEBUG
    DEBUG_OUT("led init finished\r\n");
#endif // DEBUG

    return ret;
}

//******************************** Defines **********************************//

//******************************* Declaring *********************************//


//******************************* Declaring *********************************//
