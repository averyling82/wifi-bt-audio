/*
 * Copyright 2015, Broadcom Corporation
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

#if (defined _WIFI_AP6181) || (defined _WIFI_AP6234) || (defined _WIFI_AP6212)
#include "wwd_constants.h"
#include "wwd_wifi.h"

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

wwd_result_t wwd_wifi_read_wlan_log( char* buffer, uint32_t buffer_size )
{
    UNUSED_PARAMETER(buffer);
    UNUSED_PARAMETER(buffer_size);
    return WWD_UNSUPPORTED;
}

#endif /*ap6181*/

#ifdef _WIFI_5G_AP6255
#include "wwd_constants.h"
#include "wwd_wifi.h"
#include "internal/wwd_internal.h"
#include "wwd_bus_protocol_interface.h"
#include "wwd_debug.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define VERIFY_RESULT( x )     { wwd_result_t verify_result; verify_result = ( x ); if ( verify_result != WWD_SUCCESS ) {printf("%s: line=%d ret=%d\n", __FUNCTION__, __LINE__, verify_result); return verify_result;} }

/******************************************************
 *                    Constants
 ******************************************************/
#define WLAN_BUS_UP_ATTEMPTS    ( 1000 )
#define KSO_WAIT_MS             ( 1 )
#define KSO_WAIT_US             ( 50 )
#define MAX_KSO_ATTEMPTS        ( 64 )

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
static wwd_result_t wwd_kso_enable( wiced_bool_t enable );

/******************************************************
 *               Variables Definitions
 ******************************************************/
static wiced_bool_t bus_is_up               = WICED_FALSE;
static wiced_bool_t sr_enab                 = WICED_FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/
extern wwd_result_t wwd_bus_sdio_read_register_value( wwd_bus_function_t function, uint32_t address, uint8_t value_length, /*@out@*/ uint8_t* value );

wiced_bool_t wwd_sdio_sr_cap( void )
{

    wiced_bool_t sr_cap = WICED_FALSE;
    volatile uint32_t enab_val;
    wwd_result_t result;

    /* check if fw initialized sr engine */
    wwd_bus_write_backplane_value( (uint32_t) CHIPCOMMON_CHIPCTRL_ADDR, (uint8_t) 4, (uint32_t)CC_PMUCC3 );
    result = wwd_bus_read_backplane_value( (uint32_t) CHIPCOMMON_CHIPCTRL_DATA, (uint8_t) 4, (uint8_t*)&enab_val );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("[WiFi] %s: CHIPCOMMON_CHIPCTRL_DATA read failed\n", __FUNCTION__));
        return sr_cap;
    }

    enab_val &= CC_CHIPCTRL3_SR_ENG_ENABLE;
    if (enab_val )
    {
        sr_cap = WICED_TRUE;
    }
    return sr_cap;
}

wwd_result_t wwd_chip_specific_init( void )
{
    volatile uint8_t  data;

    // sr init
    if ( wwd_sdio_sr_cap( ) == WICED_TRUE )
    {
        /* Configure WakeupCtrl register to set HtAvail request bit in chipClockCSR register
         * after the sdiod core is powered on.
         */
        VERIFY_RESULT( wwd_bus_sdio_read_register_value( BACKPLANE_FUNCTION,
                (uint32_t) SDIO_WAKEUP_CTRL, (uint8_t) 1, (uint8_t *)&data ));
        data |= SBSDIO_WCTRL_WAKE_TILL_HT_AVAIL;
        VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION,
                (uint32_t) SDIO_WAKEUP_CTRL, (uint8_t) 1, (uint8_t)data ));

#ifdef WWD_USE_CMD14
    /* Add CMD14 Support */
        VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION,
                (uint32_t) SDIOD_CCCR_BRCM_CARDCAP, (uint8_t) 1, SDIOD_CCCR_BRCM_CARDCAP_CMD14_SUPPORT | SDIOD_CCCR_BRCM_CARDCAP_CMD14_EXT ));
#endif /* USE_CMD14 */

        /* Set brcmCardCapability to noCmdDecode mode.
         * It makes sdiod_aos to wakeup host for any activity of cmd line, even though
         * module won't decode cmd or respond
         */
        VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION,
                (uint32_t) SDIOD_CCCR_BRCM_CARDCAP, (uint8_t) 1, SDIOD_CCCR_BRCM_CARDCAP_CMD_NODEC ));

        VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION,
                (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, (uint32_t) SBSDIO_FORCE_HT ) );

        /* Enable KeepSdioOn (KSO) bit for normal operation */
        VERIFY_RESULT( wwd_bus_sdio_read_register_value( BACKPLANE_FUNCTION,
                (uint32_t) SDIO_SLEEP_CSR, (uint8_t) 1, &data ));
        if ( ( data & SBSDIO_SLPCSR_KEEP_SDIO_ON ) == 0 )
        {
            data |= SBSDIO_SLPCSR_KEEP_SDIO_ON;
            VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION,
                    (uint32_t) SDIO_SLEEP_CSR, (uint8_t) 1, data ));
        }

        /* SPI bus can be configured for sleep by default.
         * KSO bit solely controls the wlan chip sleep
         */
        VERIFY_RESULT( wwd_bus_specific_sleep( ));

        sr_enab = WICED_TRUE;
        WPRINT_WWD_INFO(("[WiFi] SR Enabled\n"));
    }
    else
    {
        sr_enab = WICED_FALSE;
        WPRINT_WWD_INFO(("[WiFi] SR Disabled\n"));
    }
    return WWD_SUCCESS;
}

wwd_result_t wwd_ensure_wlan_bus_is_up( void )
{
    /* Ensure HT clock is up */
    if ( bus_is_up == WICED_TRUE )
    {
        return WWD_SUCCESS;
    }
    //WPRINT_WWD_DEBUG(("[WiFi] 3->0\n"));

    if ( sr_enab == WICED_FALSE )
    {
        uint8_t csr = 0;
        uint32_t attempts = (uint32_t) WLAN_BUS_UP_ATTEMPTS;

        /* Bus specific wakeup routine */
        VERIFY_RESULT( wwd_bus_specific_wakeup( ));

 #if 1 // arm_cr4 version ext
        wwd_bus_write_backplane_value( (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, (uint32_t)0 );
        /* Force pad isolation off if possible (in case power never toggled) */
        wwd_bus_write_backplane_value( (uint32_t) SDIO_DEVICE_CONTROL, (uint8_t) 1, (uint32_t)0 );
#endif

        VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, (uint32_t) SBSDIO_HT_AVAIL_REQ | SBSDIO_FORCE_HT ) );

        do
        {
            VERIFY_RESULT( wwd_bus_sdio_read_register_value( BACKPLANE_FUNCTION, (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, &csr ) );
            --attempts;
        }
        while ( ( ( csr & SBSDIO_HT_AVAIL ) == 0 ) &&
                ( attempts != 0 ) &&
                ( host_rtos_delay_milliseconds( (uint32_t) 1 ), 1==1 ) );

        if (attempts == 0)
        {
            WPRINT_WWD_ERROR(("[WiFi] %s: WWD_SDIO_BUS_UP_FAIL\n", __FUNCTION__));
            return WWD_SDIO_BUS_UP_FAIL;
        }
        else
        {
#if 1 // arm_cr4 version
            /* Send misc interrupt to indicate OOB not needed */
            wwd_bus_write_backplane_value( (uint32_t) SDIO_TO_SB_MAILBOX_DATA, (uint8_t) 1, (uint32_t)0 );
            wwd_bus_write_backplane_value( (uint32_t) SDIO_TO_SB_MAILBOX, (uint8_t) 1, (uint32_t)SMB_DEV_INT );
#endif
            bus_is_up = WICED_TRUE;
            return WWD_SUCCESS;
        }
    }
    else
    {
        if ( wwd_kso_enable( WICED_TRUE ) == WWD_SUCCESS )
        {
            bus_is_up = WICED_TRUE;
            return WWD_SUCCESS;
        }
        else
        {
            WPRINT_WWD_ERROR(("[WiFi] %s: WWD_SDIO_BUS_UP_FAIL\n", __FUNCTION__));
            return WWD_SDIO_BUS_UP_FAIL;
        }
    }
}

wwd_result_t wwd_allow_wlan_bus_to_sleep( void )
{
    /* Clear HT clock request */
    if ( bus_is_up == WICED_TRUE )
    {
       // WPRINT_WWD_DEBUG(("[WiFi] 0->3\n"));

        bus_is_up = WICED_FALSE;
        if ( sr_enab == WICED_FALSE )
        {
#if 1 // arm_cr4 version
            /* Tell device to start using OOB wakeup */
            wwd_bus_write_backplane_value( (uint32_t) SDIO_TO_SB_MAILBOX, (uint8_t) 1, (uint32_t)SMB_USE_OOB );
            /* Turn off our contribution to the HT clock request */
            //wwd_bus_write_backplane_value( (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, 0 );
#endif
#if 1 // arm_cr4 version ext
            wwd_bus_write_backplane_value( (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, (uint32_t)SBSDIO_FORCE_HW_CLKREQ_OFF );
            /* Isolate the bus */
            wwd_bus_write_backplane_value( (uint32_t) SDIO_DEVICE_CONTROL, (uint8_t) 1, (uint32_t)SBSDIO_DEVCTL_PADS_ISO );
#endif
#if 0 // original
            VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, 0 ));
#endif

            /* Bus specific sleep routine */
            return wwd_bus_specific_sleep( );
        }
        else
        {
           return wwd_kso_enable( WICED_FALSE );
        }
    }
    else
    {
        return WWD_SUCCESS;
    }
}

static wwd_result_t wwd_kso_enable (wiced_bool_t enable)
{
    uint8_t wr_val = 0, rd_val = 0, cmp_val, bmask;
    uint32_t attempts = ( uint32_t ) MAX_KSO_ATTEMPTS;
    wwd_result_t result;

    if ( enable == WICED_TRUE )
    {
        wr_val |= SBSDIO_SLPCSR_KEEP_SDIO_ON;
    }

    /* 1st KSO write goes to AOS wake up core if device is asleep  */
    /* Possibly device might not respond to this cmd. So, don't check return value here */
    wwd_bus_write_register_value( BACKPLANE_FUNCTION, (uint32_t) SDIO_SLEEP_CSR, (uint8_t) 1, wr_val );

    if ( enable ) {
        /* device WAKEUP through KSO:
         * write bit 0 & read back until
         * both bits 0(kso bit) & 1 (dev on status) are set
         */
        cmp_val = SBSDIO_SLPCSR_KEEP_SDIO_ON |  SBSDIO_SLPCSR_DEVICE_ON;
        bmask = cmp_val;
    }
    else
    {
        /* Put device to sleep, turn off  KSO  */
        cmp_val = 0;
        /* Check for bit0 only, bit1(devon status) may not get cleared right away */
        bmask = SBSDIO_SLPCSR_KEEP_SDIO_ON;
    }

    do {
        /* Reliable KSO bit set/clr:
         * Sdiod sleep write access appears to be in sync with PMU 32khz clk
         * just one write attempt may fail,(same is with read ?)
         * in any case, read it back until it matches written value
         */
        result = wwd_bus_sdio_read_register_value( BACKPLANE_FUNCTION, (uint32_t) SDIO_SLEEP_CSR, (uint8_t) 1, &rd_val );
        if ( ( ( rd_val & bmask ) == cmp_val ) && ( result == WWD_SUCCESS ) )
        {
            break;
        }

       // host_rtos_delay_useconds( (uint32_t) KSO_WAIT_MS );
        DelayMs((uint32_t) KSO_WAIT_MS);
        wwd_bus_write_register_value( BACKPLANE_FUNCTION, (uint32_t) SDIO_SLEEP_CSR, (uint8_t) 1, wr_val );
        attempts--;
    } while ( attempts != 0 );

#ifdef WPRINT_ENABLE_WWD_DEBUG
    if ( MAX_KSO_ATTEMPTS - attempts > 2 )
    {
        WPRINT_WWD_DEBUG(("[WiFi] %s: Try %d times to %s\n",
            __FUNCTION__, MAX_KSO_ATTEMPTS - attempts, (enable ? "KSO_SET":"KSO_CLR")));
    }
#endif

    if ( attempts == 0 )
    {
        WPRINT_WWD_ERROR(("[WiFi] %s: WWD_SDIO_BUS_UP_FAIL\n", __FUNCTION__));
        return WWD_SDIO_BUS_UP_FAIL;;
    }
    else
    {
        return WWD_SUCCESS;
    }
}

#endif/*ap6255*/


