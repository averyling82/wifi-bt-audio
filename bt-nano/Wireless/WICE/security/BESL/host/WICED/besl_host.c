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

#include <string.h>

#include <stdarg.h>
#include "besl_host_interface.h"
#include "wwd_structures.h"
#include "wiced_utilities.h"
#include "wwd_wifi.h"
#include "wwd_crypto.h"
#include "wwd_network_constants.h"
#include "../../WWD/internal/wwd_sdpcm.h"
#include "besl_host_rtos_structures.h"
#include "RTOS/wwd_rtos_interface.h"
#include "../../WWD/include/network/wwd_network_interface.h"
#include "../../wwd_buffer_interface.h"
#include "wwd_assert.h"
#include "../../WWD/internal/wwd_internal.h"
#include "supplicant_structures.h"
#include "wwd_network_constants.h"
#include "besl_host_rtos_structures.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define CHECK_IOCTL_BUFFER( buff )  if ( buff == NULL ) {  wiced_assert("Allocation failed\n", 0 == 1); return BESL_BUFFER_ALLOC_FAIL; }

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
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

void* besl_host_malloc( const char* name, uint32_t size )
{
    BESL_DEBUG(("besl_host_malloc: %s %u\n", name, (unsigned int)size));
    return malloc_named( name, size );
}

void* besl_host_calloc( const char* name, uint32_t num, uint32_t size )
{
    void *ptr;
    ptr = besl_host_malloc( name, num * size );
    if ( ptr != NULL )
    {
        memset(ptr, 0, num * size);
    }
    return ptr;
}

void besl_host_free( void* p )
{
    free( p );
}

besl_result_t besl_host_get_mac_address(besl_mac_t* address, uint32_t interface )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wwd_result_t result;
    uint32_t*      data;

    data = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wiced_mac_t) + sizeof(uint32_t), IOVAR_STR_BSSCFG_CUR_ETHERADDR );
    CHECK_IOCTL_BUFFER( data );
    *data = interface;

    result = wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        memset( address->octet, 0, sizeof(wiced_mac_t) );
        return (besl_result_t) result;
    }
    memcpy( address, host_buffer_get_current_piece_data_pointer( response ), sizeof(wiced_mac_t) );
    host_buffer_release( response, WWD_NETWORK_RX );

    return BESL_SUCCESS;
}

besl_result_t besl_host_set_mac_address(besl_mac_t* address, uint32_t interface )
{
    wiced_buffer_t buffer;
    uint32_t*      data;

    data = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wiced_mac_t) + sizeof(uint32_t), "bsscfg:" IOVAR_STR_CUR_ETHERADDR );
    CHECK_IOCTL_BUFFER( data );
    data[0] = interface;
    memcpy(&data[1], address, sizeof(wiced_mac_t));

    return (besl_result_t) wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE );
}

void besl_host_random_bytes( uint8_t* buffer, uint16_t buffer_length )
{
    wwd_wifi_get_random( buffer, buffer_length );
}

void besl_host_get_time(besl_time_t* time)
{
    *time = (besl_time_t)host_rtos_get_time();
}

uint32_t besl_host_hton32(uint32_t intlong)
{
    return htobe32(intlong);
}

uint16_t besl_host_hton16(uint16_t intshort)
{
    return htobe16(intshort);
}


void besl_host_hex_bytes_to_chars( char* cptr, const uint8_t* bptr, uint32_t blen )
{
    int i,j;
    uint8_t temp;

    i = 0;
    j = 0;
    while( i < blen )
    {
        // Convert first nibble of byte to a hex character
        temp = bptr[i] / 16;
        if ( temp < 10 )
        {
            cptr[j] = temp + '0';
        }
        else
        {
            cptr[j] = (temp - 10) + 'A';
        }
        // Convert second nibble of byte to a hex character
        temp = bptr[i] % 16;
        if ( temp < 10 )
        {
            cptr[j+1] = temp + '0';
        }
        else
        {
            cptr[j+1] = (temp - 10) + 'A';
        }
        i++;
        j+=2;
    }
}

void besl_unit_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

besl_result_t besl_host_create_packet( besl_packet_t* packet, uint16_t size )
{
    wwd_result_t result;
    result = host_buffer_get( (wiced_buffer_t*) packet, WWD_NETWORK_TX, size + WICED_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX, WICED_TRUE );
    if ( result != WWD_SUCCESS )
    {
        *packet = 0;
        return result;
    }
    host_buffer_add_remove_at_front( (wiced_buffer_t*) packet, WICED_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX );

    return result;
}

void besl_host_consume_bytes( besl_packet_t* packet, int32_t number_of_bytes )
{
    host_buffer_add_remove_at_front( (wiced_buffer_t*) packet, number_of_bytes );
}

uint8_t* besl_host_get_data( besl_packet_t packet )
{
    return host_buffer_get_current_piece_data_pointer( packet );
}

besl_result_t besl_host_set_packet_size( besl_packet_t packet, uint16_t packet_length )
{
    return host_buffer_set_size( (wiced_buffer_t) packet, packet_length );
}

uint16_t besl_host_get_packet_size( besl_packet_t packet )
{
    return host_buffer_get_current_piece_size( packet );
}

void besl_host_free_packet( besl_packet_t packet )
{
    host_buffer_release( (wiced_buffer_t) packet, WWD_NETWORK_RX );
}

void besl_host_send_packet( void* workspace, besl_packet_t packet, uint16_t size )
{
    besl_host_workspace_t* host = (besl_host_workspace_t*) workspace;
    host_buffer_set_size( (wiced_buffer_t) packet, size );
    wwd_network_send_ethernet_data( packet, host->interface );
}

besl_result_t besl_host_leave( wwd_interface_t interface )
{
    wwd_wifi_leave( interface );
    return BESL_SUCCESS;
}

void besl_host_start_timer( void* workspace, uint32_t timeout )
{
    besl_host_workspace_t* host = (besl_host_workspace_t*) workspace;
    host->timer_reference = host_rtos_get_time( );
    host->timer_timeout = timeout;
}

void besl_host_stop_timer( void* workspace )
{
    besl_host_workspace_t* host = (besl_host_workspace_t*) workspace;
    host->timer_timeout = 0;
}

uint32_t besl_host_get_current_time( void )
{
    return host_rtos_get_time();
}

uint32_t besl_host_get_timer( void* workspace )
{
    besl_host_workspace_t* host = (besl_host_workspace_t*)workspace;
    return host->timer_timeout;
}

besl_result_t besl_queue_message_packet( void* workspace, besl_event_t type, besl_packet_t packet )
{
    besl_result_t result;
    supplicant_workspace_t* temp = (supplicant_workspace_t*)workspace;
    besl_host_workspace_t* host_workspace = (besl_host_workspace_t*)temp->supplicant_host_workspace;
    besl_event_message_t   message;
    message.event_type = type;
    message.data.packet = packet;
    result = (besl_result_t) host_rtos_push_to_queue( &host_workspace->event_queue, &message, WICED_NEVER_TIMEOUT );
    if ( result != BESL_SUCCESS )
    {
        host_buffer_release( (wiced_buffer_t) packet, WWD_NETWORK_RX );
    }
    return result;
}

besl_result_t besl_queue_message_uint( void* workspace, besl_event_t type, uint32_t value )
{
    besl_host_workspace_t* host_workspace = (besl_host_workspace_t*)workspace;
    besl_event_message_t   message;
    message.event_type = type;
    message.data.value = value;
    return (besl_result_t) host_rtos_push_to_queue( &host_workspace->event_queue, &message, WICED_NEVER_TIMEOUT );
}



void besl_get_bssid( besl_mac_t* mac )
{
    wwd_wifi_get_bssid( (wiced_mac_t*)mac );
}


besl_result_t besl_set_passphrase( const uint8_t* security_key, uint8_t key_length )
{
    return (besl_result_t)wwd_wifi_set_passphrase( security_key, key_length, WWD_STA_INTERFACE );
}
