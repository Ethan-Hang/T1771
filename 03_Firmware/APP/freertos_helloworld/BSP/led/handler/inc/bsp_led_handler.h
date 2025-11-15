/******************************************************************************
 * @file bsp_led_handler.h
 *
 * @par dependencies
 * - bsp_led_driver.h
 * - stdio.h
 * - stdint.h
 * 
 * @author Ethan-Hang
 *
 * @brief LED Handler interface with dependency injection pattern
 *
 * LED handler manages multiple LED driver instances and provides
 * unified control interface. Supports OS services for critical
 * sections and delays when OS_SUPPORTING is enabled.
 *
 * Usage:
 * 1. Initialize handler with led_handler_inst()
 * 2. Register LED drivers with pf_led_register
 * 3. Control LEDs via pf_led_controller
 *
 * @version V1.0 2025-11-15
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __BSP_LED_HANDLER_H__
#define __BSP_LED_HANDLER_H__

//******************************** Includes *********************************//
#include "bsp_led_driver.h"

#include <stdio.h>
#include <stdint.h>

//******************************** Includes *********************************//

//******************************** Defines **********************************//
// #define OS_SUPPORTING         (1)       /* Enable OS supporting feature      */

// #define DEBUG                 (1)       /* Enable debug feature              */
// #define DEBUG_OUT(X)          printf(X) /* Debug output macro                */
#define INIT_PATTERN          (0xFFFFFFFF) /* Init Space with this Pattern   */

typedef struct bsp_led_handler bsp_led_handler_t;

/**
 * @brief LED handler initialization status
 * */
typedef enum
{
    HANDLER_NOT_INITED       = 0,       /* Not initialized                   */
    HANDLER_INITED           = 1,       /* Initialized successfully          */
} led_handler_init_t;

/**
 * @brief LED instance index enumeration
 * 
 * Defines indices for registered LED driver instances.
 * MAX_INSTANCE_NUMBER limits concurrent LED instances.
 * */
typedef enum
{
    LED_1                    = 0,       /* LED instance 1                    */
    LED_2,                              /* LED instance 2                    */
    LED_3,                              /* LED instance 3                    */
    LED_4,                              /* LED instance 4                    */
    LED_5,                              /* LED instance 5                    */
    LED_6,                              /* LED instance 6                    */
    LED_7,                              /* LED instance 7                    */
    LED_8,                              /* LED instance 8                    */
    LED_9,                              /* LED instance 9                    */
    LED_10,                             /* LED instance 10                   */
    MAX_INSTANCE_NUMBER,                /* Max instance count                */
    LED_NOT_INITIALIZED      = 0xFFFFFF,/* Uninitialized index               */
} led_index_t;

/**
 * @brief Handler operation status codes
 * 
 * Return codes for handler operations indicating success or
 * the type of error encountered.
 * */
typedef enum
{
    HANDLER_OK               = 0,       /* Operation successful              */
    HANDLER_ERROR            = 1,       /* General error                     */
    HANDLER_ERRORTIMEOUT     = 2,       /* Timeout error                     */
    HANDLER_ERRORRESOURCE    = 3,       /* Resource unavailable              */
    HANDLER_ERRORPARAMETER   = 4,       /* Invalid parameter                 */
    HANDLER_ERRORNOMEMORY    = 5,       /* Out of memory                     */
    HANDLER_ERRORISR         = 6,       /* ISR context error                 */
    HANDLER_RESERVED         = 0xFF,    /* Reserved                          */
} led_handler_status_t;

/**
 * @brief Millisecond timebase service interface
 * */
typedef struct 
{
    /** Get current time in milliseconds */
    led_handler_status_t (*pf_get_time_ms) ( uint32_t * const );
} handler_time_base_ms_t;

#ifdef OS_SUPPORTING
/**
 * @brief OS delay service interface
 * */
typedef struct 
{
    /** Delay execution for specified milliseconds */
    led_handler_status_t (*pf_os_delay_ms) ( uint32_t const  );
} handler_os_delay_t;

/**
 * @brief OS queue service interface
 * */
typedef struct 
{   
    /** Create a message queue */
    led_handler_status_t (*pf_os_queue_create) (
                                                   uint32_t const     item_num,
                                                   uint32_t const    item_size,
                                                   void **  const queue_handler
                                               );
    /** Put message into queue */
    led_handler_status_t (*pf_os_queue_put   ) (
                                                   void *  const queue_handler,
                                                   void *  const          item,
                                                   uint32_t             timeout
                                               );    
    /** Get message from queue */
    led_handler_status_t (*pf_os_queue_get   ) (
                                                   void *  const queue_handler,
                                                   void *  const           msg,
                                                   uint32_t             timeout
                                               );
    /** Delete queue */
    led_handler_status_t (*pf_os_queue_delete) (
                                                   void *  const  queue_handler
                                               );
} handler_os_queue_t;

/**
 * @brief OS critical section service interface
 * */
typedef struct 
{
    /** Enter critical section */
    led_handler_status_t (*pf_os_critical_enter) ( void );
    /** Exit critical section */
    led_handler_status_t (*pf_os_critical_exit ) ( void );
} handler_os_critical_t;

#endif // OS_SUPPORTING

/**
 * @brief LED control function pointer type
 * */
typedef led_handler_status_t (*pf_handler_led_control_t)(
                                                 bsp_led_driver_t * const self,
                                                 uint32_t        cycle_time_ms,
                                                 uint32_t          blink_times,
                                                 proportion_t proportion_on_off
                                                        );

/**
 * @brief LED registration function pointer type
 * */
typedef led_handler_status_t (*pf_handler_led_register_t)(
                                          bsp_led_handler_t * const       self,
                                          bsp_led_driver_t  * const led_driver,
                                          led_index_t       * const      index              
                                                         );

/**
 * @brief Registered LED instances collection
 * */
typedef struct 
{
    uint32_t                         led_instance_num;/* Count of instances  */
    bsp_led_driver_t *               led_instance_group[MAX_INSTANCE_NUMBER];
                                                      /* Instance array      */
} instance_registered_t;

/**
 * @brief LED handler main structure
 * 
 * Manages LED driver instances, interfaces, and control operations.
 * Uses dependency injection for OS services.
 * */
typedef struct bsp_led_handler
{
    /************* Target of Internal Status *************/
    uint8_t                                      is_inited;

    /*********** Target of Registered Instances *********/
    instance_registered_t                        instances;

    /*************** Target of IOs needed ****************/
    /*      The interface from core layer                */
    handler_time_base_ms_t                 *p_time_base_ms;
    /*       The interface from OS layer                 */
#ifdef OS_SUPPORTING
    os_delay_t                            *p_os_time_delay;
    handler_os_queue_t               *p_os_queue_interface;
    handler_os_critical_t                   *p_os_critical;
#endif // OS_SUPPORTING

    /****************** Target of APIs *******************/
    pf_led_control_t                     pf_led_controller;
    pf_handler_led_register_t              pf_led_register;

} bsp_led_handler_t;

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
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
                                     );

//******************************* Declaring *********************************//

#endif // End of __BSP_LED_HANDLER_H__
