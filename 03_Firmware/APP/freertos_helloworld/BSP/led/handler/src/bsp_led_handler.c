/******************************************************************************
 * @file bsp_led_handler.c
 *
 * @par dependencies
 * - bsp_led_handler.h
 * 
 * @author Ethan-Hang
 *
 * @brief
 *
 * Processing flow:
 *
 *
 * @version V1.0 2025-11-15
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_led_handler.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
/**
 * @brief Initialize LED driver array
 * 
 * Initializes all elements in the LED driver array to INIT_PATTERN
 * for memory validity checking.
 * 
 * @param[in,out] array      : LED driver array to initialize
 * @param[in]     array_size : Number of array elements
 * 
 * @return led_handler_status_t
 *         - HANDLER_OK: Initialization successful
 * 
 * @note TBD: Add memory validity checking
 * */
static led_handler_status_t __array_init(
                                         bsp_led_driver_t * array[], 
                                         uint32_t        array_size
                                        )
{
    for ( uint32_t i = 0; i < array_size; i++ )
    {
        array[i] = (bsp_led_driver_t *) INIT_PATTERN;
    }
    // TBD: valid the memory to check if init success
    return HANDLER_OK;
}

/**
 * @brief Register LED driver instance to handler
 * 
 * Registers a new LED driver instance to the handler's instance group.
 * Validates input, checks resource availability, and adds instance.
 * Thread-safe when OS is supported.
 * 
 * @param[in,out] self       : LED handler instance
 * @param[in]     led_driver : LED driver instance to register
 * @param[out]    index      : Assigned index in instance group
 * 
 * @return led_handler_status_t
 *         - HANDLER_OK            : Success
 *         - HANDLER_ERRORPARAMETER: Invalid parameter or not initialized
 *         - HANDLER_ERRORRESOURCE : Max instances reached
 * 
 * @note Thread-safe when OS_SUPPORTING enabled
 * 
 * @example
 *       led_index_t idx;
 *       led_register(&handler, &led_inst, &idx);
 * */
static led_handler_status_t led_register(
                      bsp_led_handler_t * const       self,
                      bsp_led_driver_t  * const led_driver,
                      led_index_t       * const      index
                                        )
{
#if DEBUG
    DEBUG_OUT("led_register start\r\n");
#endif // DEBUG
    led_handler_status_t ret = HANDLER_OK;

    /********************** 0.Checking the target status *********************/
    // 1. Check if the target has been instantiated
    if ( NULL       == led_driver     ||
         NULL       == index          ||
         NOT_INITED == self->is_inited )
    {
        // 2. if not instantiated, return error to caller
        // TBD: mutex to upgrade low proiority task to init target ASAP
#if DEBUG
        DEBUG_OUT("led_driver input parameter error\r\n");
#endif // DEBUG

        ret = HANDLER_ERRORPARAMETER;
        return ret;
    }

    /********************** 1.Checking input parameters **********************/
    if ( NOT_INITED == self->is_inited )
    {
#if DEBUG
        DEBUG_OUT("led register input parameter error\r\n");
#endif // DEBUG
        ret = HANDLER_ERRORRESOURCE;
        return ret;
    }
   
    /***************** 2.Adding the instance in target array *****************/
    if ( (MAX_INSTANCE_NUMBER - self->instances.led_instance_num) == 0 )
    {
        ret = HANDLER_ERRORRESOURCE;
        return ret;
    }

    #if OS_SUPPORTING
    self->p_os_critical->pf_os_critical_enter();
#endif // OS_SUPPORTING
    if ( (MAX_INSTANCE_NUMBER - self->instances.led_instance_num) > 0  )
    {
        self->instances.led_instance_group \
        [self->instances.led_instance_num] = led_driver;

        *index = (led_index_t)(self->instances.led_instance_num);
        
        self->instances.led_instance_num++;
    }
#if OS_SUPPORTING
        self->p_os_critical->pf_os_critical_exit();
#endif // OS_SUPPORTING

#if DEBUG
    DEBUG_OUT("led_register success\r\n");
#endif // DEBUG
    return ret;

}

/**
 * @brief Control LED blinking with timing parameters
 * 
 * Validates input parameters, configures LED blinking pattern,
 * and triggers the blinking operation. Blocking function.
 * 
 * @param[in,out] self             : LED driver instance
 * @param[in]     cycle_time_ms    : Blink cycle time in ms (0-10000)
 * @param[in]     blink_times      : Blink repetitions (0-1000)
 * @param[in]     proportion_on_off: ON/OFF duty cycle
 *                  - PROPORTION_1_3: 1:3 ratio (25%)
 *                  - PROPORTION_1_2: 1:2 ratio (33%)
 *                  - PROPORTION_1_1: 1:1 ratio (50%)
 * 
 * @return led_status_t
 *         - LED_OK             : Success
 *         - LED_ERRORPARAMETER : Invalid parameter
 * 
 * @note Total time = cycle_time_ms × blink_times
 * 
 * @example
 *       led_control(&led_inst, 1000, 5, PROPORTION_1_1);
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

    return ret;

}

/**
 * @brief Initialize LED handler with dependency injection
 * 
 * Initializes LED handler by mounting external and internal interfaces.
 * Validates parameters, checks initialization status, and prepares
 * instance group for LED drivers.
 * 
 * @param[in,out] self         : LED handler instance
 * @param[in]     os_delay     : OS delay service (if OS_SUPPORTING)
 * @param[in]     os_queue     : OS queue service (if OS_SUPPORTING)
 * @param[in]     os_critical  : OS critical section (if OS_SUPPORTING)
 * @param[in]     time_base_ms : Millisecond timebase service
 * 
 * @return led_handler_status_t
 *         - HANDLER_OK            : Success
 *         - HANDLER_ERRORPARAMETER: NULL pointer in parameters
 *         - HANDLER_ERRORRESOURCE : Already initialized
 *         - HANDLER_ERRORMEMORY   : Array init failed
 * 
 * @note Must be called before any other handler operations
 * 
 * @example
 *       led_handler_inst(&handler, &delay, &queue, &crit, &time);
 * */
led_handler_status_t led_handler_inst(
                bsp_led_handler_t      * const        self,
#if OS_SUPPORTING
                os_delay_t             * const    os_delay,
                handler_os_queue_t     * const    os_queue,
                handler_os_critical_t  * const os_critical,
#endif // OS_SUPPORTING
                handler_time_base_ms_t * const time_base_ms
                                     )
{
    led_handler_status_t ret = HANDLER_OK;
#if DEBUG
    DEBUG_OUT("Handler kick off\r\n");
#endif // DEBUG
    /************ 1.Checking input parameters ************/
    if ( NULL == self         ||
         NULL == time_base_ms
#if OS_SUPPORTING
      || NULL == os_delay
      || NULL == os_queue
#endif // OS_SUPPORTING
        )
    {
#if DEBUG
        DEBUG_OUT("input parameter error\r\n");
#endif // DEBUG      
        
        ret = HANDLER_ERRORPARAMETER;
        return ret;
    }

    /************* 2.Checking the Resources **************/
    if ( INITED == self->is_inited )
    {        
#if DEBUG
        DEBUG_OUT("handler errorresource: already inited\r\n");
#endif // DEBUG

        ret = HANDLER_ERRORRESOURCE;
        return ret;
    }

#if DEBUG
    DEBUG_OUT("Handler_inst_start\r\n");
#endif // DEBUG

    /************** 3.Adding the interface ***************/

    // 3.1 mount external interfaces
    self->p_time_base_ms             =        time_base_ms;
#if OS_SUPPORTING
    self->p_os_time_delay            =            os_delay;
    self->p_os_queue_interface       =            os_queue;
    self->p_os_critical              =         os_critical;
#endif // OS_SUPPORTING

    // 3.2 mount internal interfaces
    self->pf_led_controller          =         led_control;
    self->pf_led_register            =        led_register;

    /**************** 4.Adding the targets ***************/
    // 4.1 init the variables will be used
    self->instances.led_instance_num =                   0;
    ret = __array_init(self->instances.led_instance_group,
                       MAX_INSTANCE_NUMBER
                      );
    if ( HANDLER_OK != ret )
    {
#if DEBUG
        DEBUG_OUT("handler errormemory: array init failed\r\n");
#endif // DEBUG
        return ret;
    }

    self->is_inited = INITED;
#if DEBUG
    DEBUG_OUT("Handler_inst_end\r\n");
#endif // DEBUG

    return ret;
}


//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//
