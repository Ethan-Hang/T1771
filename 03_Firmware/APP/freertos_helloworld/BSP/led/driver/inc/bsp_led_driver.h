/******************************************************************************
 * @file bsp_led_driver.h
 *
 * @par dependencies
 * - stdio.h
 * - stdint.h
 * 
 * @author Ethan-Hang
 *
 * @brief Provides HAL APIs for LED control and operations
 *
 * Usage:
 * Call functions directly.
 *
 * @version V1.0 2025-11-13
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __BSP_LED_DRIVER_H__
#define __BSP_LED_DRIVER_H__

//******************************** Includes *********************************//
#include <stdio.h>
#include <stdint.h>

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define INITED             1        /* LED Driver inited flag                */
#define NOT_INITED         0        /* LED Driver not inited flag            */

#define OS_SUPPORTING     (1)       /* Enable OS supporting feature          */

#define DEBUG             (0)       /* Enable debug feature                  */
#define DEBUG_OUT(X)      printf(X) /* Debug output macro                    */

typedef struct bsp_led_driver bsp_led_driver_t;

typedef enum
{
    PROPORTION_1_3       = 0,       /* ON:OFF = 1:3                          */
    PROPORTION_1_2       = 1,       /* ON:OFF = 1:2                          */
    PROPORTION_1_1       = 2,       /* ON:OFF = 1:1                          */
    PROPORTION_x_x       = 0xFF,    /* Custom proportion                     */
} proportion_t;

typedef enum
{
    LED_OK               = 0,       /* Operation successful                  */
    LED_ERROR            = 1,       /* General error                         */
    LED_ERRORTIMEOUT     = 2,       /* Timeout error                         */
    LED_ERRORRESOURCE    = 3,       /* Resource unavailable                  */
    LED_ERRORPARAMETER   = 4,       /* Invalid parameter                     */
    LED_ERRORNOMEMORY    = 5,       /* Out of memory                         */
    LED_ERRORISR         = 6,       /* ISR context error                     */
    LED_RESERVED         = 0xFF,    /* LED Reserved                          */
} led_status_t;

typedef struct 
{
    led_status_t (*pf_led_on)                    (void); /* Turn LED on      */
    led_status_t (*pf_led_off)                   (void); /* Turn LED off     */
} led_operations_t;

typedef struct 
{
    led_status_t (*pf_get_time_ms) ( uint32_t * const ); /* Get time in ms   */
} time_base_ms_t;

#ifdef OS_SUPPORTING
typedef struct 
{
    led_status_t (*pf_os_delay_ms)   ( const uint32_t ); /* OS delay in ms   */
} os_delay_t;
#endif // OS_SUPPORTING

typedef led_status_t (*pf_led_control_t)(
                                        bsp_led_driver_t * const self,
                                        uint32_t        cycle_time_ms,
                                        uint32_t          blink_times,
                                        proportion_t proportion_on_off
                                        );

typedef struct bsp_led_driver
{
    /************* Target of Internal Status *************/
    uint8_t                                    led_is_init;
    /***************** Target of Features ****************/
    /* The whole time of blink                           */
    uint32_t                                 cycle_time_ms;
    /* The times of blink                                */
    uint32_t                                   blink_times;
    /* The time of light on and off                      */
    proportion_t                         proportion_on_off;

    /*************** Target of IOs needed ****************/
    /*      The interface from core layer                */
    led_operations_t                      *p_led_opes_inst;
    time_base_ms_t                         *p_time_base_ms;
    /*       The interface from OS layer                 */
#ifdef OS_SUPPORTING
    os_delay_t                            *p_os_time_delay;     
#endif // OS_SUPPORTING

    /****************** Target of APIs *******************/
    pf_led_control_t                     pf_led_controller;

} bsp_led_driver_t;

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
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
                      bsp_led_driver_t * const        self,
                      led_operations_t * const     led_ops,
#ifdef OS_SUPPORTING
                      os_delay_t       * const    os_delay,
#endif // OS_SUPPORTING
                      time_base_ms_t   * const time_base_ms
                            );

//******************************* Declaring *********************************//

#endif // End of __BSP_LED_DRIVER_H__
