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
 *  Implements user functions for controlling the Wi-Fi system
 *
 *  This file provides end-user functions which allow actions such as scanning for
 *  Wi-Fi networks, joining Wi-Fi networks, getting the MAC address, etc
 *
 */

#include "BspConfig.h"
#ifdef    _DRIVER_WIFI__

#define NOT_INCLUDE_OTHER

#include <string.h> /* For strlen, stricmp, memcpy. memset */
#include <stddef.h>
#include "wwd_management.h"
#include "wwd_wifi.h"
#include "wwd_events.h"
#include "wwd_assert.h"
#include "wwd_wlioctl.h"
#include "wwd_debug.h"
#include "platform/wwd_platform_interface.h"
#include "wwd_buffer_interface.h"
#include "network/wwd_network_constants.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_sdpcm.h"
#include "wwd_bcmendian.h"
#include "wwd_ap.h"
#include "wwd_internal.h"
#include "wwd_bus_protocol.h"
#include "wiced_utilities.h"




//#pragma arm section code = "ap6181wifiCode", rodata = "ap6181wifiCode", rwdata = "ap6181wifidata", zidata = "ap6181wifidata"

/******************************************************
 * @cond       Constants
 ******************************************************/

#define SSID_MAX_LEN (32)

/* These are the flags in the BSS Capability Information field as defined in section 7.3.1.4 of IEEE Std 802.11-2007  */
#define DOT11_CAP_ESS                     (0x0001)   /** Extended service set capability */
#define DOT11_CAP_IBSS                    (0x0002)   /** Ad-hoc capability (Independent Basic Service Set) */
#define DOT11_CAP_PRIVACY                 (0x0010)   /** Privacy subfield - indicates data confidentiality is required for all data frames exchanged */
#define WL_CHANSPEC_CHAN_MASK             (0x00ff)
#define LEGACY_WL_BSS_INFO_VERSION           (107)   /** older version of wl_bss_info struct */

#define WICED_CREDENTIAL_TEST_TIMEOUT     (1500)

#define WPA_OUI_TYPE1                     "\x00\x50\xF2\x01"   /** WPA OUI */

#define MAX_SUPPORTED_MCAST_ENTRIES   (10)

#define WLC_EVENT_MSG_LINK      (0x01)

#define JOIN_ASSOCIATED             (uint32_t)(1 << 0)
#define JOIN_AUTHENTICATED          (uint32_t)(1 << 1)
#define JOIN_LINK_READY             (uint32_t)(1 << 2)
#define JOIN_SECURITY_COMPLETE      (uint32_t)(1 << 3)
#define JOIN_COMPLETE               (uint32_t)(1 << 4)
#define JOIN_NO_NETWORKS            (uint32_t)(1 << 5)

#define DEFAULT_JOIN_TIMEOUT      (7000)

#ifndef DEFAULT_PM2_SLEEP_RET_TIME
#define DEFAULT_PM2_SLEEP_RET_TIME   (40)
#endif

#define NULL_FRAMES_WITH_PM_SET_LIMIT ( 100 )

#define VALID_SECURITY_MASK    0x00FFFFFF

#define CHECK_IOCTL_BUFFER( buff )  if ( buff == NULL ) {  wiced_assert("Allocation failed\n", 0 == 1); return WWD_BUFFER_ALLOC_FAIL; }
#define CHECK_RETURN( expr )  { wwd_result_t check_res = (expr); if ( check_res != WWD_SUCCESS ) { wiced_assert("Command failed\n", 0 == 1); return check_res; } }
#define RETURN_WITH_ASSERT( expr )  { wwd_result_t check_res = (expr); wiced_assert("Command failed\n", check_res == WWD_SUCCESS ); return check_res; }


#define PACKET_FILTER_LIST_BUFFER_MAX_LEN ( WICED_LINK_MTU - sizeof(IOVAR_STR_PKT_FILTER_LIST) - IOCTL_OFFSET )

/** @endcond */

/******************************************************
 *             Local Structures
 ******************************************************/

#pragma pack(1)

typedef struct
{
    uint32_t    entry_count;
    wiced_mac_t macs[1];
} mcast_list_t;

typedef struct
{
    uint32_t cfg;
    uint32_t val;
} bss_setbuf_t;

typedef struct
{
    int32_t     value;
    wiced_mac_t mac_address;
} client_rssi_t;


#pragma pack()


/******************************************************
 *             Static Variables
 ******************************************************/

static wiced_scan_result_callback_t   scan_result_callback = NULL;
static wiced_scan_result_t**          wwd_scan_result_ptr;

static uint32_t              wiced_join_status[3];
const wwd_event_num_t        join_events[]  = { WLC_E_SET_SSID, WLC_E_LINK, WLC_E_AUTH, WLC_E_DEAUTH_IND, WLC_E_DISASSOC_IND, WLC_E_PSK_SUP, WLC_E_ROAM, WLC_E_NONE };
static const wwd_event_num_t scan_events[]  = { WLC_E_ESCAN_RESULT, WLC_E_NONE };

static uint8_t  wiced_wifi_powersave_mode = NO_POWERSAVE_MODE;
static uint16_t wiced_wifi_return_to_sleep_delay;

/* Note: monitor_mode_enabled variable is accessed by SDPCM */
static wiced_bool_t wwd_sdpcm_monitor_mode_enabled = WICED_FALSE;

wiced_bool_t wwd_wifi_p2p_go_is_up = WICED_FALSE;

/* Values are in 100's of Kbit/sec (1 = 100Kbit/s). Arranged as:
 * [Bit index]
 *    [0] = 20Mhz only
 *       [0] = Long GI
 *       [1] = Short GI
 *    [1] = 40MHz support
 *       [0] = Long GI
 *       [1] = Short GI
 */
static const uint16_t mcs_data_rate_lookup_table[32][2][2] =
{
    [0 ] = { {  65  ,   72  },  {   135 ,   150 } },
    [1 ] = { {  130 ,   144 },  {   270 ,   300 } },
    [2 ] = { {  195 ,   217 },  {   405 ,   450 } },
    [3 ] = { {  260 ,   289 },  {   540 ,   600 } },
    [4 ] = { {  390 ,   433 },  {   810 ,   900 } },
    [5 ] = { {  520 ,   578 },  {   1080,   1200} },
    [6 ] = { {  585 ,   650 },  {   1215,   1350} },
    [7 ] = { {  650 ,   722 },  {   1350,   1500} },
    [8 ] = { {  130 ,   144 },  {   270 ,   300 } },
    [9 ] = { {  260 ,   289 },  {   540 ,   600 } },
    [10] = { {  390 ,   433 },  {   810 ,   900 } },
    [11] = { {  520 ,   578 },  {   1080,   1200} },
    [12] = { {  780 ,   867 },  {   1620,   1800} },
    [13] = { {  1040,   1156},  {   2160,   2400} },
    [14] = { {  1170,   1300},  {   2430,   2700} },
    [15] = { {  1300,   1444},  {   2700,   3000} },
    [16] = { {  195 ,   217 },  {   405 ,   450 } },
    [17] = { {  390 ,   433 },  {   810 ,   900 } },
    [18] = { {  585 ,   650 },  {   1215,   1350} },
    [19] = { {  780 ,   867 },  {   1620,   1800} },
    [20] = { {  1170,   1300},  {   2430,   2700} },
    [21] = { {  1560,   1733},  {   3240,   3600} },
    [22] = { {  1755,   1950},  {   3645,   4050} },
    [23] = { {  1950,   2167},  {   4050,   4500} },
    [24] = { {  260 ,   288 },  {   540 ,   600 } },
    [25] = { {  520 ,   576 },  {   1080,   1200} },
    [26] = { {  780 ,   868 },  {   1620,   1800} },
    [27] = { {  1040,   1156},  {   2160,   2400} },
    [28] = { {  1560,   1732},  {   3240,   3600} },
    [29] = { {  2080,   2312},  {   4320,   4800} },
    [30] = { {  2340,   2600},  {   4860,   5400} },
    [31] = { {  2600,   2888},  {   5400,   6000} },
};

/* Note that the qos_map is accessed by SDPCM */
uint8_t wwd_tos_map[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };

int  wifi_scan_doing = 0;
extern  int wifi_set_scan_handle;
/******************************************************
 *             Static Function prototypes
 ******************************************************/

static inline /*@null@*/ tlv8_header_t* wlu_parse_tlvs              ( const tlv8_header_t* tlv_buf, uint32_t buflen, dot11_ie_id_t key );
static            wiced_bool_t    wlu_is_wpa_ie                     ( vendor_specific_ie_header_t* wpaie, tlv8_header_t** tlvs, uint32_t* tlvs_len );
static /*@null@*/ void*           wiced_join_events_handler         ( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data );
static            void*           wwd_scan_result_handler           ( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data );
static            wwd_result_t    wwd_wifi_prepare_join             ( wwd_interface_t interface, wiced_security_t security, /*@unique@*/ const uint8_t* security_key, uint8_t key_length, host_semaphore_type_t* semaphore );
static            wwd_result_t    wwd_wifi_get_packet_filters_inner ( uint32_t max_count, uint32_t offset, wiced_packet_filter_t* list, wiced_bool_t enabled_list, uint32_t* count_out );
static            wwd_result_t    wwd_wifi_set_band_specific_rate   ( wwd_interface_t interface, uint32_t rate );
static            wwd_result_t    wwd_wifi_check_join_status        ( wwd_interface_t interface );

/******************************************************
 *             Function definitions
 ******************************************************/

/*
 * NOTE: search references of function wlu_get in wl/exe/wlu.c to find what format the returned IOCTL data is.
 */

wwd_result_t wwd_wifi_scan( wiced_scan_type_t                              scan_type,
                            wiced_bss_type_t                               bss_type,
                            /*@null@*/ const wiced_ssid_t*                 optional_ssid,
                            /*@null@*/ const wiced_mac_t*                  optional_mac,
                            /*@null@*/ /*@unique@*/ const uint16_t*        optional_channel_list,
                            /*@null@*/ const wiced_scan_extended_params_t* optional_extended_params,
                            wiced_scan_result_callback_t                   callback,
                            wiced_scan_result_t**                          result_ptr,
                            /*@null@*/ void*                               user_data,
                            wwd_interface_t                                interface
                          )
{
    wiced_buffer_t     buffer;
    wl_escan_params_t* scan_params;
    wwd_result_t     retval;
    uint16_t           param_size        = offsetof( wl_escan_params_t, params ) + WL_SCAN_PARAMS_FIXED_SIZE;
    uint16_t           channel_list_size = 0;


    wiced_assert("Bad args", callback != NULL);

    /* Determine size of channel_list, and add it to the parameter size so correct sized buffer can be allocated */
    if ( optional_channel_list != NULL )
    {
        for ( channel_list_size = 0; optional_channel_list[channel_list_size] != 0; channel_list_size++ )
        {
        }
        param_size = (uint16_t) ( param_size + channel_list_size * sizeof(uint16_t) );
    }

    CHECK_RETURN( wwd_management_set_event_handler( scan_events, wwd_scan_result_handler, user_data, interface ) );
    wifi_scan_doing = 1;
    wifi_set_scan_handle = 0;
    rk_printf("scan start");
    /* Allocate a buffer for the IOCTL message */
    scan_params = (wl_escan_params_t*) wwd_sdpcm_get_iovar_buffer( &buffer, param_size, IOVAR_STR_ESCAN );
    CHECK_IOCTL_BUFFER( scan_params );

    /* Clear the scan parameters structure */
    memset( scan_params, 0, sizeof(wl_escan_params_t) );

    /* Fill in the appropriate details of the scan parameters structure */
    scan_params->version          = htod32(ESCAN_REQ_VERSION);
    scan_params->action           = htod16(WL_SCAN_ACTION_START);
    scan_params->params.scan_type = (int8_t) scan_type;
    scan_params->params.bss_type  = (int8_t) bss_type;

    /* Fill out the SSID parameter if provided */
    if ( optional_ssid != NULL )
    {
        scan_params->params.ssid.SSID_len = optional_ssid->length;
        memcpy( scan_params->params.ssid.SSID, optional_ssid->value, scan_params->params.ssid.SSID_len );
    }

    /* Fill out the BSSID parameter if provided */
    if ( optional_mac != NULL )
    {
        memcpy( scan_params->params.bssid.octet, optional_mac, sizeof(wiced_mac_t) );
    }
    else
    {
        memset( scan_params->params.bssid.octet, 0xff, sizeof(wiced_mac_t) );
    }

    /* Fill out the extended parameters if provided */
    if ( optional_extended_params != NULL )
    {
        scan_params->params.nprobes      = optional_extended_params->number_of_probes_per_channel;
        scan_params->params.active_time  = optional_extended_params->scan_active_dwell_time_per_channel_ms;
        scan_params->params.passive_time = optional_extended_params->scan_passive_dwell_time_per_channel_ms;
        scan_params->params.home_time    = optional_extended_params->scan_home_channel_dwell_time_between_channels_ms;
    }
    else
    {
        scan_params->params.nprobes      = (int32_t) -1;
        scan_params->params.active_time  = (int32_t) -1;
        scan_params->params.passive_time = (int32_t) -1;
        scan_params->params.home_time    = (int32_t) -1;
    }

    /* Copy the channel list parameter if provided */
    if ( ( channel_list_size > 0 ) && ( optional_channel_list != NULL ) )
    {
        memcpy( scan_params->params.channel_list, optional_channel_list, channel_list_size * sizeof(uint16_t) );
        scan_params->params.channel_num = (int32_t) channel_list_size;
    }

    scan_result_callback = callback;
    wwd_scan_result_ptr = result_ptr;

    /* Send the Incremental Scan IOVAR message - blocks until the response is received */
    retval = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, interface );

    /* Return the success of the IOCTL */
    return retval;
}


wwd_result_t wwd_wifi_abort_scan( void )
{
    wiced_buffer_t     buffer;
    wl_escan_params_t* scan_params;
    wwd_result_t       result;

    /* Allocate a buffer for the IOCTL message */
    scan_params = (wl_escan_params_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_escan_params_t), IOVAR_STR_ESCAN );
    CHECK_IOCTL_BUFFER( scan_params );

    /* Clear the scan parameters structure */
    memset( scan_params, 0, sizeof(wl_escan_params_t) );

    /* Fill in the appropriate details of the scan parameters structure */
    scan_params->version = ESCAN_REQ_VERSION;
    scan_params->action  = WL_SCAN_ACTION_ABORT;

    /* Send the Scan IOVAR message to abort scan*/
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );

    return result;
}

/** Handles scan result events
 *
 *  This function receives scan record events, and parses them into a better format, then passes the results
 *  to the user application.
 *
 * @param event_header     : The event details
 * @param event_data       : The data for the event which contains the scan result structure
 * @param handler_user_data: data which will be passed to user application
 *
 * @returns : handler_user_data parameter
 *
 */
static void* wwd_scan_result_handler( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data )
{
    wiced_scan_result_t*     record;
    wl_escan_result_t*       eresult;
    wl_bss_info_t*           bss_info;
    uint16_t                 chanspec;
    uint32_t                 version;
    tlv8_header_t*           cp;
    uint32_t                 len;
    uint16_t                 ie_offset;
    uint32_t                 bss_info_length;
    rsn_ie_fixed_portion_t*  rsnie;
    wpa_ie_fixed_portion_t*  wpaie = NULL;
    uint8_t                  rate_num;
    dsss_parameter_set_ie_t* dsie;
    ht_capabilities_ie_t*    ht_capabilities_ie;
    uint32_t                 count_tmp = 0;
    uint16_t                 temp16;




    if ( scan_result_callback == NULL)
    {
        return handler_user_data;
    }

    if ( event_header->status == WLC_E_STATUS_SUCCESS)
    {
        scan_result_callback( NULL, handler_user_data, WICED_SCAN_COMPLETED_SUCCESSFULLY );
        scan_result_callback = NULL;
        wifi_scan_doing = 0;
        rk_printf("scan over");
        return handler_user_data;
    }
    if ( event_header->status == WLC_E_STATUS_NEWSCAN ||
         event_header->status == WLC_E_STATUS_NEWASSOC ||
         event_header->status == WLC_E_STATUS_ABORT )
    {
        scan_result_callback( NULL, handler_user_data, WICED_SCAN_ABORTED );
        scan_result_callback = NULL;
        wifi_scan_doing = 0;
        rk_printf("scan over");
        return handler_user_data;
    }

    if ( event_header->status != WLC_E_STATUS_PARTIAL )
    {
        return handler_user_data;
    }

    if ( wwd_scan_result_ptr == NULL )
    {
        scan_result_callback( (wiced_scan_result_t**)event_data, handler_user_data, WICED_SCAN_INCOMPLETE );
        return handler_user_data;
    }

    record = (wiced_scan_result_t*) ( *wwd_scan_result_ptr );
    eresult = (wl_escan_result_t*) event_data;
    bss_info = &eresult->bss_info[0];

    wiced_assert( "More than one result returned by firmware", eresult->bss_count == 1 );
    if (eresult->bss_count != 1)
        return handler_user_data;

    /* Copy the SSID into the output record structure */
    record->SSID.length = bss_info->SSID_len;
    memset( record->SSID.value, 0, (size_t) SSID_MAX_LEN );
    memcpy( record->SSID.value, bss_info->SSID, record->SSID.length );
    //printf("SSID = %s\n",record->SSID.value);
    /* Copy the BSSID into the output record structure */
    memcpy( (void*) record->BSSID.octet, (const void*) bss_info->BSSID.octet, sizeof(wiced_mac_t) );

    /* Copy the RSSI into the output record structure */
    record->signal_strength = (int16_t)(WICED_READ_16(&bss_info->RSSI));
    record->on_channel = WICED_TRUE;
    //printf("signal_strength = %d\n",record->signal_strength);
    /* Find maximum data rate and put it in the output record structure */
    record->max_data_rate = 0;
    count_tmp = WICED_READ_32(&bss_info->rateset.count);
    if ( count_tmp > 16 )
    {
        count_tmp = 16;
    }

    for ( rate_num = 0; rate_num < count_tmp; rate_num++ )
    {
        uint32_t rate = ( bss_info->rateset.rates[rate_num] & 0x7f ) * (unsigned int) 500;
        if ( record->max_data_rate < rate )
        {
            record->max_data_rate = rate;
        }
    }

    /* Write the BSS type into the output record structure */
    record->bss_type = ( ( bss_info->capability & DOT11_CAP_ESS  ) != 0 ) ? WICED_BSS_TYPE_INFRASTRUCTURE :
                       ( ( bss_info->capability & DOT11_CAP_IBSS ) != 0 ) ? WICED_BSS_TYPE_ADHOC :
                                                                            WICED_BSS_TYPE_UNKNOWN;

    /* Determine the network security.
     * Some of this section has been copied from the standard broadcom host driver file wl/exe/wlu.c function wl_dump_wpa_rsn_ies
     */

    ie_offset = WICED_READ_16(&bss_info->ie_offset);
    cp  = (tlv8_header_t*) ( ( (uint8_t*) bss_info ) + ie_offset );
    len = WICED_READ_32(&bss_info->ie_length);
    bss_info_length = WICED_READ_32(&bss_info->length);

    /* Validate the length of the IE section */
    if ( ( ie_offset > bss_info_length ) ||
         ( len > bss_info_length - ie_offset ) )
    {
        wiced_assert( "Invalid ie length", 1 == 0 );
        return handler_user_data;
    }

    /* Find an RSN IE (Robust-Security-Network Information-Element) */
    rsnie = (rsn_ie_fixed_portion_t*) wlu_parse_tlvs( cp, len, DOT11_IE_ID_RSN );

    /* Find a WPA IE */
    if ( rsnie == NULL )
    {
        tlv8_header_t*  parse = cp;
        uint32_t        parse_len = len;
        while ( ( wpaie = (wpa_ie_fixed_portion_t*) wlu_parse_tlvs( parse, parse_len, DOT11_IE_ID_VENDOR_SPECIFIC ) ) != 0 )
        {
            if ( wlu_is_wpa_ie( (vendor_specific_ie_header_t*) wpaie, &parse, &parse_len ) != WICED_FALSE )
            {
                break;
            }
        }
    }

    temp16 = WICED_READ_16(&bss_info->capability);

    /* Check if AP is configured for WPA2 */
    if ( ( rsnie != NULL ) &&
         ( rsnie->tlv_header.length >= RSN_IE_MINIMUM_LENGTH + rsnie->pairwise_suite_count * sizeof(uint32_t) ) )
    {
        uint16_t a;
        uint32_t group_key_suite;
        record->security = (wiced_security_t)WPA2_SECURITY;

        group_key_suite = NTOH32( rsnie->group_key_suite ) & 0xFF;
        /* Check the RSN contents to see if there are any references to TKIP cipher (2) in the group key or pairwise keys. If so it must be mixed mode. */
        if ( group_key_suite == (uint32_t) WICED_CIPHER_TKIP )
        {
            record->security |= TKIP_ENABLED;
        }
        if ( group_key_suite == (uint32_t) WICED_CIPHER_CCMP_128 )
        {
            record->security |= AES_ENABLED;
        }

        for ( a = 0; a < rsnie->pairwise_suite_count; ++a )
        {
            uint32_t pairwise_suite_list_item = NTOH32( rsnie->pairwise_suite_list[a] ) & 0xFF;
            if ( pairwise_suite_list_item == (uint32_t) WICED_CIPHER_TKIP )
            {
                record->security |= TKIP_ENABLED;
            }

            if ( pairwise_suite_list_item == (uint32_t) WICED_CIPHER_CCMP_128 )
            {
                record->security |= AES_ENABLED;
            }
        }

        if (record->security == WPA2_SECURITY)
        {
            record->security = WICED_SECURITY_UNKNOWN;
        }
        else
        {
            akm_suite_portion_t* akm_suites;
            akm_suites = (akm_suite_portion_t*) &(rsnie->pairwise_suite_list[rsnie->pairwise_suite_count]);
            for ( a = 0; a < akm_suites->akm_suite_count; ++a )
            {
                uint32_t akm_suite_list_item = NTOH32(akm_suites->akm_suite_list[a]) & 0xFF;
                if ( akm_suite_list_item == (uint32_t) WICED_AKM_8021X )
                {
                    record->security |= ENTERPRISE_ENABLED;
                }
            }
        }
    }
    /* Check if AP is configured for WPA */
    else if ( ( wpaie != NULL ) &&
              ( wpaie->vendor_specific_header.tlv_header.length >= WPA_IE_MINIMUM_LENGTH + wpaie->unicast_suite_count * sizeof(uint32_t) ) )
    {
        uint16_t a;
        uint32_t group_key_suite;
        akm_suite_portion_t* akm_suites;

        record->security = WPA_SECURITY;
        group_key_suite = NTOH32( wpaie->multicast_suite ) & 0xFF;
        if ( group_key_suite == (uint32_t) WICED_CIPHER_TKIP )
        {
            record->security |= TKIP_ENABLED;
        }
        if ( group_key_suite == (uint32_t) WICED_CIPHER_CCMP_128 )
        {
            record->security |= AES_ENABLED;
        }

        akm_suites = (akm_suite_portion_t*) &(wpaie->unicast_suite_list[wpaie->unicast_suite_count]);
        for ( a = 0; a < akm_suites->akm_suite_count; ++a )
        {
            uint32_t akm_suite_list_item = NTOH32(akm_suites->akm_suite_list[a]) & 0xFF;
            if ( akm_suite_list_item == (uint32_t) WICED_AKM_8021X )
            {
                record->security |= ENTERPRISE_ENABLED;
            }
        }

        for ( a = 0; a < wpaie->unicast_suite_count; ++a )
        {
            if ( wpaie->unicast_suite_list[a][3] == (uint32_t) WICED_CIPHER_CCMP_128 )
            {
                record->security |= AES_ENABLED;
            }
        }
    }
    /* Check if AP is configured for WEP, that is, if the capabilities field indicates privacy, then security supports WEP */
    else if ( ( temp16 & DOT11_CAP_PRIVACY ) != 0 )
    {
        record->security = WICED_SECURITY_WEP_PSK;
    }
    else
    {
        /* Otherwise no security */
        record->security = WICED_SECURITY_OPEN;
    }
    //printf("record->security = 0x%04x\n",record->security);
    /* Update the maximum data rate with 11n rates from the HT Capabilities IE */
    ht_capabilities_ie = (ht_capabilities_ie_t*)wlu_parse_tlvs( cp, len, DOT11_IE_ID_HT_CAPABILITIES );
    if ( ( ht_capabilities_ie != NULL ) &&
         ( ht_capabilities_ie->tlv_header.length == HT_CAPABILITIES_IE_LENGTH ) )
    {
        uint8_t a;
        uint8_t supports_40mhz =   ( ht_capabilities_ie->ht_capabilities_info & HT_CAPABILITIES_INFO_SUPPORTED_CHANNEL_WIDTH_SET ) != 0 ? 1 : 0;
        uint8_t short_gi[2]    = { ( ht_capabilities_ie->ht_capabilities_info & HT_CAPABILITIES_INFO_SHORT_GI_FOR_20MHZ          ) != 0 ? 1 : 0,
                                   ( ht_capabilities_ie->ht_capabilities_info & HT_CAPABILITIES_INFO_SHORT_GI_FOR_40MHZ          ) != 0 ? 1 : 0 };

        /* Find highest bit from MCS info */
        for (a = 31; a != 0xFF; --a)
        {
            if ( ( ht_capabilities_ie->rx_mcs[a / 8] & ( 1 << ( a % 8 ) )) != 0 )
            {
                break;
            }
        }
        if (a != 0xFF)
        {
            record->max_data_rate = 100UL * mcs_data_rate_lookup_table[a][supports_40mhz][short_gi[supports_40mhz]];
        }
    }

    /* Get the channel */
    chanspec        = WICED_READ_16( &bss_info->chanspec );
    version         = WICED_READ_32( &bss_info->version );
    record->channel = ( (uint8_t) ( chanspec & WL_CHANSPEC_CHAN_MASK ) );

    /* Old WLAN firmware reported off channel probe responses - parse the response to check the channel */
    dsie =  (dsss_parameter_set_ie_t*) wlu_parse_tlvs( cp, len, DOT11_IE_ID_DSSS_PARAMETER_SET );
    if ( ( dsie != NULL ) &&
         ( dsie->tlv_header.length == DSSS_PARAMETER_SET_LENGTH ) &&
         ( record->channel != dsie->current_channel ) )
    {
        /* Received an off channel report */
        record->channel    = dsie->current_channel;
        record->on_channel = WICED_FALSE;
    }

    if ( ( version != (uint32_t) LEGACY_WL_BSS_INFO_VERSION ) && ( bss_info->n_cap != 0 ) )
    {
        record->band = ( ( chanspec & WL_CHANSPEC_BAND_MASK ) == (uint16_t) WL_CHANSPEC_BAND_2G ) ? WICED_802_11_BAND_2_4GHZ : WICED_802_11_BAND_5GHZ;
    }
    else
    {
        record->band = WICED_802_11_BAND_2_4GHZ;
    }

    scan_result_callback( wwd_scan_result_ptr, handler_user_data, WICED_SCAN_INCOMPLETE );
    if ( *wwd_scan_result_ptr == NULL )
    {
#if 0
        wwd_management_set_event_handler( scan_events, NULL, NULL );
#endif /* if 0 */
    }

    return handler_user_data;
}


wwd_result_t wwd_wifi_join( const wiced_ssid_t* ssid, wiced_security_t auth_type, const uint8_t* security_key, uint8_t key_length, host_semaphore_type_t* semaphore )
{
    wiced_buffer_t        buffer;
    host_semaphore_type_t join_sema;
    wwd_result_t          result;
    wlc_ssid_t*           ssid_params;

    if ( ssid->length > (size_t) SSID_MAX_LEN )
    {
        WPRINT_WWD_DEBUG(("wiced_wifi_join: SSID too long\n"));
        return WWD_WLAN_BADSSIDLEN;
    }

    /* Keep WLAN awake while joining */
    WWD_WLAN_KEEP_AWAKE( );

    if ( wwd_wifi_set_block_ack_window_size( WWD_STA_INTERFACE ) != WWD_SUCCESS )
    {
        return WWD_SET_BLOCK_ACK_WINDOW_FAIL;
    }

    if ( semaphore == NULL )
    {
        CHECK_RETURN( host_rtos_init_semaphore( &join_sema ) );

        result = wwd_wifi_prepare_join( WWD_STA_INTERFACE, auth_type, security_key, key_length, &join_sema );
    }
    else
    {
        result = wwd_wifi_prepare_join( WWD_STA_INTERFACE, auth_type, security_key, key_length, semaphore );
    }
    if ( result == WWD_SUCCESS )
    {
        /* Join network */
        ssid_params = (struct wlc_ssid *) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wlc_ssid_t) );
        CHECK_IOCTL_BUFFER( ssid_params );
        memset( ssid_params, 0, sizeof(wlc_ssid_t) );
        ssid_params->SSID_len = ssid->length;
        memcpy( ssid_params->SSID, ssid->value, ssid_params->SSID_len );
        result = wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_SSID, buffer, 0, WWD_STA_INTERFACE );

        if ( result == WWD_SUCCESS && semaphore == NULL )
        {
            result = host_rtos_get_semaphore( &join_sema, DEFAULT_JOIN_TIMEOUT, WICED_FALSE );
            wiced_assert( "Get semaphore failed", ( result == WWD_SUCCESS ) || ( result == WWD_TIMEOUT ) );
            if ( wwd_wifi_is_ready_to_transceive( WWD_STA_INTERFACE ) != WWD_SUCCESS )
            {
                result = wwd_wifi_leave( WWD_STA_INTERFACE );
                wiced_assert( "Leave network failed", result == WWD_SUCCESS );
            }
            result = wwd_wifi_is_ready_to_transceive( WWD_STA_INTERFACE );
        }
    }
    if ( semaphore == NULL )
    {
        host_rtos_deinit_semaphore( &join_sema );
    }

    WWD_WLAN_LET_SLEEP( );
    return result;
}


wwd_result_t wwd_wifi_join_specific( const wiced_scan_result_t* ap, const uint8_t* security_key, uint8_t key_length, host_semaphore_type_t* semaphore, wwd_interface_t interface )
{
    wiced_buffer_t        buffer;
    host_semaphore_type_t join_semaphore;
    wwd_result_t          result;
    wl_extjoin_params_t*  ext_join_params;
    wl_join_params_t*     join_params;
    wiced_security_t      security = ap->security;

    /* Keep WLAN awake while joining */
    WWD_WLAN_KEEP_AWAKE( );

    if ( wwd_wifi_set_block_ack_window_size( interface ) != WWD_SUCCESS )
    {
        return WWD_SET_BLOCK_ACK_WINDOW_FAIL;
    }

    if ( ap->bss_type == WICED_BSS_TYPE_ADHOC )
    {
        security |= IBSS_ENABLED;
    }

    if ( semaphore == NULL )
    {
        CHECK_RETURN( host_rtos_init_semaphore( &join_semaphore ) );

        result = wwd_wifi_prepare_join( interface, security, security_key, key_length, &join_semaphore );
    }
    else
    {
        result = wwd_wifi_prepare_join( interface, security, security_key, key_length, semaphore );
    }
    if ( result == WWD_SUCCESS )
    {
        /* Check if soft AP is running, if so, move its current channel to the the destination AP */
        if ( wwd_wifi_is_ready_to_transceive( WWD_AP_INTERFACE ) == WWD_SUCCESS )
        {
            uint32_t current_softap_channel = 0;
            wwd_wifi_get_channel( WWD_AP_INTERFACE, &current_softap_channel );
            if ( current_softap_channel != ap->channel )
            {
                wwd_wifi_set_channel( WWD_AP_INTERFACE, ap->channel );
                host_rtos_delay_milliseconds( 100 );
            }
        }

        /* Join network */
        ext_join_params = (wl_extjoin_params_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_extjoin_params_t), "join" );
        CHECK_IOCTL_BUFFER( ext_join_params );
        memset( ext_join_params, 0, sizeof(wl_extjoin_params_t) );

        ext_join_params->ssid.SSID_len = ap->SSID.length;
        memcpy( ext_join_params->ssid.SSID, ap->SSID.value, ext_join_params->ssid.SSID_len );
        memcpy( &ext_join_params->assoc_params.bssid, &ap->BSSID, sizeof(wiced_mac_t) );
        ext_join_params->scan_params.scan_type    = 0;
        ext_join_params->scan_params.active_time  = -1;
        ext_join_params->scan_params.home_time    = -1;
        ext_join_params->scan_params.nprobes      = -1;
        ext_join_params->scan_params.passive_time = -1;
#ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
        ext_join_params->assoc_params.bssid_cnt = 0;
#endif /* ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS */
        ext_join_params->assoc_params.chanspec_num = (uint32_t) 1;
        ext_join_params->assoc_params.chanspec_list[0] = (wl_chanspec_t) htod16((ap->channel | WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE));
        result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, interface );

        /* Some firmware, e.g. for 4390, does not support the join IOVAR, so use the older IOCTL call instead */
        if ( result == WWD_WLAN_UNSUPPORTED )
        {
            join_params = (wl_join_params_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wl_join_params_t) );
            CHECK_IOCTL_BUFFER( join_params );
            memset( join_params, 0, sizeof(wl_join_params_t) );
            memcpy( &join_params->ssid, &ext_join_params->ssid, sizeof ( wlc_ssid_t ) );
            memcpy( &join_params->params.bssid, &ap->BSSID, sizeof(wiced_mac_t) );
#ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
            join_params->params.bssid_cnt = 0;
#endif
            join_params->params.chanspec_num = (uint32_t) 1;
            join_params->params.chanspec_list[0] = (wl_chanspec_t) htod16((ap->channel | WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE));
            result = wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_SSID, buffer, 0, interface );
        }

        if ( result == WWD_SUCCESS && semaphore == NULL )
        {
            host_rtos_get_semaphore( &join_semaphore, (uint32_t) DEFAULT_JOIN_TIMEOUT, WICED_FALSE );
            if ( wwd_wifi_is_ready_to_transceive( interface ) != WWD_SUCCESS )
            {
                wwd_wifi_leave( interface );
            }
            result = wwd_wifi_is_ready_to_transceive( interface );
        }
    }
    if ( semaphore == NULL )
    {
        host_rtos_deinit_semaphore( &join_semaphore );
    }

    WWD_WLAN_LET_SLEEP( );
    return result;
}

static wwd_result_t wwd_wifi_prepare_join( wwd_interface_t interface, wiced_security_t auth_type, /*@unique@*/ const uint8_t* security_key, uint8_t key_length, host_semaphore_type_t* semaphore )
{
    wiced_buffer_t buffer;
    wwd_result_t   retval;
    uint16_t       a;
    uint32_t*      wsec_setting;
    uint32_t*      data;
    uint32_t*      infra;
    uint32_t*      wpa_auth;
    uint32_t*      auth;

    if ( ( ( ( key_length > (uint8_t) WSEC_MAX_PSK_LEN ) ||
             ( key_length < (uint8_t) WSEC_MIN_PSK_LEN ) ) &&
           ( ( auth_type == WICED_SECURITY_WPA_TKIP_PSK ) ||
             ( auth_type == WICED_SECURITY_WPA_AES_PSK ) ||
             ( auth_type == WICED_SECURITY_WPA2_AES_PSK ) ||
             ( auth_type == WICED_SECURITY_WPA2_TKIP_PSK ) ||
             ( auth_type == WICED_SECURITY_WPA2_MIXED_PSK ) ) ) )
    {
#if 0
        WPRINT_WWD_DEBUG(("wiced_wifi_prepare_join: Security key invalid\n"));
#endif /* if 0 */
        return WWD_INVALID_KEY;
    }

    if ( !( ( interface == WWD_STA_INTERFACE ) || ( ( interface == WWD_P2P_INTERFACE ) && ( wwd_wifi_p2p_go_is_up == WICED_FALSE ) ) ) )
    {
        return WWD_INVALID_INTERFACE;
    }

    /* Clear the current join status */
    wiced_join_status[ interface ] = 0;

    /* Set Wireless Security Type */
    wsec_setting = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( wsec_setting );
    *wsec_setting = (uint32_t) ((auth_type&0xFF) & ~WPS_ENABLED);
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_WSEC, buffer, 0, interface ) );

    /* Set supplicant variable */
    data = wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, "bsscfg:"IOVAR_STR_SUP_WPA );
    CHECK_IOCTL_BUFFER( data );
    data[0] = interface;
    data[1] = (uint32_t) ( ( ( ( auth_type & WPA_SECURITY )  != 0 ) ||
                           ( ( auth_type & WPA2_SECURITY ) != 0 ) ) ? 1 : 0 );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, interface ) );

    /* Set the EAPOL version to whatever the AP is using (-1) */
    //rk change
    data = wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, "bsscfg:"IOVAR_STR_SUP_WPA2_EAPVER );
  //  data = wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_SUP_WPA2_EAPVER );
    CHECK_IOCTL_BUFFER( data );
    data[0] = interface;
    data[1] = (uint32_t)-1;
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, interface ) );

    /* Send WPA Key */
    switch ( auth_type )
    {
        case WICED_SECURITY_OPEN:
        case WICED_SECURITY_IBSS_OPEN:
        case WICED_SECURITY_WPS_OPEN:
        case WICED_SECURITY_WPS_SECURE:
            break;
        case WICED_SECURITY_WPA_TKIP_PSK:
        case WICED_SECURITY_WPA_AES_PSK:
        case WICED_SECURITY_WPA_MIXED_PSK:
        case WICED_SECURITY_WPA2_AES_PSK:
        case WICED_SECURITY_WPA2_TKIP_PSK:
        case WICED_SECURITY_WPA2_MIXED_PSK:
            CHECK_RETURN( wwd_wifi_set_passphrase( security_key, key_length, interface ) );
            break;

        case WICED_SECURITY_WPA_TKIP_ENT:
        case WICED_SECURITY_WPA_AES_ENT:
        case WICED_SECURITY_WPA_MIXED_ENT:
        case WICED_SECURITY_WPA2_TKIP_ENT:
        case WICED_SECURITY_WPA2_AES_ENT:
        case WICED_SECURITY_WPA2_MIXED_ENT:
            break;

        case WICED_SECURITY_WEP_PSK:
        case WICED_SECURITY_WEP_SHARED:
            for ( a = 0; a < key_length; a = (uint16_t) ( a + 2 + security_key[1] ) )
            {
                const wiced_wep_key_t* in_key = (const wiced_wep_key_t*) &security_key[a];
                wl_wsec_key_t* out_key = (wl_wsec_key_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wl_wsec_key_t) );
                CHECK_IOCTL_BUFFER( out_key );
                memset( out_key, 0, sizeof(wl_wsec_key_t) );
                out_key->index = in_key->index;
                out_key->len = in_key->length;
                memcpy( out_key->data, in_key->data, in_key->length );
                switch ( in_key->length )
                {
                    case 5:
                        out_key->algo = (uint32_t) CRYPTO_ALGO_WEP1;
                        break;
                    case 13:
                        out_key->algo = (uint32_t)CRYPTO_ALGO_WEP128;
                        break;
                    case 16:
                        /* default to AES-CCM */
                        out_key->algo = (uint32_t) CRYPTO_ALGO_AES_CCM;
                        break;
                    case 32:
                        out_key->algo = (uint32_t) CRYPTO_ALGO_TKIP;
                        break;
                    default:
                        host_buffer_release(buffer, WWD_NETWORK_TX);
                        return WWD_INVALID_KEY;
                }
                /* Set the first entry as primary key by default */
                if ( a == 0 )
                {
                    out_key->flags |= WL_PRIMARY_KEY;
                }
                out_key->index = htod32(out_key->index);
                out_key->len   = htod32(out_key->len);
                out_key->algo  = htod32(out_key->algo);
                out_key->flags = htod32(out_key->flags);
                CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_KEY, buffer, NULL, interface ) );
            }
            break;
        case WICED_SECURITY_FORCE_32_BIT:
        case WICED_SECURITY_UNKNOWN:
        default:
            wiced_assert("wiced_wifi_prepare_join: Unsupported security type\n", 0 != 0 );
            break;
    }

    /* Set infrastructure mode */
    infra = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( infra );
    *infra = (uint32_t) ( ( auth_type & IBSS_ENABLED ) == 0 )? 1 : 0;
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_INFRA, buffer, 0, interface ) );

    /* Set authentication type */
    auth = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( auth );
    if ( auth_type == WICED_SECURITY_WEP_SHARED )
    {
        *auth = 1; /* 1 = Shared Key authentication */
    }
    else
    {
        *auth = 0; /*  0 = Open System authentication */
    }
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_AUTH, buffer, 0, interface ) );

    /* Set WPA authentication mode */
    wpa_auth = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( wpa_auth );

    switch ( auth_type )
    {
        case WICED_SECURITY_IBSS_OPEN:
            wiced_join_status[interface] |= JOIN_AUTHENTICATED;  /* IBSS does not get authenticated onto an AP */
            /* intentional fall-thru */
            /* Disables Eclipse static analysis warning */
            /* no break */
        case WICED_SECURITY_OPEN:
        case WICED_SECURITY_WPS_OPEN:
        case WICED_SECURITY_WPS_SECURE:
            *wpa_auth = WPA_AUTH_DISABLED;
            wiced_join_status[interface] |= JOIN_SECURITY_COMPLETE;  /* Open Networks do not have to complete security */
            break;
        case WICED_SECURITY_WPA_TKIP_PSK:
        case WICED_SECURITY_WPA_AES_PSK:
        case WICED_SECURITY_WPA_MIXED_PSK:
            *wpa_auth = (uint32_t) WPA_AUTH_PSK;
            break;
        case WICED_SECURITY_WPA2_AES_PSK:
        case WICED_SECURITY_WPA2_TKIP_PSK:
        case WICED_SECURITY_WPA2_MIXED_PSK:
            *wpa_auth = (uint32_t) WPA2_AUTH_PSK;
            break;

        case WICED_SECURITY_WPA_TKIP_ENT:
        case WICED_SECURITY_WPA_AES_ENT:
        case WICED_SECURITY_WPA_MIXED_ENT:
            *wpa_auth = (uint32_t) WPA_AUTH_UNSPECIFIED;
            break;

        case WICED_SECURITY_WPA2_TKIP_ENT:
        case WICED_SECURITY_WPA2_AES_ENT:
        case WICED_SECURITY_WPA2_MIXED_ENT:
            *wpa_auth = (uint32_t) WPA2_AUTH_UNSPECIFIED;
            break;

        case WICED_SECURITY_WEP_PSK:
        case WICED_SECURITY_WEP_SHARED:
            *wpa_auth = WPA_AUTH_DISABLED;
            wiced_join_status[interface] |= JOIN_SECURITY_COMPLETE;
            break;
        case WICED_SECURITY_UNKNOWN:
        case WICED_SECURITY_FORCE_32_BIT:
        default:
            WPRINT_WWD_DEBUG(("Unsupported Security type\n"));
            *wpa_auth = WPA_AUTH_DISABLED;
            break;
    }

    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_WPA_AUTH, buffer, 0, interface ) );

    retval = wwd_management_set_event_handler( join_events, wiced_join_events_handler, (void*) semaphore, interface );
    wiced_assert("Set Event handler failed\n", retval == WWD_SUCCESS );

    return retval;
}


wwd_result_t wwd_wifi_leave( wwd_interface_t interface )
{
    wiced_buffer_t buffer;

    CHECK_RETURN( wwd_management_set_event_handler( join_events, NULL, NULL, interface ) );

    /* Disassociate from AP */
    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, 0 ) )

    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_DISASSOC, buffer, 0, interface ) );

    wiced_join_status[ interface ] = 0;
    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_deauth_sta( const wiced_mac_t* mac, wwd_dot11_reason_code_t reason, wwd_interface_t interface )
{
    wiced_buffer_t buffer;
    wwd_result_t result;
    scb_val_t* scb_val;

    scb_val = (scb_val_t *) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(scb_val_t) );
    CHECK_IOCTL_BUFFER( scb_val );
    memset((char *)scb_val, 0, sizeof(scb_val_t));
    memcpy((char *)&scb_val->ea, (char *) mac, sizeof(wiced_mac_t));
    scb_val->val = (uint32_t)reason;
    result = wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SCB_DEAUTHENTICATE_FOR_REASON, buffer, 0, interface );

    return result;
}

/** Callback for join events
 *  This is called when the WLC_E_SET_SSID event is received,
 *  indicating that the system has joined successfully.
 *  Wakes the thread which was doing the join, allowing it to resume.
 */
static /*@null@*/ void* wiced_join_events_handler( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data )
{
    host_semaphore_type_t* semaphore = (host_semaphore_type_t*) handler_user_data;
    wiced_bool_t join_complete = WICED_FALSE;

    UNUSED_PARAMETER(event_data);

    if ( event_header->interface != (uint8_t) WWD_STA_INTERFACE && event_header->interface != (uint8_t) WWD_P2P_INTERFACE )
    {
        return handler_user_data;
    }

    switch ( event_header->event_type )
    {
        case WLC_E_ROAM:
            if ( event_header->status == WLC_E_STATUS_SUCCESS )
            {
                wiced_join_status[event_header->interface] |= JOIN_LINK_READY;
            }
            break;

        case WLC_E_PSK_SUP:
            if ( event_header->status == WLC_SUP_KEYED )
            {
                /* Successful WPA key exchange */
                wiced_join_status[event_header->interface] |= JOIN_SECURITY_COMPLETE;
            }
            else if ( event_header->status != WLC_SUP_LAST_BASIC_STATE && event_header->status != WLC_SUP_KEYXCHANGE )
            {
                /* WPA PSK error - abort (usually means key was incorrect) */
                join_complete = WICED_TRUE;
            }
            break;

        case WLC_E_SET_SSID:
            if ( event_header->status == WLC_E_STATUS_SUCCESS )
            {
                /* SSID has been successfully set. */
                wiced_join_status[event_header->interface] |= JOIN_COMPLETE;
            }
            else if ( event_header->status == WLC_E_STATUS_NO_NETWORKS )
            {
                /* wiced_join_status = JOIN_NO_NETWORKS; */
                /* join_complete = WICED_TRUE; */
            }
            else
            {
                join_complete = WICED_TRUE;
            }
            break;

        case WLC_E_LINK:
            if ( ( event_header->flags & WLC_EVENT_MSG_LINK ) != 0 )
            {
                wiced_join_status[event_header->interface] |= JOIN_LINK_READY;
            }
            else
            {
                wiced_join_status[event_header->interface] &= ~JOIN_LINK_READY;
            }
            break;

        case WLC_E_DEAUTH_IND:
        case WLC_E_DISASSOC_IND:
            wiced_join_status[event_header->interface] &= ~JOIN_AUTHENTICATED;
            break;

        case WLC_E_AUTH:
            if ( event_header->status == WLC_E_STATUS_SUCCESS )
            {
                wiced_join_status[event_header->interface] |= JOIN_AUTHENTICATED;
            }
            else
            {
                /* We cannot authenticate. Perhaps we're blocked */
                join_complete = WICED_TRUE;
            }
            break;

        /* Note - These are listed to keep gcc pedantic checking happy */
        case WLC_E_NONE:
        case WLC_E_JOIN:
        case WLC_E_START:
        case WLC_E_AUTH_IND:
        case WLC_E_DEAUTH:
        case WLC_E_ASSOC:
        case WLC_E_ASSOC_IND:
        case WLC_E_REASSOC:
        case WLC_E_REASSOC_IND:
        case WLC_E_DISASSOC:
        case WLC_E_QUIET_START:
        case WLC_E_QUIET_END:
        case WLC_E_BEACON_RX:
        case WLC_E_MIC_ERROR:
        case WLC_E_NDIS_LINK:
        case WLC_E_TXFAIL:
        case WLC_E_PMKID_CACHE:
        case WLC_E_RETROGRADE_TSF:
        case WLC_E_PRUNE:
        case WLC_E_AUTOAUTH:
        case WLC_E_EAPOL_MSG:
        case WLC_E_SCAN_COMPLETE:
        case WLC_E_ADDTS_IND:
        case WLC_E_DELTS_IND:
        case WLC_E_BCNSENT_IND:
        case WLC_E_BCNRX_MSG:
        case WLC_E_BCNLOST_MSG:
        case WLC_E_ROAM_PREP:
        case WLC_E_PFN_NET_FOUND:
        case WLC_E_PFN_NET_LOST:
        case WLC_E_RESET_COMPLETE:
        case WLC_E_JOIN_START:
        case WLC_E_ROAM_START:
        case WLC_E_ASSOC_START:
        case WLC_E_IBSS_ASSOC:
        case WLC_E_RADIO:
        case WLC_E_PSM_WATCHDOG:
        case WLC_E_CCX_ASSOC_START:
        case WLC_E_CCX_ASSOC_ABORT:
        case WLC_E_PROBREQ_MSG:
        case WLC_E_SCAN_CONFIRM_IND:
        case WLC_E_COUNTRY_CODE_CHANGED:
        case WLC_E_EXCEEDED_MEDIUM_TIME:
        case WLC_E_ICV_ERROR:
        case WLC_E_UNICAST_DECODE_ERROR:
        case WLC_E_MULTICAST_DECODE_ERROR:
        case WLC_E_TRACE:
        case WLC_E_BTA_HCI_EVENT:
        case WLC_E_IF:
        case WLC_E_P2P_DISC_LISTEN_COMPLETE:
        case WLC_E_RSSI:
        case WLC_E_PFN_SCAN_COMPLETE:
        case WLC_E_EXTLOG_MSG:
        case WLC_E_ACTION_FRAME:
        case WLC_E_ACTION_FRAME_COMPLETE:
        case WLC_E_PRE_ASSOC_IND:
        case WLC_E_PRE_REASSOC_IND:
        case WLC_E_CHANNEL_ADOPTED:
        case WLC_E_AP_STARTED:
        case WLC_E_DFS_AP_STOP:
        case WLC_E_DFS_AP_RESUME:
        case WLC_E_WAI_STA_EVENT:
        case WLC_E_WAI_MSG:
        case WLC_E_ESCAN_RESULT:
        case WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE:
        case WLC_E_PROBRESP_MSG:
        case WLC_E_P2P_PROBREQ_MSG:
        case WLC_E_DCS_REQUEST:
        case WLC_E_FIFO_CREDIT_MAP:
        case WLC_E_ACTION_FRAME_RX:
        case WLC_E_WAKE_EVENT:
        case WLC_E_RM_COMPLETE:
        case WLC_E_HTSFSYNC:
        case WLC_E_OVERLAY_REQ:
        case WLC_E_CSA_COMPLETE_IND:
        case WLC_E_EXCESS_PM_WAKE_EVENT:
        case WLC_E_PFN_SCAN_NONE:
        case WLC_E_PFN_SCAN_ALLGONE:
        case WLC_E_GTK_PLUMBED:
        case WLC_E_ASSOC_IND_NDIS:
        case WLC_E_REASSOC_IND_NDIS:
        case WLC_E_ASSOC_REQ_IE:
        case WLC_E_ASSOC_RESP_IE:
        case WLC_E_ASSOC_RECREATED:
        case WLC_E_ACTION_FRAME_RX_NDIS:
        case WLC_E_AUTH_REQ:
        case WLC_E_TDLS_PEER_EVENT:
        case WLC_E_SPEEDY_RECREATE_FAIL:
        case WLC_E_NATIVE:
        case WLC_E_PKTDELAY_IND:
        case WLC_E_AWDL_AW:
        case WLC_E_AWDL_ROLE:
        case WLC_E_AWDL_EVENT:
        case WLC_E_NIC_AF_TXS:
        case WLC_E_NIC_NIC_REPORT:
        case WLC_E_BEACON_FRAME_RX:
        case WLC_E_SERVICE_FOUND:
        case WLC_E_GAS_FRAGMENT_RX:
        case WLC_E_GAS_COMPLETE:
        case WLC_E_P2PO_ADD_DEVICE:
        case WLC_E_P2PO_DEL_DEVICE:
        case WLC_E_WNM_STA_SLEEP:
        case WLC_E_TXFAIL_THRESH:
        case WLC_E_PROXD:
        case WLC_E_IBSS_COALESCE:
        case WLC_E_AWDL_RX_PRB_RESP:
        case WLC_E_AWDL_RX_ACT_FRAME:
        case WLC_E_AWDL_WOWL_NULLPKT:
        case WLC_E_AWDL_PHYCAL_STATUS:
        case WLC_E_AWDL_OOB_AF_STATUS:
        case WLC_E_AWDL_SCAN_STATUS:
        case WLC_E_AWDL_AW_START:
        case WLC_E_AWDL_AW_END:
        case WLC_E_AWDL_AW_EXT:
        case WLC_E_AWDL_PEER_CACHE_CONTROL:
        case WLC_E_CSA_START_IND:
        case WLC_E_CSA_DONE_IND:
        case WLC_E_CSA_FAILURE_IND:
        case WLC_E_CCA_CHAN_QUAL:
        case WLC_E_BSSID:
        case WLC_E_TX_STAT_ERROR:
        case WLC_E_BCMC_CREDIT_SUPPORT:
        case WLC_E_PSTA_PRIMARY_INTF_IND:
        case WLC_E_LAST:
        case WLC_E_FORCE_32_BIT:
        default:
            wiced_assert( "Received event which was not registered\n", 0 != 0 );
            break;
    }

    if ( wwd_wifi_is_ready_to_transceive( event_header->interface ) == WWD_SUCCESS )
    {
        join_complete = WICED_TRUE;
    }

    if ( join_complete == WICED_TRUE )
    {
        if ( semaphore != NULL )
        {
            host_rtos_set_semaphore( semaphore, WICED_FALSE );
        }
        return NULL;
    }
    else
    {
        return handler_user_data;
    }
}


wwd_result_t wwd_wifi_get_mac_address( wiced_mac_t* mac, wwd_interface_t interface )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wiced_mac_t), IOVAR_STR_CUR_ETHERADDR ) );

    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, interface ) );

    memcpy( mac, host_buffer_get_current_piece_data_pointer( response ), sizeof(wiced_mac_t) );
    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_set_mac_address( wiced_mac_t mac )
{
    wiced_buffer_t buffer;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wiced_mac_t), IOVAR_STR_CUR_ETHERADDR );
    CHECK_IOCTL_BUFFER( data );
    memcpy( data, &mac, sizeof(wiced_mac_t) );
    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_is_ready_to_transceive( wwd_interface_t interface )
{
#if 0
    /* Handle Splint bug */ /*@-noeffect@*/ (void) wwd_wifi_ap_is_up; /*@+noeffect@*/
#endif

    switch ( interface )
    {
        case WWD_AP_INTERFACE:
            return ( wwd_wifi_ap_is_up == WICED_TRUE ) ? WWD_SUCCESS : WWD_INTERFACE_NOT_UP;

        case WWD_P2P_INTERFACE:
            if ( wwd_wifi_p2p_go_is_up == WICED_TRUE )
            {
                return WWD_SUCCESS;
            }
            // Otherwise fall through and check P2P client join status
            /* Disables Eclipse static analysis warning */
            /* no break */
        case WWD_STA_INTERFACE:
            return wwd_wifi_check_join_status( interface );
            /* Disables Eclipse static analysis warning */
            /* No break needed due to returns in all case paths */
            /* no break */
        case WWD_INTERFACE_MAX:
        case WWD_ETHERNET_INTERFACE:
        default:
            wiced_assert( "Bad interface", 0 != 0 );
            return WWD_UNKNOWN_INTERFACE;
    }
}


wwd_result_t wwd_wifi_enable_powersave_with_throughput( uint16_t return_to_sleep_delay_ms )
{
    wiced_buffer_t buffer;
    uint32_t* data;

    if ( return_to_sleep_delay_ms == 0 )
    {
        return_to_sleep_delay_ms = 200; // Use the default value
    }
    else if ( return_to_sleep_delay_ms < 10 )
    {
        return WWD_DELAY_TOO_SHORT;
    }
    else if ( return_to_sleep_delay_ms > 2000 )
    {
        return WWD_DELAY_TOO_LONG;
    }

    /* Set the maximum time to wait before going back to sleep */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_PM2_SLEEP_RET );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) ( return_to_sleep_delay_ms / 10 )* 10;
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );

#ifdef CHIP_FIRMWARE_SUPPORTS_PM_LIMIT_IOVAR
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_PM_LIMIT );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) NULL_FRAMES_WITH_PM_SET_LIMIT;
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
#endif /* ifdef CHIP_FIRMWARE_SUPPORTS_PM_LIMIT_IOVAR */

    /* set PM2 fast return to sleep powersave mode */
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) PM2_POWERSAVE_MODE;

    /* Record the power save mode and the return to sleep delay */
    wiced_wifi_powersave_mode        = PM2_POWERSAVE_MODE;
    wiced_wifi_return_to_sleep_delay = return_to_sleep_delay_ms;

    RETURN_WITH_ASSERT( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_PM, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_enable_powersave( void )
{
    wiced_buffer_t buffer;
    uint32_t* data;

    /* Set legacy powersave mode - PM1 */
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) PM1_POWERSAVE_MODE;

    /* Save the power save mode */
    wiced_wifi_powersave_mode = PM1_POWERSAVE_MODE;

    RETURN_WITH_ASSERT( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_PM, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_disable_powersave( void )
{
    wiced_buffer_t buffer;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( data );
    *data = NO_POWERSAVE_MODE;

    /* Save the power save mode */
    wiced_wifi_powersave_mode = NO_POWERSAVE_MODE;

    return wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_PM, buffer, NULL, WWD_STA_INTERFACE );
}


uint8_t wiced_wifi_get_powersave_mode( void )
{
    return wiced_wifi_powersave_mode;
}

uint16_t wiced_wifi_get_return_to_sleep_delay( void )
{
    return wiced_wifi_return_to_sleep_delay;
}

wwd_result_t wwd_wifi_get_tx_power( uint8_t* dbm )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_QTXPOWER );
    CHECK_IOCTL_BUFFER( data );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    *dbm = (uint8_t) ( *data / 4 );
    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_set_tx_power( uint8_t dbm )
{
    wiced_buffer_t buffer;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_QTXPOWER );
    CHECK_IOCTL_BUFFER( data );
    if ( dbm == (uint8_t) 0xFF )
    {
        *data = (uint32_t) 127;
    }
    else
    {
        *data = (uint32_t) ( 4 * dbm );
    }
    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_set_listen_interval( uint8_t listen_interval, wiced_listen_interval_time_unit_t time_unit )
{
    uint32_t* data;
    wiced_buffer_t buffer;
    uint8_t listen_interval_dtim;

    if (time_unit == WICED_LISTEN_INTERVAL_TIME_UNIT_DTIM)
    {
        listen_interval_dtim = listen_interval;
    }
    else
    {
        /* If the wake interval measured in DTIMs is set to 0, the wake interval is measured in beacon periods */
        listen_interval_dtim = 0;

        /* The wake period is measured in beacon periods, set the value as required */
        data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_LISTEN_INTERVAL_BEACON );
        CHECK_IOCTL_BUFFER( data );
        *data = (uint32_t) listen_interval;
        CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
    }

    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_LISTEN_INTERVAL_DTIM );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) listen_interval_dtim;
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );

    return wwd_wifi_set_listen_interval_assoc( (uint16_t)listen_interval );

}


wwd_result_t wwd_wifi_set_listen_interval_assoc( uint16_t listen_interval )
{
    wiced_buffer_t buffer;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_LISTEN_INTERVAL_ASSOC );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) listen_interval;
    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_get_listen_interval( wiced_listen_interval_t* li )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    int*           data;

    data = (int*)wwd_sdpcm_get_iovar_buffer( &buffer, 4, IOVAR_STR_LISTEN_INTERVAL_BEACON );
    CHECK_IOCTL_BUFFER( data );
    memset( data, 0, 1 );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    memcpy( (uint8_t *) &(li->beacon), (char *)host_buffer_get_current_piece_data_pointer( response ), 1 );
    host_buffer_release(response, WWD_NETWORK_RX);

    data = (int*)wwd_sdpcm_get_iovar_buffer( &buffer, 4, IOVAR_STR_LISTEN_INTERVAL_DTIM );
    CHECK_IOCTL_BUFFER( data );
    memset( data, 0, 1 );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    memcpy( (uint8_t *) &(li->dtim), (char *)host_buffer_get_current_piece_data_pointer( response ), 1 );
    host_buffer_release(response, WWD_NETWORK_RX);

    data = (int*)wwd_sdpcm_get_iovar_buffer( &buffer, 4, IOVAR_STR_LISTEN_INTERVAL_ASSOC );
    CHECK_IOCTL_BUFFER( data );
    memset( data, 0, 4 );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    memcpy( (uint16_t *) &(li->assoc), (char *)host_buffer_get_current_piece_data_pointer( response ), 2 );
    host_buffer_release(response, WWD_NETWORK_RX);

    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_set_ofdm_dutycycle( uint8_t duty_cycle_val )
{
    wiced_buffer_t buffer;
    uint32_t*      data;
    if( duty_cycle_val > 100 )
    {
        return WWD_INVALID_DUTY_CYCLE;
    }
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_DUTY_CYCLE_OFDM );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t)duty_cycle_val;

    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_set_cck_dutycycle( uint8_t duty_cycle_val )
{
    wiced_buffer_t buffer;
    uint32_t* data;
    if( duty_cycle_val > 100 )
    {
        return WWD_INVALID_DUTY_CYCLE;
    }
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_DUTY_CYCLE_CCK );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t)duty_cycle_val;

    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_get_ofdm_dutycycle( uint8_t* duty_cycle_value )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_DUTY_CYCLE_OFDM );
    CHECK_IOCTL_BUFFER( data );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    *duty_cycle_value = (uint8_t)*data;
    host_buffer_release( response, WWD_NETWORK_RX );

    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_get_cck_dutycycle( uint8_t* duty_cycle_value )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_DUTY_CYCLE_CCK );
    CHECK_IOCTL_BUFFER( data );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    *duty_cycle_value = (uint8_t)*data;
    host_buffer_release( response, WWD_NETWORK_RX );

    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_get_pmk( const char* psk, uint8_t psk_length, char* pmk )
{
    wsec_pmk_t*    psk_ioctl;
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wsec_pmk_t*    psk_info;

    psk_ioctl = ( wsec_pmk_t* )wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wsec_pmk_t) );
    CHECK_IOCTL_BUFFER( psk_ioctl );

    memcpy( psk_ioctl->key, psk, psk_length );
    psk_ioctl->key[psk_length] = 0;
    psk_ioctl->key_len      = psk_length;
    psk_ioctl->flags        = 0;

    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_WSEC_PMK, buffer, &response, WWD_STA_INTERFACE ) );

    psk_info = (wsec_pmk_t*) host_buffer_get_current_piece_data_pointer( response );
    if ( psk_info->key_len != WSEC_MAX_PSK_LEN )
    {
        host_buffer_release( response, WWD_NETWORK_RX );
        return WWD_PMK_WRONG_LENGTH;
    }
    memcpy( pmk, psk_info->key, psk_info->key_len );
    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_register_multicast_address( const wiced_mac_t* mac )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    uint16_t a;
    mcast_list_t* orig_mcast_list;
    mcast_list_t* new_mcast_list;

    /* Get the current multicast list */
    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(uint32_t) + MAX_SUPPORTED_MCAST_ENTRIES * sizeof(wiced_mac_t), IOVAR_STR_MCAST_LIST ) );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    /* Verify address is not currently registered */
    orig_mcast_list = (mcast_list_t*) host_buffer_get_current_piece_data_pointer( response );
    for ( a = 0; a < orig_mcast_list->entry_count; ++a )
    {
        /* Check if any address matches */
        if ( 0 == memcmp( mac, &orig_mcast_list->macs[a], sizeof(wiced_mac_t) ) )
        {
            /* A matching address has been found so we can stop now. */
            host_buffer_release( response, WWD_NETWORK_RX );
            return WWD_SUCCESS;
        }
    }

    /* Add the provided address to the list and write the new multicast list */
    new_mcast_list = (mcast_list_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) ( sizeof(uint32_t) + ( orig_mcast_list->entry_count + 1 ) * sizeof(wiced_mac_t) ), IOVAR_STR_MCAST_LIST );
    CHECK_IOCTL_BUFFER( new_mcast_list );
    new_mcast_list->entry_count = orig_mcast_list->entry_count;
    memcpy( new_mcast_list->macs, orig_mcast_list->macs, orig_mcast_list->entry_count * sizeof(wiced_mac_t) );
    host_buffer_release( response, WWD_NETWORK_RX );
    memcpy( &new_mcast_list->macs[new_mcast_list->entry_count], mac, sizeof(wiced_mac_t) );
    ++new_mcast_list->entry_count;
    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_unregister_multicast_address( const wiced_mac_t* mac )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    uint16_t a;
    mcast_list_t* orig_mcast_list;

    /* Get the current multicast list */
    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(uint32_t) + MAX_SUPPORTED_MCAST_ENTRIES * sizeof(wiced_mac_t), IOVAR_STR_MCAST_LIST ) );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    /* Find the address, assuming it is part of the list */
    orig_mcast_list = (mcast_list_t*) host_buffer_get_current_piece_data_pointer( response );
    if ( orig_mcast_list->entry_count != 0 )
    {
        mcast_list_t* new_mcast_list = (mcast_list_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) ( sizeof(uint32_t) + ( orig_mcast_list->entry_count - 1 ) * sizeof(wiced_mac_t) ), IOVAR_STR_MCAST_LIST );
        CHECK_IOCTL_BUFFER( new_mcast_list );
        for ( a = 0; a < orig_mcast_list->entry_count; ++a )
        {
            if ( 0 == memcmp( mac, &orig_mcast_list->macs[a], sizeof(wiced_mac_t) ) )
            {
                /* Copy the existing list up to the matching address */
                memcpy( new_mcast_list->macs, orig_mcast_list->macs, a * sizeof(wiced_mac_t) );

                /* Skip the current address and copy the remaining entries */
                memcpy( &new_mcast_list->macs[a], &orig_mcast_list->macs[a + 1], ( size_t )( orig_mcast_list->entry_count - a - 1 ) * sizeof(wiced_mac_t) );

                new_mcast_list->entry_count = orig_mcast_list->entry_count - 1;
                host_buffer_release( response, WWD_NETWORK_RX );
                RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
            }
        }
        /* There was something in the list, but the request MAC wasn't there */
        host_buffer_release( buffer, WWD_NETWORK_TX );
    }
    /* If we get here than the address wasn't in the list or the list was empty */
    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_get_rssi( int32_t* rssi )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 ) );
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_RSSI, buffer, &response, WWD_STA_INTERFACE ) );

    memcpy( rssi, host_buffer_get_current_piece_data_pointer( response ), sizeof(int32_t) );
    host_buffer_release( response, WWD_NETWORK_RX );

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_get_noise( int32_t* noise )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 ) );
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_PHY_NOISE, buffer, &response, WWD_STA_INTERFACE ) );

    memcpy( noise, host_buffer_get_current_piece_data_pointer( response ), sizeof(int32_t) );
    host_buffer_release( response, WWD_NETWORK_RX );

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_get_ap_client_rssi( int32_t* rssi, const wiced_mac_t* client_mac_addr  )
{
    client_rssi_t* client_rssi;
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    /* WLAN expects buffer size to be 4-byte aligned */
    client_rssi = (client_rssi_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, ROUND_UP( sizeof(client_rssi_t), sizeof(uint32_t)) );
    CHECK_IOCTL_BUFFER( client_rssi );

    memcpy(&client_rssi->mac_address, client_mac_addr, sizeof(wiced_mac_t));
    client_rssi->value = 0;

    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_RSSI, buffer, &response, WWD_AP_INTERFACE ) );

    memcpy( rssi, host_buffer_get_current_piece_data_pointer( response ), sizeof(int32_t) );
    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_select_antenna( wiced_antenna_t antenna )
{
    wiced_buffer_t buffer;
    uint32_t* data;

    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) antenna;
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_TXANT, buffer, 0, WWD_STA_INTERFACE ) );

    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) antenna;
    RETURN_WITH_ASSERT( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_ANTDIV, buffer, 0, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_set_roam_trigger( int32_t trigger_level )
{
    wiced_buffer_t buffer;
    uint32_t*      data;

    data = (uint32_t*)wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t)sizeof(uint32_t) );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t)trigger_level;

    return wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_ROAM_TRIGGER, buffer, 0, WWD_STA_INTERFACE );
}


wwd_result_t wwd_wifi_get_roam_trigger( int32_t* trigger_level )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    int32_t*       data;

    /* Validate input arguments */
    wiced_assert("Bad args", trigger_level != NULL);

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t)sizeof(uint32_t) ) );
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_ROAM_TRIGGER, buffer, &response, WWD_STA_INTERFACE ) );
    data = (int32_t*)host_buffer_get_current_piece_data_pointer( response );
    *trigger_level = *data;

    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_set_roam_delta( int32_t trigger_delta )
{
    wiced_buffer_t buffer;
    uint32_t*      data;

    data = (uint32_t*)wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t)sizeof(uint32_t) );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) trigger_delta;

    return wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_ROAM_DELTA, buffer, 0, WWD_STA_INTERFACE );
}


wwd_result_t wwd_wifi_get_roam_delta( int32_t* trigger_delta )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    int32_t*       data;

    wiced_assert("Bad args", trigger_delta != NULL);

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t)sizeof(uint32_t) ) );
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_ROAM_DELTA, buffer, &response, WWD_STA_INTERFACE ) );
    data = (int32_t*)host_buffer_get_current_piece_data_pointer( response );
    *trigger_delta = *data;

    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_set_roam_scan_period( uint32_t roam_scan_period )
{
    wiced_buffer_t  buffer;
    uint32_t*       data;

    data = (uint32_t*)wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t)sizeof(uint32_t) );
    CHECK_IOCTL_BUFFER( data );
    *data = roam_scan_period;

    return wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_ROAM_SCAN_PERIOD, buffer, 0, WWD_STA_INTERFACE );
}


wwd_result_t wwd_wifi_get_roam_scan_period( uint32_t* roam_scan_period )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    uint32_t*      data;

    /* Validate input arguments */
    wiced_assert("Bad args", roam_scan_period != NULL);

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t)sizeof(uint32_t) ) );
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_ROAM_SCAN_PERIOD, buffer, &response, WWD_STA_INTERFACE ) );
    data = (uint32_t*)host_buffer_get_current_piece_data_pointer( response );
    *roam_scan_period = *data;

    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_turn_off_roam( wiced_bool_t disable )
{
    wiced_buffer_t buffer;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_pkt_filter_enable_t), IOVAR_STR_ROAM_OFF );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t)disable;
    RETURN_WITH_ASSERT(  wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_send_action_frame( const wl_action_frame_t* action_frame )
{
    wiced_buffer_t buffer;
    wiced_action_frame_t* frame;
    uint32_t* a = (uint32_t*) wwd_sdpcm_get_iovar_buffer(&buffer, sizeof(wiced_action_frame_t)+4, IOVAR_STR_BSSCFG_ACTION_FRAME );
    CHECK_IOCTL_BUFFER( a );
    *a = 1;
    frame = (wiced_action_frame_t*)(a+1);
    memcpy(frame, action_frame, sizeof(wiced_action_frame_t));
    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE) );
}


wwd_result_t wwd_wifi_get_acparams_sta( edcf_acparam_t *acp )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    int* data = (int*) wwd_sdpcm_get_iovar_buffer( &buffer, 64, IOVAR_STR_AC_PARAMS_STA );
    CHECK_IOCTL_BUFFER( data );
    memset( data, 0, 64 );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    memcpy( (char *)acp, (char *)host_buffer_get_current_piece_data_pointer( response ), ( sizeof( edcf_acparam_t ) * 4 ) );
    host_buffer_release(response, WWD_NETWORK_RX);

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_enable_monitor_mode( void )
{
    wiced_buffer_t buffer;
    uint32_t*      data;

    /* Enable allmulti mode */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer(&buffer, 4, IOVAR_STR_ALLMULTI );
    CHECK_IOCTL_BUFFER( data );
    *data = 1;

    CHECK_RETURN( wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE) );

    /* Enable monitor mode */
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer(&buffer, 4);
    CHECK_IOCTL_BUFFER( data );
    *data = 1;

    CHECK_RETURN( wwd_sdpcm_send_ioctl(SDPCM_SET, WLC_SET_MONITOR, buffer, NULL, WWD_STA_INTERFACE) );

    wwd_sdpcm_monitor_mode_enabled = WICED_TRUE;
    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_disable_monitor_mode( void )
{
    wiced_buffer_t buffer;
    uint32_t*      data;

    /* Disable allmulti mode */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer(&buffer, 4, IOVAR_STR_ALLMULTI );
    CHECK_IOCTL_BUFFER( data );
    *data = 0;

    CHECK_RETURN( wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE) );

    /* Disable monitor mode */
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer(&buffer, 4);
    CHECK_IOCTL_BUFFER( data );
    *data = 0;

    CHECK_RETURN( wwd_sdpcm_send_ioctl(SDPCM_SET, WLC_SET_MONITOR, buffer, NULL, WWD_STA_INTERFACE) );

    wwd_sdpcm_monitor_mode_enabled = WICED_FALSE;
    return WWD_SUCCESS;
}

wiced_bool_t wwd_wifi_monitor_mode_is_enabled( void )
{
    return wwd_sdpcm_monitor_mode_enabled;
}

wwd_result_t wwd_wifi_get_bssid( wiced_mac_t* bssid )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    memset( bssid, 0, sizeof( wiced_mac_t ) );

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof( wiced_mac_t ) ) );
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_BSSID, buffer, &response, WWD_STA_INTERFACE ) );

    memcpy( bssid->octet, host_buffer_get_current_piece_data_pointer( response ), sizeof( wiced_mac_t ) );
    host_buffer_release( response, WWD_NETWORK_RX );

    return WWD_SUCCESS;
}


/******************************************************
 *             Wiced-internal functions
 ******************************************************/

/** Set the Wi-Fi device down. Internal use only
 *
 * @param interface
 * @return
 */
wwd_result_t wwd_wifi_set_down( wwd_interface_t interface )
{
    wiced_buffer_t buffer;

    UNUSED_PARAMETER( interface );

    /* Send DOWN command */
    CHECK_IOCTL_BUFFER(  wwd_sdpcm_get_ioctl_buffer( &buffer, 0 ) )
    RETURN_WITH_ASSERT( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_DOWN, buffer, 0, WWD_STA_INTERFACE ) );
}

wwd_result_t wwd_wifi_manage_custom_ie( wwd_interface_t interface, wiced_custom_ie_action_t action, /*@unique@*/ const uint8_t* oui, uint8_t subtype, const void* data, uint16_t length, uint16_t which_packets)
{
    wiced_buffer_t    buffer;
    vndr_ie_setbuf_t* ie_setbuf;
    uint32_t*         iovar_data;

    wiced_assert("Bad Args", oui != NULL);

    iovar_data = (uint32_t*)wwd_sdpcm_get_iovar_buffer(&buffer, (uint16_t)(sizeof(vndr_ie_setbuf_t) + length + 4), IOVAR_STR_BSSCFG_VENDOR_IE );
    CHECK_IOCTL_BUFFER( iovar_data );
    *iovar_data = interface;
    ie_setbuf = (vndr_ie_setbuf_t*)(iovar_data+1);

    /* Copy the vndr_ie SET command ("add"/"del") to the buffer */
    if (action == WICED_ADD_CUSTOM_IE)
    {
        memcpy( (char*)ie_setbuf->cmd, "add", 3 );
    }
    else
    {
        memcpy( (char*)ie_setbuf->cmd, "del", 3 );
    }
    ie_setbuf->cmd[3] = 0;

    /* Set the values */
    ie_setbuf->vndr_ie_buffer.iecount = (int32_t) 1;

    ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].pktflag = which_packets;
    ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.id  = 0xdd;
    ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.len = (uint8_t)(length + sizeof(ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.oui) + 1); /* +1: one byte for sub type */

    /*@-usedef@*/ /* Stop lint warning about vndr_ie_list array element not yet being defined */
    memcpy( ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.oui, oui, (size_t)WIFI_IE_OUI_LENGTH );
    /*@+usedef@*/

    ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.data[0] = subtype;

    memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.data[1], data, length);

    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}


void wwd_wifi_prioritize_acparams( const edcf_acparam_t *acp, int *priority )
{
    int aci;
    int aifsn;
    int ecwmin;
    int ecwmax;
    int ranking_basis[AC_COUNT];
    int *p;
    int i;

    p = priority;

    for (aci = 0; aci < AC_COUNT; aci++, acp++, p++)
    {
        aifsn  = acp->ACI & EDCF_AIFSN_MASK;
        ecwmin = acp->ECW & EDCF_ECWMIN_MASK;
        ecwmax = (acp->ECW & EDCF_ECWMAX_MASK) >> EDCF_ECWMAX_SHIFT;
        ranking_basis[aci] = aifsn + ecwmin + ecwmax; /* Default AC_VO will be the littlest ranking value */
        *p = 1; /* Initialise priority starting at 1 */
    }

    p = priority;

    /* Primitive ranking method which works for AC priority swapping when values for cwmin, cwmax and aifsn are varied */
    for (aci = 0; aci < AC_COUNT; aci++, p++) /* Compare each ACI against each other ACI */
    {
        i = 0;
        for (i = 0; i < AC_COUNT; i++)
        {
            if ( i != aci )
            {
                /* Smaller ranking value has higher priority, so increment priority for each ACI which has a higher ranking value */
                if (ranking_basis[aci] < ranking_basis[i])
                {
                    *p = *p + 1;
                }
            }
        }
    }

    // Now invert so that highest priority is the lowest number, e.g. for default settings voice will be priority 1 and background will be priority 4
    p = priority;
    for (i = 0; i < AC_COUNT; i++, p++)
    {
        *p = AC_COUNT + 1 - *p;
    }
}

wwd_result_t wwd_wifi_update_tos_map( void )
{
    edcf_acparam_t ac_params[AC_COUNT];
    const wiced_qos_access_category_t tos_to_ac_map[8] = { WMM_AC_BE, WMM_AC_BK, WMM_AC_BK, WMM_AC_BE, WMM_AC_VI, WMM_AC_VI, WMM_AC_VO, WMM_AC_VO };
    wiced_qos_access_category_t ac;
    wiced_bool_t admission_control_mandatory[AC_COUNT];
    int i, j;

    if ( wwd_wifi_get_acparams_sta( ac_params ) != WWD_SUCCESS )
    {
        return WWD_WLAN_ERROR;
    }

    /* For each access category, record whether admission control is necessary or not */
    for (i = 0; i < AC_COUNT; i++)
    {
        j = (ac_params[i].ACI & EDCF_ACI_MASK) >> EDCF_ACI_SHIFT; // Determine which access category (in case they're out of order)
        if ( ( ac_params[i].ACI & EDCF_ACM_MASK ) == 0 )
        {
            admission_control_mandatory[j] = WICED_FALSE;
        }
        else
        {
            admission_control_mandatory[j] = WICED_TRUE;
        }
    }

    /* For each type of service value look up the Access Category that is mapped to this type of service and update the TOS map
     * with what the AP actually allows, if necessary */
    for (i = 0; i < 8; i++ )
    {
        ac = tos_to_ac_map[i];
        if ( admission_control_mandatory[ac] == WICED_FALSE ) /* No need to re-map to lower priority */
        {
            wwd_tos_map[i] = (uint8_t)i;
        }
        else
        {
            if ( ac == WMM_AC_VO )
            {
                if ( admission_control_mandatory[WMM_AC_VI] == WICED_FALSE )
                {
                    wwd_tos_map[i] = WMM_AC_VI;
                }
                else if ( admission_control_mandatory[WMM_AC_BE] == WICED_FALSE )
                {
                    wwd_tos_map[i] = WMM_AC_BE;
                }
                else
                {
                    wwd_tos_map[i] = WMM_AC_BK;
                }
            }
            else if ( ac == WMM_AC_VI )
            {
                if ( admission_control_mandatory[WMM_AC_BE] == WICED_FALSE )
                {
                    wwd_tos_map[i] = WMM_AC_BE;
                }
                else
                {
                    wwd_tos_map[i] = WMM_AC_BK;
                }
            }
            else /* Case for best effort and background is the same since we are going to send at lowest priority anyway */
            {
                wwd_tos_map[i] = WMM_AC_BK;
            }
        }
    }

    return WWD_SUCCESS;
}

void wwd_wifi_edcf_ac_params_print( const wiced_edcf_ac_param_t *acp, const int *priority )
{
#ifdef WPRINT_ENABLE_WWD_DEBUG
    int aci;
    int acm, aifsn, ecwmin, ecwmax, txop;
    static const char ac_names[AC_COUNT][6] = {"AC_BE", "AC_BK", "AC_VI", "AC_VO"};

    if ( acp != NULL )
    {
        for (aci = 0; aci < AC_COUNT; aci++, acp++)
        {
            if (((acp->ACI & EDCF_ACI_MASK) >> EDCF_ACI_SHIFT) != aci)
            {
                WPRINT_WWD_ERROR(("Warning: AC params out of order\n"));
            }
            acm = (acp->ACI & EDCF_ACM_MASK) ? 1 : 0;
            aifsn = acp->ACI & EDCF_AIFSN_MASK;
            ecwmin = acp->ECW & EDCF_ECWMIN_MASK;
            ecwmax = (acp->ECW & EDCF_ECWMAX_MASK) >> EDCF_ECWMAX_SHIFT;
            txop = (uint16_t)acp->TXOP;
            WPRINT_WWD_DEBUG(("%s: raw: ACI 0x%x ECW 0x%x TXOP 0x%x\n", ac_names[aci], acp->ACI, acp->ECW, txop));
            WPRINT_WWD_DEBUG(("       dec: aci %d acm %d aifsn %d " "ecwmin %d ecwmax %d txop 0x%x\n", aci, acm, aifsn, ecwmin, ecwmax, txop) );
                /* CWmin = 2^(ECWmin) - 1 */
                /* CWmax = 2^(ECWmax) - 1 */
                /* TXOP = number of 32 us units */
            WPRINT_WWD_DEBUG(("       eff: CWmin %d CWmax %d TXop %dusec\n", EDCF_ECW2CW(ecwmin), EDCF_ECW2CW(ecwmax), EDCF_TXOP2USEC(txop)));
        }
    }

    if ( priority != NULL )
    {
        for (aci = 0; aci < AC_COUNT; aci++, priority++)
        {
            WPRINT_WWD_DEBUG(("%s: ACI %d Priority %d\n", ac_names[aci], aci, *priority));
        }
    }
#else /* ifdef WPRINT_ENABLE_WWD_DEBUG */
    UNUSED_PARAMETER( acp );
    UNUSED_PARAMETER( priority );
#endif /* ifdef WPRINT_ENABLE_WWD_DEBUG */
}


wwd_result_t wwd_wifi_get_channel( wwd_interface_t interface, uint32_t* channel )
{
    wiced_buffer_t  buffer;
    wiced_buffer_t  response;
    channel_info_t* channel_info;

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(channel_info_t) ) );

    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_CHANNEL, buffer, &response, interface ) );

    channel_info = (channel_info_t*) host_buffer_get_current_piece_data_pointer( response );
    *channel = (uint32_t)channel_info->hw_channel;
    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_set_channel( wwd_interface_t interface, uint32_t channel )
{
    wiced_buffer_t    buffer;
    uint32_t*         data;
    wl_chan_switch_t* chan_switch;
    wwd_interface_t   temp_interface = interface;

    /* Map P2P interface to either STA or AP interface depending if it's running as group owner or client */
    if ( interface == WWD_P2P_INTERFACE )
    {
        if ( wwd_wifi_p2p_go_is_up == WICED_TRUE )
        {
            temp_interface = WWD_AP_INTERFACE;
        }
        else
        {
            temp_interface = WWD_STA_INTERFACE;
        }
    }

    switch ( temp_interface )
    {
        case WWD_STA_INTERFACE:
            data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(uint32_t) );
            CHECK_IOCTL_BUFFER( data );
            *data = channel;
            CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_SET_CHANNEL, buffer, NULL, interface ) );
            break;

        case WWD_AP_INTERFACE:
            chan_switch = (wl_chan_switch_t*)wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_chan_switch_t), IOVAR_STR_CSA );
            CHECK_IOCTL_BUFFER( chan_switch );
            chan_switch->chspec = (wl_chanspec_t)(WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE | channel);
            chan_switch->count  = 1;
            chan_switch->mode   = 1;
            chan_switch->reg    = 0;
            CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, interface ) );
            break;

        case WWD_P2P_INTERFACE:
        case WWD_INTERFACE_MAX:
        case WWD_ETHERNET_INTERFACE:
        default:
            wiced_assert( "Bad interface", 0 != 0 );
            return WWD_UNKNOWN_INTERFACE;
    }

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_get_counters( wwd_interface_t interface, wiced_counters_t* counters )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wiced_counters_t* received_counters;

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wiced_counters_t), IOVAR_STR_COUNTERS ) );

    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, interface ) );

    received_counters = (wiced_counters_t*) host_buffer_get_current_piece_data_pointer( response );
    memcpy(counters, received_counters, sizeof(wiced_counters_t));
    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_get_rate( wwd_interface_t interface, uint32_t* rate )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    int32_t* wl_rate;

    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(int32_t) ) );
    CHECK_RETURN (wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_RATE, buffer, &response, interface ));
    wl_rate = (int32_t*) host_buffer_get_current_piece_data_pointer( response );
    memcpy(rate, wl_rate, sizeof(uint32_t));
    if (*wl_rate == -1)
        *rate = 0;
    host_buffer_release(response, WWD_NETWORK_RX);

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_set_band_specific_rate( wwd_interface_t interface, uint32_t rate )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    uint32_t band;
    uint32_t bandlist[3];
    char aname[] = IOVAR_STR_ARATE;
    char bgname[] = IOVAR_STR_BGRATE;
    char *name;
    int32_t *data;

    /* Get band */
    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(uint32_t) ) );
    CHECK_RETURN (wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_BAND, buffer, &response, interface ));
    band = *(uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    host_buffer_release(response, WWD_NETWORK_RX);

    /* Get supported Band List */
    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(bandlist) ) );
    CHECK_RETURN (wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_BANDLIST, buffer, &response, interface ));
    memcpy(bandlist, (uint32_t*) host_buffer_get_current_piece_data_pointer( response ), sizeof(bandlist));
    host_buffer_release(response, WWD_NETWORK_RX);

    /* only support a maximum of 2 bands */
    if (!bandlist[0])
        return WWD_WLAN_ERROR;
    else if (bandlist[0] > 2)
        bandlist[0] = 2;

    switch (band) {
    /* In case of auto band selection the current associated band will be in the entry 1 */
    case WLC_BAND_AUTO :
        if (bandlist[1] == WLC_BAND_5G)
            name = (char *)aname;
        else if (bandlist[1] == WLC_BAND_2G)
            name = (char *)bgname;
        else
            return WWD_WLAN_ERROR;

        break;

    case WLC_BAND_5G :
        name = (char *)aname;
        break;

    case WLC_BAND_2G :
        name = (char *)bgname;
        break;

    default :
        return WWD_WLAN_ERROR;
        break;
    }

    data = (int32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(int32_t), name );
    CHECK_IOCTL_BUFFER( data );
    if (rate == 0)
        *data = -1; // Auto rate
    else
        *data = (int32_t)rate;

    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}

wwd_result_t wwd_wifi_get_supported_band_list( wiced_band_list_t* band_list )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    /* Get supported Band List */
    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wiced_band_list_t) ) );
    CHECK_RETURN (wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_BANDLIST, buffer, &response, WWD_STA_INTERFACE ));
    memcpy(band_list, (uint32_t*) host_buffer_get_current_piece_data_pointer( response ), sizeof(wiced_band_list_t));
    host_buffer_release(response, WWD_NETWORK_RX);

    if ( band_list->number_of_bands == 0 )
    {
        return WWD_WLAN_ERROR;
    }

    return WWD_SUCCESS;
}


/* STF parameters are not yet supported - FIXME*/
wwd_result_t wwd_wifi_set_mcs_rate( wwd_interface_t interface, int32_t mcs, wiced_bool_t mcsonly )
{
    wiced_buffer_t buffer;
    int32_t *data;
    uint32_t nrate = 0;

    if (mcs != -1)
    {
        nrate |= mcs & NRATE_RATE_MASK;
        nrate |= NRATE_MCS_INUSE;

        if (mcsonly)
        {
            nrate |= NRATE_OVERRIDE_MCS_ONLY;
        }
        data = (int32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(int32_t), IOVAR_STR_NRATE );
        CHECK_IOCTL_BUFFER( data );
        *data = (int32_t)nrate;

        RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, interface ) );
    }
    else
    {
        /* reset rates to auto */
        RETURN_WITH_ASSERT( wwd_wifi_set_band_specific_rate( interface, 0 ));
    }
}

wwd_result_t wwd_wifi_set_legacy_rate( wwd_interface_t interface, int32_t rate )
{
    wiced_buffer_t buffer;
    int32_t *data;
    uint32_t nrate = 0;

    if (rate != 0)
    {
        nrate |= rate & NRATE_RATE_MASK;

        data = (int32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(int32_t), IOVAR_STR_NRATE );
        CHECK_IOCTL_BUFFER( data );
        *data = (int32_t)nrate;

        RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, interface ) );
    }
    else
    {
        /* reset rates to auto */
        data = (int32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(int32_t), IOVAR_STR_BGRATE );
        CHECK_IOCTL_BUFFER( data );
        *data = -1;

        RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, interface ) );
    }
}

wwd_result_t wwd_wifi_disable_11n_support( wwd_interface_t interface, wiced_bool_t disable )
{
    wiced_buffer_t buffer;

    uint32_t* data = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(uint32_t), IOVAR_STR_NMODE );
    CHECK_IOCTL_BUFFER( data );
    *data = (disable) ? 0 : 1;
    RETURN_WITH_ASSERT(  wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, interface ) );
}

wwd_result_t wwd_wifi_set_packet_filter_mode( wiced_packet_filter_mode_t mode )
{
    wiced_buffer_t buffer;

    uint32_t* data = (uint32_t*)wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(uint32_t), IOVAR_STR_PKT_FILTER_MODE );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t)mode;
    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}

wwd_result_t wwd_wifi_add_packet_filter( const wiced_packet_filter_t* settings )
{
    wl_pkt_filter_t* packet_filter;
    wiced_buffer_t   buffer;
    uint32_t         buffer_length = ( 2 * (uint32_t)settings->mask_size ) + WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN;

    packet_filter = (wl_pkt_filter_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t)buffer_length , IOVAR_STR_PKT_FILTER_ADD );
    CHECK_IOCTL_BUFFER( packet_filter );

    /* Copy filter entries */
    packet_filter->id                   = settings->id;
    packet_filter->type                 = 0;
    packet_filter->negate_match         = settings->rule;
    packet_filter->u.pattern.offset     = (uint32_t)settings->offset;
    packet_filter->u.pattern.size_bytes = settings->mask_size;

    /* Copy mask */
    memcpy( packet_filter->u.pattern.mask_and_pattern, settings->mask, settings->mask_size );

    /* Copy filter pattern */
    memcpy( packet_filter->u.pattern.mask_and_pattern + settings->mask_size, settings->pattern, settings->mask_size );

    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}

wwd_result_t wwd_wifi_remove_packet_filter( uint8_t filter_id )
{
    wiced_buffer_t buffer;

    uint32_t* data = (uint32_t*)wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(uint32_t), IOVAR_STR_PKT_FILTER_DELETE );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t)filter_id;
    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}

wwd_result_t wwd_wifi_enable_packet_filter( uint8_t filter_id )
{
    return wwd_wifi_toggle_packet_filter( filter_id, WICED_TRUE );
}

wwd_result_t wwd_wifi_disable_packet_filter( uint8_t filter_id )
{
    return wwd_wifi_toggle_packet_filter( filter_id, WICED_FALSE );
}

wwd_result_t wwd_wifi_toggle_packet_filter( uint8_t filter_id, wiced_bool_t enable )
{
    wiced_buffer_t buffer;

    wl_pkt_filter_enable_t* data = (wl_pkt_filter_enable_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_pkt_filter_enable_t), IOVAR_STR_PKT_FILTER_ENABLE );
    CHECK_IOCTL_BUFFER( data );
    data->id     = (uint32_t)filter_id;
    data->enable = (uint32_t)enable;
    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}

wwd_result_t wwd_wifi_get_packet_filter_stats( uint8_t filter_id, wiced_packet_filter_stats_t* stats )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    uint32_t* data = (uint32_t*)wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(uint32_t) + sizeof(wiced_packet_filter_stats_t), IOVAR_STR_PKT_FILTER_STATS );
    CHECK_IOCTL_BUFFER( data );

    memset( data, 0, sizeof(uint32_t) + sizeof(wiced_packet_filter_stats_t) );
    *data = (uint32_t)filter_id;

    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    memcpy( (char *)stats, (char *)host_buffer_get_current_piece_data_pointer( response ), ( sizeof(wiced_packet_filter_stats_t) ) );
    host_buffer_release( response, WWD_NETWORK_RX );

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_clear_packet_filter_stats( uint32_t filter_id )
{
    wiced_buffer_t buffer;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(uint32_t), IOVAR_STR_PKT_FILTER_CLEAR_STATS );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t)filter_id;
    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}


wwd_result_t wwd_wifi_get_packet_filters( uint32_t max_count, uint32_t offset, wiced_packet_filter_t* list,  uint32_t* count_out )
{
    wiced_bool_t          enabled_list;

    *count_out = 0;

    for ( enabled_list = WICED_FALSE; enabled_list <= WICED_TRUE; enabled_list++ )
    {
        uint32_t num_returned = 0;
        wwd_result_t retval;
        retval = wwd_wifi_get_packet_filters_inner(  max_count, offset, list, enabled_list, &num_returned );
        list += num_returned;
        offset = MAX( offset - num_returned, 0 );
        max_count = MAX( max_count - num_returned, 0 );
        *count_out += num_returned;
        CHECK_RETURN( retval );
    }

    return WWD_SUCCESS;
}


static wwd_result_t wwd_wifi_get_packet_filters_inner( uint32_t max_count, uint32_t offset, wiced_packet_filter_t* list, wiced_bool_t enabled_list, uint32_t* count_out )
{
    wiced_buffer_t        buffer;
    wiced_buffer_t        response;
    uint32_t*             data;
    wl_pkt_filter_list_t* filter_list;
    uint32_t              filter_ptr;
    uint32_t              i;

    wwd_result_t retval = WWD_SUCCESS;
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, PACKET_FILTER_LIST_BUFFER_MAX_LEN, IOVAR_STR_PKT_FILTER_LIST );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t)enabled_list;

    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    filter_list  = (wl_pkt_filter_list_t*)host_buffer_get_current_piece_data_pointer( response );
    filter_ptr   = (uint32_t)filter_list->filter;

    *count_out = 0;

    for ( i = offset; i < filter_list->num; i++ )
    {
        wl_pkt_filter_t*       in_filter  = (wl_pkt_filter_t*)filter_ptr;
        wiced_packet_filter_t* out_filter = &list[i-offset];

        if ( i >= offset + max_count )
        {
            retval = WWD_PARTIAL_RESULTS;
            break;
        }

        out_filter->id        = in_filter->id;
        out_filter->rule      = ( in_filter->negate_match == 0 ) ? WICED_PACKET_FILTER_RULE_POSITIVE_MATCHING : WICED_PACKET_FILTER_RULE_NEGATIVE_MATCHING;
        out_filter->offset    = (uint16_t)in_filter->u.pattern.offset;
        out_filter->mask_size = (uint16_t)in_filter->u.pattern.size_bytes;
        out_filter->enabled_status = enabled_list;

        out_filter->mask      = NULL;
        out_filter->pattern   = NULL;

        /* Update WL filter pointer */
        filter_ptr += (WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * in_filter->u.pattern.size_bytes);

        /* WLAN returns word-aligned filter list */
        filter_ptr = ROUND_UP( filter_ptr, 4 );

        (*count_out)++;
    }

    host_buffer_release( response, WWD_NETWORK_RX );
    return retval;
}


wwd_result_t wwd_wifi_get_packet_filter_mask_and_pattern( uint32_t filter_id, uint32_t max_size, uint8_t* mask, uint8_t* pattern, uint32_t* size_out )
{
    wiced_bool_t          enabled_list;

    for ( enabled_list = WICED_FALSE; enabled_list <= WICED_TRUE; enabled_list++ )
    {

        wiced_buffer_t        buffer;
        wiced_buffer_t        response;
        uint32_t*             data;
        wl_pkt_filter_list_t* filter_list;
        uint32_t              filter_ptr;
        uint32_t              i;
        wl_pkt_filter_t*      in_filter;

        data = wwd_sdpcm_get_iovar_buffer( &buffer, PACKET_FILTER_LIST_BUFFER_MAX_LEN, IOVAR_STR_PKT_FILTER_LIST );
        CHECK_IOCTL_BUFFER( data );
        *data = (uint32_t)enabled_list;

        CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

        filter_list  = (wl_pkt_filter_list_t*)host_buffer_get_current_piece_data_pointer( response );
        filter_ptr   = (uint32_t)filter_list->filter;

        for ( i = 0; i < filter_list->num; i++ )
        {
            in_filter  = (wl_pkt_filter_t*)filter_ptr;

            if ( in_filter->id == filter_id )
            {
                *size_out = MIN( in_filter->u.pattern.size_bytes, max_size );
                memcpy ( mask,    in_filter->u.pattern.mask_and_pattern, *size_out );
                memcpy ( pattern, in_filter->u.pattern.mask_and_pattern + in_filter->u.pattern.size_bytes, *size_out );
                host_buffer_release( response, WWD_NETWORK_RX );
                if ( *size_out < in_filter->u.pattern.size_bytes )
                {
                    return WWD_PARTIAL_RESULTS;
                }
                return WWD_SUCCESS;
            }

            /* Update WL filter pointer */
            filter_ptr += (WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * in_filter->u.pattern.size_bytes);

            /* WLAN returns word-aligned filter list */
            filter_ptr = ROUND_UP( filter_ptr, 4 );
        }
    }
    return WWD_FILTER_NOT_FOUND;
}



wwd_result_t wwd_wifi_add_keep_alive( const wiced_keep_alive_packet_t* keep_alive_packet_info )
{
    wiced_buffer_t buffer;

    uint16_t length = (uint16_t)(keep_alive_packet_info->packet_length + WL_MKEEP_ALIVE_FIXED_LEN);
    wl_mkeep_alive_pkt_t* packet_info = (wl_mkeep_alive_pkt_t*) wwd_sdpcm_get_iovar_buffer( &buffer, length, IOVAR_STR_MKEEP_ALIVE );
    CHECK_IOCTL_BUFFER( packet_info );

    packet_info->version       = htod16(WL_MKEEP_ALIVE_VERSION);
    packet_info->length        = htod16(WL_MKEEP_ALIVE_FIXED_LEN);
    packet_info->keep_alive_id = keep_alive_packet_info->keep_alive_id;
    packet_info->period_msec   = htod32(keep_alive_packet_info->period_msec);
    packet_info->len_bytes     = htod16(keep_alive_packet_info->packet_length);
    memcpy(packet_info->data, keep_alive_packet_info->packet, keep_alive_packet_info->packet_length);

    RETURN_WITH_ASSERT( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) );
}

wwd_result_t wwd_wifi_get_keep_alive( wiced_keep_alive_packet_t* keep_alive_packet_info )
{
    wl_mkeep_alive_pkt_t* packet_info;
    wiced_buffer_t        buffer;
    wiced_buffer_t        response;
    uint32_t*             data;
    uint16_t              max_info_length = (uint16_t)(WL_MKEEP_ALIVE_FIXED_LEN + keep_alive_packet_info->packet_length);

    wiced_assert("Bad args", (keep_alive_packet_info != NULL) && (keep_alive_packet_info->packet_length > 4) && (keep_alive_packet_info->keep_alive_id <= 3));

    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, max_info_length, IOVAR_STR_MKEEP_ALIVE );  /* get a buffer to store the keep_alive info into */
    CHECK_IOCTL_BUFFER( data );
    memset( data, 0, max_info_length );
    data[0] = keep_alive_packet_info->keep_alive_id;

    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) );

    packet_info = (wl_mkeep_alive_pkt_t*)host_buffer_get_current_piece_data_pointer( response );
    keep_alive_packet_info->packet_length = packet_info->len_bytes;
    keep_alive_packet_info->period_msec = packet_info->period_msec;
    memcpy( keep_alive_packet_info->packet, packet_info->data, (size_t)MIN(keep_alive_packet_info->packet_length, (host_buffer_get_current_piece_size(response)-WL_MKEEP_ALIVE_FIXED_LEN)) );
    host_buffer_release( response, WWD_NETWORK_RX );

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_disable_keep_alive( uint8_t id )
{
    wiced_keep_alive_packet_t packet_info;
    packet_info.keep_alive_id = id;
    packet_info.period_msec   = 0;
    packet_info.packet_length = 0;
    packet_info.packet        = NULL;
    return wwd_wifi_add_keep_alive( &packet_info );
}

wwd_result_t wwd_wifi_get_associated_client_list( void* client_list_buffer, uint16_t buffer_length )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    wiced_maclist_t* data = (wiced_maclist_t*)wwd_sdpcm_get_ioctl_buffer( &buffer, buffer_length );
    CHECK_IOCTL_BUFFER( data );
    memset(data, 0, buffer_length);
    data->count = ((wiced_maclist_t*)client_list_buffer)->count;

    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_ASSOCLIST, buffer, &response, WWD_AP_INTERFACE ) );

    memcpy( client_list_buffer, (void*) host_buffer_get_current_piece_data_pointer( response ), (size_t)MIN(host_buffer_get_current_piece_size(response), buffer_length) );
    host_buffer_release( response, WWD_NETWORK_RX );
    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_get_max_associations( uint32_t* max_assoc )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wwd_result_t   result;

    uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_MAX_ASSOC );
    CHECK_IOCTL_BUFFER( data );
    result = wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE );

    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    *max_assoc = (uint8_t)*data;
    host_buffer_release( response, WWD_NETWORK_RX );

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_test_credentials( wiced_scan_result_t* ap, const uint8_t* security_key, uint8_t key_length )
{
    host_semaphore_type_t semaphore;
    wwd_result_t          result;
    uint32_t              previous_softap_channel = 0;

    host_rtos_init_semaphore( &semaphore );

    /* Check if soft AP interface is up, if so, record its current channel */
    if ( wwd_wifi_is_ready_to_transceive( WWD_AP_INTERFACE ) == WWD_SUCCESS )
    {
        wwd_wifi_get_channel( WWD_AP_INTERFACE, &previous_softap_channel );
    }

    /* Try and join the AP with the credentials provided, but only wait for a short time */
    result = wwd_wifi_join_specific( ap, security_key, key_length, &semaphore, WWD_STA_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        goto test_credentials_deinit;
    }

    result = host_rtos_get_semaphore( &semaphore, WICED_CREDENTIAL_TEST_TIMEOUT, WICED_FALSE );

    /* Immediately leave so we can go back to the original soft AP channel */
    wwd_wifi_leave( WWD_STA_INTERFACE );

    /* If applicable, move the soft AP back to its original channel */
    if ( ( previous_softap_channel != 0 ) && ( previous_softap_channel != ap->channel ) )
    {
        wwd_wifi_set_channel( WWD_AP_INTERFACE, previous_softap_channel );
    }
test_credentials_deinit:
    host_rtos_deinit_semaphore( &semaphore );

    return result;
}

wwd_result_t wwd_wifi_get_ap_info( wl_bss_info_t* ap_info, wiced_security_t* security )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    uint32_t*      data;
    wwd_result_t   result = WWD_SUCCESS;

    /* Read the BSS info */
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wl_bss_info_t) + 4 );
    CHECK_IOCTL_BUFFER( data );
    *data = sizeof(wl_bss_info_t) + 4;
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_BSS_INFO, buffer, &response, WWD_STA_INTERFACE ) );

    memcpy( ap_info, (void*) (host_buffer_get_current_piece_data_pointer( response ) + 4), sizeof(wl_bss_info_t) );
    host_buffer_release( response, WWD_NETWORK_RX );

    /* Read the WSEC setting */
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(uint32_t) );
    CHECK_IOCTL_BUFFER( data );
    CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_WSEC, buffer, &response, WWD_STA_INTERFACE ) );

    data     = (uint32_t*)host_buffer_get_current_piece_data_pointer( response );
    *security = (wiced_security_t)(*data);
    host_buffer_release( response, WWD_NETWORK_RX );

    if ( *security == WICED_SECURITY_WEP_PSK )
    {
        /* Read the WEP auth setting */
        data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(uint32_t) );
        CHECK_IOCTL_BUFFER( data );
        CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_AUTH, buffer, &response, WWD_STA_INTERFACE ) );

        data = (uint32_t*)host_buffer_get_current_piece_data_pointer( response );

        if ( *data == SHARED_AUTH )
        {
            *security |= SHARED_ENABLED;
        }
        host_buffer_release( response, WWD_NETWORK_RX );
    }
    else if ( (*security & (TKIP_ENABLED | AES_ENABLED) ) != 0 )
    {
        /* Read the WPA auth setting */
        data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(uint32_t) );
        CHECK_IOCTL_BUFFER( data );
        CHECK_RETURN( wwd_sdpcm_send_ioctl( SDPCM_GET, WLC_GET_WPA_AUTH, buffer, &response, WWD_STA_INTERFACE ) );

        data = (uint32_t*)host_buffer_get_current_piece_data_pointer( response );

        if ( *data == WPA2_AUTH_PSK )
        {
            *security |= WPA2_SECURITY;
        }
        else if ( *data == WPA_AUTH_PSK )
        {
            *security |= WPA_SECURITY;
        }
        host_buffer_release( response, WWD_NETWORK_RX );
    }
    else if ( *security != WICED_SECURITY_OPEN )
    {
        *security = WICED_SECURITY_UNKNOWN;
        result    = WWD_UNKNOWN_SECURITY_TYPE;
    }

    return result;
}

wwd_result_t wwd_wifi_set_ht_mode( wwd_interface_t interface, wiced_ht_mode_t ht_mode )
{
    wiced_buffer_t buffer;
    uint32_t*      data;

    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(uint32_t), IOVAR_STR_MIMO_BW_CAP );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) ht_mode;
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, interface ) );

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_get_ht_mode( wwd_interface_t interface, wiced_ht_mode_t* ht_mode )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    uint32_t*      data;

    /* Validate the input arguments */
    wiced_assert("Bad args", ht_mode != NULL);

    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(uint32_t), IOVAR_STR_MIMO_BW_CAP );
    CHECK_IOCTL_BUFFER( data );
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, interface ) );

    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    *ht_mode = (wiced_ht_mode_t)(*data);
    host_buffer_release( response, WWD_NETWORK_RX );

    return WWD_SUCCESS;
}


wwd_result_t wwd_wifi_set_passphrase( const uint8_t* security_key, uint8_t key_length, wwd_interface_t interface )
{
    wiced_buffer_t buffer;
    wsec_pmk_t* psk = (wsec_pmk_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wsec_pmk_t) );

    CHECK_IOCTL_BUFFER( psk );
    memset( psk, 0, sizeof(wsec_pmk_t) );
    memcpy( psk->key, security_key, key_length );
    psk->key_len = key_length;
    psk->flags = (uint16_t) WSEC_PASSPHRASE;

    host_rtos_delay_milliseconds( 1 ); /* Delay required to allow radio firmware to be ready to receive PMK and avoid intermittent failure */

    return wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_WSEC_PMK, buffer, 0, interface );
}


/******************************************************
 *             Static Functions
 ******************************************************/

/** Searches for a specific WiFi Information Element in a byte array
 *
 * Traverse a string of 1-byte tag/1-byte length/variable-length value
 * triples, returning a pointer to the substring whose first element
 * matches tag
 *
 * @note : This funciton has been copied directly from the standard broadcom host driver file wl/exe/wlu.c
 *
 *
 * @param tlv_buf : The byte array containing the Information Elements (IEs)
 * @param buflen  : The length of the tlv_buf byte array
 * @param key     : The Information Element tag to search for
 *
 * @return    NULL : if no matching Information Element was found
 *            Non-Null : Pointer to the start of the matching Information Element
 */

static inline /*@null@*/ tlv8_header_t* wlu_parse_tlvs( const tlv8_header_t* tlv_buf, uint32_t buflen, dot11_ie_id_t key )
{

    return (tlv8_header_t*) tlv_find_tlv8( (const uint8_t*) tlv_buf, buflen, key );
}

/** Checks if a WiFi Information Element is a WPA entry
 *
 * Is this body of this tlvs entry a WPA entry? If
 * not update the tlvs buffer pointer/length
 *
 * @note : This funciton has been copied directly from the standard broadcom host driver file wl/exe/wlu.c
 *
 * @param wpaie    : The byte array containing the Information Element (IE)
 * @param tlvs     : The larger IE array to be updated if not a WPA IE
 * @param tlvs_len : The current length of larger IE array
 *
 * @return    WICED_TRUE  : if IE matches the WPA OUI (Organizationally Unique Identifier) and its type = 1
 *            WICED_FALSE : otherwise
 */
static wiced_bool_t wlu_is_wpa_ie( vendor_specific_ie_header_t* wpaie, tlv8_header_t** tlvs, uint32_t* tlvs_len )
{
    vendor_specific_ie_header_t* ie = wpaie;

    /* If the contents match the WPA_OUI and type=1 */
    if ( ( ie->tlv_header.length >= (uint8_t) VENDOR_SPECIFIC_IE_MINIMUM_LENGTH ) &&
         ( memcmp( ie->oui, WPA_OUI_TYPE1, sizeof(ie->oui) ) == 0 ) )
    {
        /* Found the WPA IE */
        return WICED_TRUE;
    }

    /* point to the next ie */
    *tlvs = (tlv8_header_t*)( ((uint8_t*) ie) + ie->tlv_header.length + sizeof(tlv8_header_t) );

    /* calculate the length of the rest of the buffer */
    *tlvs_len -= (uint32_t) ( *tlvs - (tlv8_header_t*)ie );

    return WICED_FALSE;
}


wwd_result_t wwd_wifi_get_wifi_version( char* version, uint8_t length )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wwd_result_t   result;
    uint32_t*      data;

    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, length, IOVAR_STR_VERSION );
    CHECK_IOCTL_BUFFER( data );

    result = wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE );
    if ( result == WWD_SUCCESS )
    {
        char* wlan_firmware_version = (char*)host_buffer_get_current_piece_data_pointer( response );

        if ( wlan_firmware_version != NULL )
        {
            uint32_t version_length = MIN( strlen( wlan_firmware_version ), length );

            memcpy( version, wlan_firmware_version, version_length );

            if ( version[version_length - 1] == '\n' )
            {
                /* Remove newline in WLAN firmware version string. Formatting is handled by WPRINT below */
                version[version_length - 1] = '\0';
            }
        }

        host_buffer_release( response, WWD_NETWORK_RX );
    }
    return result;
}

wwd_result_t wwd_wifi_enable_minimum_power_consumption( void )
{
    wiced_buffer_t buffer;
    uint32_t*      data;
    wwd_result_t   result;

    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, "mpc" );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) 1;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE );
    wiced_assert("mpc 1 failed", result == WWD_SUCCESS );
    return result;
}

static wwd_result_t wwd_wifi_check_join_status( wwd_interface_t interface )
{
    switch ( wiced_join_status[ interface ] )
    {
        case JOIN_NO_NETWORKS :
            return WWD_NETWORK_NOT_FOUND;

        case JOIN_AUTHENTICATED | JOIN_LINK_READY | JOIN_SECURITY_COMPLETE | JOIN_COMPLETE :
            return WWD_SUCCESS;

        case 0:
        case JOIN_SECURITY_COMPLETE : /* For open/WEP */
            return WWD_NOT_AUTHENTICATED;

        case JOIN_AUTHENTICATED | JOIN_LINK_READY | JOIN_SECURITY_COMPLETE :
            return WWD_JOIN_IN_PROGRESS;

        case JOIN_AUTHENTICATED | JOIN_LINK_READY :
        case JOIN_AUTHENTICATED | JOIN_LINK_READY | JOIN_COMPLETE :
            return WWD_NOT_KEYED;

        default:
            return WWD_INVALID_JOIN_STATUS;
    }
}

//#pragma arm section code
#endif


