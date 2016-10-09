/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */
#include "BspConfig.h"
#ifdef _DRIVER_WIFI__
#define NOT_INCLUDE_OTHER


#include <stdint.h>
#include <string.h>
#include "platform_cmsis.h"
#include "platform_peripheral.h"
#include "platform_config.h"
#include "wwd_constants.h"
#include "wwd_platform_common.h"
#include "wwd_assert.h"
#include "wiced_dct_common.h"
#include "ap6181wifi.h"
#include "gpio.h"
#include "interrupt.h"

//#pragma arm section code = "ap6181wifiCode", rodata = "ap6181wifiCode", rwdata = "ap6181wifidata", zidata = "ap6181wifidata"


//rk change
//#include "platform/wwd_platform_interface.h"
#include "wwd_platform_interface.h"
/*rk del start*/
#if 0
#include "wiced_framework.h"
#endif

extern void wwd_thread_notify_irq( void );

/*rk del end*/

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
_ap6181WICE_WIFI_INIT_
wwd_result_t host_platform_init( void )
{

#ifdef _WIFI_OB
    rk_printf("gpio_in");
    IntRegister(INT_ID_GPIO2, GpioInt2);
    Grf_GpioMuxSet(AP6181_HOST_WAKE_GPIO_CH, AP6181_HOST_WAKE_GPIO_PIN, Type_Gpio);
    Gpio_SetPinDirection(AP6181_HOST_WAKE_GPIO_CH, AP6181_HOST_WAKE_GPIO_PIN, GPIO_IN);
    Grf_GPIO_SetPinPull(AP6181_HOST_WAKE_GPIO_CH,AP6181_HOST_WAKE_GPIO_PIN,DISABLE);
    Gpio_DisableInt(AP6181_HOST_WAKE_GPIO_CH, AP6181_HOST_WAKE_GPIO_PIN);
    Gpio_SetIntMode(AP6181_HOST_WAKE_GPIO_CH, AP6181_HOST_WAKE_GPIO_PIN, IntrTypeRisingEdge);
    GpioIsrRegister(AP6181_HOST_WAKE_GPIO_CH, AP6181_HOST_WAKE_GPIO_PIN, wwd_thread_notify_irq);
#endif
    //Grf_GpioMuxSet(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, Type_Gpio);
    //Gpio_SetPinDirection(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_OUT);

    host_platform_power_wifi(WICED_FALSE);
    DelayMs(2);
    host_platform_power_wifi(WICED_TRUE);
    return WWD_SUCCESS;
}
_ap6181WICE_WIFI_INIT_
wwd_result_t host_platform_deinit( void )
{
#if 0
#if defined ( WICED_USE_WIFI_RESET_PIN )
    platform_gpio_init( &wifi_control_pins[WWD_PIN_RESET], OUTPUT_PUSH_PULL );
    host_platform_reset_wifi( WICED_TRUE );  /* Start wifi chip in reset */
#endif

#if defined ( WICED_USE_WIFI_POWER_PIN )
    platform_gpio_init( &wifi_control_pins[WWD_PIN_POWER], OUTPUT_PUSH_PULL );
    host_platform_power_wifi( WICED_FALSE ); /* Start wifi chip with regulators off */
#endif

    host_platform_deinit_wlan_powersave_clock( );
#endif
    //Grf_GpioMuxSet(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, Type_Gpio);
    //Gpio_SetPinDirection(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_IN);
    return WWD_SUCCESS;
}

void host_platform_reset_wifi( wiced_bool_t reset_asserted )
{
#if 0
#if defined ( WICED_USE_WIFI_RESET_PIN )
    ( reset_asserted == WICED_TRUE ) ? platform_gpio_output_low( &wifi_control_pins[ WWD_PIN_RESET ] ) : platform_gpio_output_high( &wifi_control_pins[ WWD_PIN_RESET ] );
#else
    UNUSED_PARAMETER( reset_asserted );
#endif
#endif

}

void host_platform_power_wifi( wiced_bool_t power_enabled )
{
#if 0
#if   defined ( WICED_USE_WIFI_POWER_PIN ) && defined ( WICED_USE_WIFI_POWER_PIN_ACTIVE_HIGH )
    ( power_enabled == WICED_TRUE ) ? platform_gpio_output_high( &wifi_control_pins[WWD_PIN_POWER] ) : platform_gpio_output_low ( &wifi_control_pins[WWD_PIN_POWER] );
#elif defined ( WICED_USE_WIFI_POWER_PIN )
    ( power_enabled == WICED_TRUE ) ? platform_gpio_output_low ( &wifi_control_pins[WWD_PIN_POWER] ) : platform_gpio_output_high( &wifi_control_pins[WWD_PIN_POWER] );
#else
    UNUSED_PARAMETER( power_enabled );
#endif
#endif
    if(power_enabled)
    {
        //Gpio_SetPinLevel(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_HIGH);
    }
    else
    {
        //Gpio_SetPinLevel(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_LOW);
    }

}

#if 0
uint32_t host_platform_get_cycle_count( void )
{

    /* From the ARM Cortex-M3 Techinical Reference Manual
     * 0xE0001004  DWT_CYCCNT  RW  0x00000000  Cycle Count Register */
    return DWT->CYCCNT;
}
#endif

wiced_bool_t host_platform_is_in_interrupt_context( void )
{
    /* From the ARM Cortex-M3 Techinical Reference Manual
     * 0xE000ED04   ICSR    RW [a]  Privileged  0x00000000  Interrupt Control and State Register */
    return ( ( SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk ) != 0 ) ? WICED_TRUE : WICED_FALSE;
}

void host_platform_get_mac_address( wiced_mac_t* mac )
{
#if !defined ( WICED_DISABLE_BOOTLOADER )
    wiced_mac_t* temp_mac;
    wiced_dct_read_lock( (void**) &temp_mac, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, OFFSETOF( platform_dct_wifi_config_t, mac_address ), sizeof(wiced_mac_t) );
    memcpy( mac->octet, temp_mac, sizeof(wiced_mac_t) );
    wiced_dct_read_unlock( temp_mac, WICED_FALSE );
#else
    UNUSED_PARAMETER( mac );
#endif
}
_ap6181WICE_WIFI_INIT_
wwd_result_t host_platform_deinit_wlan_powersave_clock( void )
{
#if 0
    platform_gpio_init( &wifi_control_pins[WWD_PIN_32K_CLK], OUTPUT_PUSH_PULL );
    platform_gpio_output_low( &wifi_control_pins[WWD_PIN_32K_CLK] );
#endif
    return WWD_SUCCESS;
}


//#pragma arm section code
#endif
