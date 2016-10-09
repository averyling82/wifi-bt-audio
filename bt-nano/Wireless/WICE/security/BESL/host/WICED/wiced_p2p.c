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

#include "wiced.h"
#include "wiced_p2p.h"
#include "../WWD/internal/wwd_sdpcm.h"
#include "wiced_wps.h"
#include "besl_host_rtos_structures.h"
#include "../internal/wiced_internal_api.h"
#include "wiced_security.h"
#include "p2p_constants.h"
#include "p2p_structures.h"
#include "p2p_frame_writer.h"
#include "wwd_buffer_interface.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define CHECK_IOCTL_BUFFER( buff )  if ( buff == NULL ) {  wiced_assert("Allocation failed\n", 0 == 1); return BESL_BUFFER_ALLOC_FAIL; }

/******************************************************
 *                    Constants
 ******************************************************/

#define DOT11_PMK_LEN                 (32)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    uint32_t bsscfgidx;
    char     ifname[16];
} wl_p2p_ifq_t;

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const wiced_ip_setting_t p2p_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192, 168, 10,  1) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255, 255, 255, 0) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192, 168, 10,  1) ),
};

/******************************************************
 *               Function Definitions
 ******************************************************/

besl_result_t besl_p2p_init( p2p_workspace_t* workspace, const besl_p2p_device_detail_t* device_details )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    uint32_t*      data;
    wwd_result_t   result;
    REFERENCE_DEBUG_ONLY_VARIABLE(result);

    memset( workspace, 0, sizeof(p2p_workspace_t) );

    workspace->group_owner_intent             = device_details->group_owner_intent;
    workspace->listen_channel.operating_class = device_details->listen_channel.operating_class;
    workspace->listen_channel.channel         = device_details->listen_channel.channel;
    workspace->group_formation_timeout        = device_details->group_formation_timeout;
    workspace->device_name_length             = strlen(device_details->wps_device_details.device_name);
    memcpy( workspace->listen_channel.country_string, &device_details->listen_channel.country_string,  3 );
    memcpy( workspace->device_name,                    device_details->wps_device_details.device_name, workspace->device_name_length );

    /* Turn off all the other Wi-Fi interfaces */
    wiced_network_down( WWD_STA_INTERFACE );
    wiced_network_down( WWD_AP_INTERFACE );
    wiced_network_down( WWD_P2P_INTERFACE );

    /* Query the AP interface to ensure that it is up XXX is this always going to be 36? */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 36, IOVAR_STR_BSSCFG_SSID );
    CHECK_IOCTL_BUFFER( data );
    memset(data, 0, 36);
    data[0] = (uint32_t) WWD_AP_INTERFACE;
    wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE );

    /* Turn roaming off for P2P */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, 4, "roam_off");
    CHECK_IOCTL_BUFFER( data );
    *data = 1;
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    /* Override MAC address because they do */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, sizeof(besl_mac_t), "p2p_da_override");
    CHECK_IOCTL_BUFFER( data );
    memset(data, 0, sizeof(besl_mac_t));
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    /* Turn off MPC or action frame sequence numbers are broken as well as other things */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_MPC );
    CHECK_IOCTL_BUFFER( data );
    *data = 0;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    wiced_assert("", result == WWD_SUCCESS);

    /*  Enable discovery. This sets the p2p device address used by the firmware to be the locally administered version of the STA MAC address */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, 4, "p2p_disc");
    CHECK_IOCTL_BUFFER( data );
    *data = 1;
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    /* Save the original STA MAC address */
    besl_host_get_mac_address(&workspace->original_mac_address, WWD_STA_INTERFACE );

    /* Generate P2P device (used in negotiation) and P2P interface MAC addresses (used by group client or owner) */
    memcpy(&workspace->p2p_device_address, &workspace->original_mac_address, sizeof(besl_mac_t));
    workspace->p2p_device_address.octet[0] |= 0x02;
    memcpy(&workspace->p2p_interface_address, &workspace->p2p_device_address, sizeof(besl_mac_t));
//    workspace->p2p_interface_address.octet[4] ^= 0x80;

    /* Check that we can read the p2p interface. This also appears necessary to bring it up. */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) sizeof(besl_mac_t), "p2p_if" );
    CHECK_IOCTL_BUFFER( data );
    memcpy(data, &workspace->p2p_device_address, sizeof(besl_mac_t));
    result = wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE );

    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("unable to read p2p interface\r\n"));
        return result;
    }

    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    wl_p2p_ifq_t* go_if = (wl_p2p_ifq_t*)data;
    BESL_DEBUG(("p2p interface %u, %s\n", (unsigned int)go_if->bsscfgidx, go_if->ifname));
    workspace->p2p_interface = go_if->bsscfgidx;
    host_buffer_release( response, WWD_NETWORK_RX );

    /* Get the P2P interface address */
    besl_host_get_mac_address(&workspace->device_info.p2p_device_address, workspace->p2p_interface);

    BESL_DEBUG(("STA MAC: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", workspace->device_info.p2p_device_address.octet[0],
        workspace->device_info.p2p_device_address.octet[1],
        workspace->device_info.p2p_device_address.octet[2],
        workspace->device_info.p2p_device_address.octet[3],
        workspace->device_info.p2p_device_address.octet[4],
        workspace->device_info.p2p_device_address.octet[5]));

    /* Set the device details */
    workspace->wps_device_details = &device_details->wps_device_details;

    /* Allow the P2P library to initialize */
    besl_p2p_init_common(workspace, device_details);

    /* Bring up P2P interface */
    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer( &buffer, 0 ) );

    result = wwd_sdpcm_send_ioctl(SDPCM_SET, WLC_UP, buffer, NULL, workspace->p2p_interface);
    wiced_assert("", result == WWD_SUCCESS);

    /* Set wsec to WPA2-AES in the discovery bsscfg to ensure our P2P probe responses have the privacy bit set in the 802.11 WPA IE.
     * Some peer devices may not initiate WPS with us if this bit is not set. */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, 8, "bsscfg:wsec");
    CHECK_IOCTL_BUFFER( data );
    data[0] = workspace->p2p_interface;
    data[1] = WICED_SECURITY_WPA2_AES_PSK;
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    /*  Add P2P event handler on the STA interface since the P2P interface is not yet up */
    result = wwd_management_set_event_handler( p2p_discovery_events, p2p_event_handler, workspace, WWD_STA_INTERFACE );
    wiced_assert("", result == WWD_SUCCESS);

    /* Create the P2P thread */
    p2p_thread_start( workspace );

    return BESL_SUCCESS;
}

// XXX make this p2p_init_common and have a separate function for initialising a GO from an application
besl_result_t besl_p2p_init_common( p2p_workspace_t* workspace, const besl_p2p_device_detail_t* device_details )
{
    uint8_t a;
    uint8_t random_suffix[2];

    workspace->channel_list.operating_class    = device_details->operating_channel.operating_class;
    workspace->channel_list.number_of_channels = device_details->channel_list.number_of_channels;
    memcpy( &workspace->channel_list.channel_list, device_details->channel_list.channel_list, device_details->channel_list.number_of_channels );
    workspace->device_info.config_methods      = htobe16( device_details->wps_device_details.config_methods );
    workspace->configuration_timeout           = ( device_details->client_configuration_timeout << 8 ) | device_details->go_configuration_timeout;
    memcpy(workspace->channel_list.country_string, device_details->channel_list.country_string, 3);
    workspace->p2p_capability                  = device_details->p2p_capability;
    workspace->allowed_configuration_methods   = device_details->wps_device_details.config_methods;
    memcpy(&workspace->operating_channel, &device_details->operating_channel, sizeof(p2p_channel_info_t));
    memcpy(&workspace->group_candidate.operating_channel, &device_details->operating_channel, sizeof(p2p_channel_info_t));

    workspace->device_info.number_of_secondary_devices = 0; // XXX device info should be included in device details
    workspace->device_info.primary_type.category       = 0x0a00;
    workspace->device_info.primary_type.sub_category   = 0x0500;
    workspace->device_info.primary_type.oui            = 0x04f25000;

    workspace->p2p_wps_device_password_id = device_details->device_password_id;
    workspace->peer_device_timeout = device_details->peer_device_timeout;

    /* Prepare random GO SSID if necessary */
    if ( workspace->group_candidate.ssid_length == 0 )
    {
        workspace->group_candidate.ssid_length = P2P_SSID_PREFIX_LENGTH + 2;
        memcpy( workspace->group_candidate.ssid, P2P_SSID_PREFIX, P2P_SSID_PREFIX_LENGTH );
        besl_host_random_bytes(random_suffix, 2);
        for (a = 0; a < 2; ++a)
        {
            random_suffix[a] = '0' + (random_suffix[a] % 62);
            if (random_suffix[a] > '9')
            {
                random_suffix[a] += 'A' - '9' - 1;
            }
            if (random_suffix[a] > 'Z')
            {
                random_suffix[a] += 'a' - 'Z' - 1;
            }
        }
        memcpy( &workspace->group_candidate.ssid[P2P_SSID_PREFIX_LENGTH], random_suffix, 2 );
    }

    if (workspace->i_am_group_owner != 1)
    {
        /*  Add WPS IE into both the probe request and response */
        p2p_write_wps_probe_request_ie( workspace);
        p2p_write_wps_probe_response_ie( workspace);

        /*  Add P2P IE into the probe request after the WPS IE */
        p2p_write_probe_request_ie( workspace );

        /*  Add P2P IE into the probe response after the WPS IE */
        p2p_write_probe_response_ie( workspace );
    }

    return BESL_SUCCESS;
}


besl_result_t besl_p2p_deinit( p2p_workspace_t* workspace )
{
    return p2p_deinit( workspace );
}

besl_result_t besl_p2p_start( p2p_workspace_t* workspace )
{
    p2p_message_t message;
    message.type = P2P_EVENT_START_REQUESTED;
    message.data = NULL;
    p2p_host_send_message( &message, WICED_WAIT_FOREVER );
    /* Delay to allow the p2p thread to start and run */
    host_rtos_delay_milliseconds( 10 );

    return BESL_SUCCESS;
}

p2p_discovered_device_t* besl_p2p_host_find_device(p2p_workspace_t* workspace, const besl_mac_t* mac)
{
    int a = 0;
    p2p_discovered_device_t* device = NULL;

    /* Return a pointer to an existing device, or an available entry in the table, or NULL */
    while ( a < P2P_MAX_DISCOVERED_DEVICES )
    {
        if ( ( workspace->discovered_devices[a].status != P2P_DEVICE_INVALID ) &&
             ( memcmp( &workspace->discovered_devices[a].p2p_device_address, mac, sizeof(besl_mac_t) ) == 0 ) )
        {
            return &workspace->discovered_devices[a];
        }
        if ( workspace->discovered_devices[a].status == P2P_DEVICE_INVALID )
        {
            device = &workspace->discovered_devices[a];
        }
        a++;
    }

    return device;
}


// XXX need to change this function so it accepts a destination MAC address as one of the arguments. May also need to know action frame type and sub-type.
besl_result_t besl_p2p_send_action_frame( p2p_workspace_t* workspace, const p2p_discovered_device_t* device, p2p_action_frame_writer_t writer, uint32_t channel, wwd_interface_t interface, uint32_t dwell_time )
{
    wiced_buffer_t  buffer;
    wl_af_params_t* frame;
    p2p_message_t   message;
    wwd_result_t    result;

    workspace->current_channel = channel;

    uint32_t* a = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_af_params_t) + 4, "bsscfg:actframe" );
    CHECK_IOCTL_BUFFER( a );
    *a = workspace->p2p_interface;
    frame = (wl_af_params_t*) ( a + 1 );
    frame->channel    = channel;
    frame->dwell_time = dwell_time;

    memcpy( &frame->action_frame.da, &device->p2p_device_address, sizeof(besl_mac_t) );

    /*
     * When communication is not within a P2P Group, e.g. during Service Discovery, P2P Invitation, GO Negotiation and Device Discoverability,
     * a P2P Device shall use the P2P Device Address of the intended destination as the BSSID in Request, or Confirmation frames and its own
     *  P2P Device Address as the BSSID in Response frames.
    */
    if ( workspace->group_owner_is_up == 0 )
    {
        memcpy( &frame->BSSID, &frame->action_frame.da, 6 );
    }
    else
    {
        memcpy( &frame->BSSID, &workspace->p2p_interface_address, 6 );
    }

    uint8_t* end_of_data = writer( workspace, device, frame->action_frame.data );

    frame->action_frame.len      = end_of_data - frame->action_frame.data;
    frame->action_frame.packetId = htobe32(workspace->p2p_action_frame_cookie); // It comes back in reverse order in the event that occurs when the packet is ACKed
    ++workspace->p2p_action_frame_cookie;

    message.type = P2P_EVENT_PACKET_TO_BE_SENT;
    message.data = buffer;
    result = p2p_host_send_message( &message, WICED_WAIT_FOREVER );

    if ( result != WWD_SUCCESS)
    {
        return result;
    }

    return BESL_SUCCESS;
}

besl_result_t besl_p2p_start_registrar( void )
{
    p2p_message_t   message;
    wwd_result_t    result;

    message.type = P2P_EVENT_START_REGISTRAR;
    message.data = NULL;
    result = p2p_host_send_message( &message, WICED_WAIT_FOREVER );

    if ( result != WWD_SUCCESS)
    {
        return result;
    }

    return BESL_SUCCESS;
}

void besl_p2p_host_negotiation_complete( p2p_workspace_t* workspace )
{
    p2p_message_t message;
    message.type = P2P_EVENT_NEGOTIATION_COMPLETE;
    message.data = NULL;
    p2p_host_send_message( &message, WICED_WAIT_FOREVER );
}

besl_result_t besl_p2p_get_group_formation_progress( p2p_workspace_t* workspace )
{
    return workspace->p2p_result;
}

besl_result_t besl_p2p_go_get_client_wps_progress( p2p_workspace_t* workspace )
{
    if (workspace->p2p_current_state == P2P_STATE_GROUP_OWNER)
    {
        switch (workspace->p2p_wps_agent->wps_result)
        {
            case WPS_NOT_STARTED: return BESL_P2P_GROUP_OWNER_WAITING_FOR_CONNECTION;
            case WPS_IN_PROGRESS: return BESL_P2P_GROUP_OWNER_WPS_IN_PROGRESS;
            case WPS_COMPLETE:    return BESL_P2P_GROUP_OWNER_WPS_COMPLETE;
            case WPS_PBC_OVERLAP: return BESL_P2P_GROUP_OWNER_WPS_PBC_OVERLAP;
            case WPS_TIMEOUT:     return BESL_P2P_GROUP_OWNER_WPS_TIMED_OUT;
            case WPS_ABORTED:     return BESL_P2P_GROUP_OWNER_WPS_ABORTED;

            default:
                return BESL_P2P_UNKNOWN;
        }
    }

    return workspace->p2p_result;
}

besl_result_t besl_p2p_start_negotiation( p2p_workspace_t* workspace )
{
    besl_result_t  result = BESL_SUCCESS;
    p2p_message_t message;

    message.type = P2P_EVENT_START_NEGOTIATION;
    message.data = NULL;
    p2p_host_send_message( &message, WICED_NO_WAIT );

    return result;
}

besl_result_t besl_p2p_find_group_owner( p2p_workspace_t* workspace )
{
    besl_result_t  result = BESL_SUCCESS;
    p2p_message_t message;

    message.type = P2P_EVENT_FIND_GROUP_OWNER;
    message.data = NULL;
    p2p_host_send_message( &message, WICED_WAIT_FOREVER );

    return result;
}

besl_result_t besl_p2p_get_discovered_peers( p2p_workspace_t* workspace, p2p_discovered_device_t** devices, uint8_t* device_count )
{
    *devices = workspace->discovered_devices;
    *device_count = workspace->discovered_device_count;

    return BESL_SUCCESS;
}

besl_result_t besl_p2p_group_owner_start( p2p_workspace_t* workspace )
{
    uint32_t*             data;
    wwd_result_t          result;
    wiced_buffer_t        buffer;
    wiced_buffer_t        response;
    uint8_t               pmk[DOT11_PMK_LEN + 8]; // PMK storage must be 40 octets in length for use in the function that converts from passphrase to pmk
    uint8_t               security_key[64];
    wsec_pmk_t*           psk;

    BESL_DEBUG( ("Group owner start entry\n") );

    /* Clear the p2p event queue before bringing up group owner */
    if ( workspace->p2p_thread_running == 1 )
    {
        p2p_clear_event_queue();
    }

    workspace->p2p_wps_agent = besl_host_calloc( "p2p go wps agent", 1, sizeof(wps_agent_t) );
    if ( workspace->p2p_wps_agent == NULL )
    {
        return BESL_ERROR_OUT_OF_MEMORY;
    }
    memset( workspace->p2p_wps_agent, 0, sizeof(wps_agent_t) );

    /* Re-init some parts of the p2p workspace since we may be coming from group negotiation */
    memset( workspace->discovered_devices, 0, sizeof(p2p_discovered_device_t) * P2P_MAX_DISCOVERED_DEVICES );
    workspace->discovered_device_count = 0;

    /* Generate a random passphrase if necessary */
    if ( workspace->p2p_passphrase_length == 0 )
    {
        workspace->p2p_passphrase_length = 8;
        besl_802_11_generate_random_passphrase( (char *)workspace->p2p_passphrase, workspace->p2p_passphrase_length );
    }

    if ( workspace->p2p_passphrase_length != 64 )
    {
        memset(pmk, 0, DOT11_PMK_LEN + 8);
        if ( besl_802_11_generate_pmk( (char *)workspace->p2p_passphrase, (unsigned char *)workspace->group_candidate.ssid, workspace->group_candidate.ssid_length, (unsigned char*)pmk ) != BESL_SUCCESS )
        {
            return BESL_P2P_ERROR_FAIL;
        }
        besl_host_hex_bytes_to_chars( (char*)security_key, pmk, DOT11_PMK_LEN );
    }
    else
    {
        memcpy( security_key, workspace->p2p_passphrase, 64 );
    }

    /* Turn off all the other Wi-Fi interfaces */
    wiced_network_down( WWD_STA_INTERFACE );
    wiced_network_down( WWD_AP_INTERFACE );
    wiced_network_down( WWD_P2P_INTERFACE );

    /* Query the AP interface to ensure that it has been initialized before creating P2P interface */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 36, IOVAR_STR_BSSCFG_SSID );
    CHECK_IOCTL_BUFFER( data );
    memset(data, 0, 36);
    data[0] = (uint32_t) WWD_AP_INTERFACE;
    wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE );

    /* Save the original STA MAC address */
    besl_host_get_mac_address(&workspace->original_mac_address, WWD_STA_INTERFACE );

    /* Create device address */
    memcpy(&workspace->p2p_device_address, &workspace->original_mac_address, sizeof(besl_mac_t));
    workspace->p2p_device_address.octet[0] |= 0x02;
    memcpy( &workspace->group_candidate.p2p_device_address, &workspace->p2p_device_address, sizeof(besl_mac_t) );
    memcpy( &workspace->group_candidate.bssid, &workspace->p2p_device_address, sizeof(besl_mac_t) );

    /* Create p2p interface address */
    memcpy(&workspace->p2p_interface_address, &workspace->p2p_device_address, sizeof(besl_mac_t));
    //workspace->p2p_interface_address.octet[4] ^= 0x80;

    /* Turn roaming off for P2P */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, 4, "roam_off");
    CHECK_IOCTL_BUFFER( data );
    *data = 1;
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    /* Override MAC address */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, sizeof(besl_mac_t), "p2p_da_override");
    CHECK_IOCTL_BUFFER( data );
    memset(data, 0, sizeof(besl_mac_t));
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    /* Turn off MPC or action frame sequence numbers are broken as well as other things */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_MPC );
    CHECK_IOCTL_BUFFER( data );
    *data = 0;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    wiced_assert("", result == WWD_SUCCESS);

    /* If the group owner is created as a result of negotiation then set the operating channel from the negotiation phase,
     * otherwise use the pre-configured operating channel.
     */
    if ( workspace->p2p_thread_running == 1 )
    {
        memcpy( &workspace->operating_channel, &workspace->group_candidate.operating_channel, sizeof(p2p_channel_info_t) );
    }
    /* Make sure the current channel is the operating channel. XXX we may remove current channel later */
    workspace->current_channel = (uint32_t) workspace->operating_channel.channel;

    /* Create P2P interface */
    wl_p2p_if_t* p2p_if = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_p2p_if_t), "p2p_ifadd" );
    CHECK_IOCTL_BUFFER( p2p_if );
    p2p_if->interface_type = P2P_GROUP_OWNER_MODE;
    p2p_if->chan_spec = workspace->operating_channel.channel | WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE;

    memcpy( &p2p_if->mac_address, &workspace->p2p_interface_address, sizeof(besl_mac_t) );
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE );
    if (result != WWD_SUCCESS)
    {
        BESL_DEBUG(("p2p_ifadd fail\r\n"));
    }
    uint32_t scb_timeout = 20;
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) sizeof(uint32_t) );
    CHECK_IOCTL_BUFFER( data );
    *data = scb_timeout;
    result = wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_SCB_TIMEOUT, buffer, 0, WWD_STA_INTERFACE );
    if (result != WWD_SUCCESS)
    {
        BESL_DEBUG(("set scb timeout fail\r\n"));
    }

    /* Read the p2p interface so it comes up */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) sizeof(besl_mac_t), "p2p_if" );
    CHECK_IOCTL_BUFFER( data );
    memcpy(data, &workspace->p2p_interface_address, sizeof(besl_mac_t));
    result = wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("unable to read go p2p interface\r\n"));
        return result;
    }

    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    host_buffer_release( response, WWD_NETWORK_RX );

    /* Bring up P2P interface - if we don't bring up the interface then things are flaky, e.g. negotiation request goes out with NULL BSSID and destination etc */
    CHECK_IOCTL_BUFFER( wwd_sdpcm_get_ioctl_buffer(&buffer, 0 ) );
    result = wwd_sdpcm_send_ioctl(SDPCM_SET, WLC_UP, buffer, NULL, workspace->p2p_interface);
    wiced_assert("", result == WWD_SUCCESS);

    /* Set the SSID */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 40, IOVAR_STR_BSSCFG_SSID );
    CHECK_IOCTL_BUFFER( data );
    data[0] = (uint32_t) WWD_P2P_INTERFACE; /* Set the bsscfg index */
    data[1] = strlen( workspace->group_candidate.ssid ); /* Set the ssid length */
    memcpy( &data[2], (uint8_t*)&workspace->group_candidate.ssid, data[1] );
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );

    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, IOVAR_STR_BSSCFG_WSEC );
    CHECK_IOCTL_BUFFER( data );
    data[0] = (uint32_t) WWD_P2P_INTERFACE;
    data[1] = (uint32_t) ( ( (WICED_SECURITY_WPA2_AES_PSK | WPS_ENABLED) & ( ~WPS_ENABLED ) ) | SES_OW_ENABLED );
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("unable to set wsec\r\n"));
    }

    /* Set the wpa auth */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, IOVAR_STR_BSSCFG_WPA_AUTH );
    CHECK_IOCTL_BUFFER( data );
    data[0] = (uint32_t) WWD_P2P_INTERFACE;
    data[1] = (uint32_t) WPA2_AUTH_PSK;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("unable to set wpa auth\r\n"));
    }

    /* Set the security key */
    psk = (wsec_pmk_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wsec_pmk_t) );
    CHECK_IOCTL_BUFFER( psk );
    memcpy( psk->key, security_key, 64 );
    psk->key_len = 64;
    psk->flags   = (uint16_t) WSEC_PASSPHRASE;
    host_rtos_delay_milliseconds( 1 ); // Delay required to allow radio firmware to be ready to receive PMK and avoid intermittent failure
    result = wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_WSEC_PMK, buffer, 0, WWD_P2P_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("unable to set passphrase\r\n"));
    }

    /* Restrict the number of associated devices */
    data   = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, "bss_maxassoc" );
    CHECK_IOCTL_BUFFER( data );
    *data  = P2P_MAX_ASSOCIATED_DEVICES;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("unable to set max associated devices\r\n"));
    }

    /* Set DTIM period */
    data   = wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( data );
    *data  = (uint32_t) WICED_DEFAULT_SOFT_AP_DTIM_PERIOD;
    result = wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_DTIMPRD, buffer, 0, WWD_P2P_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("unable to set dtim period\r\n"));
    }

    wwd_wifi_set_block_ack_window_size( WWD_P2P_INTERFACE );

    /* Restrict the BSS Rate Set to OFDM */
    data   = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, "bss_rateset" );
    CHECK_IOCTL_BUFFER( data );
    *data  = (uint32_t)1;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("unable to set BSS rate set\r\n"));
    }

    /* Bring up the BSS */
    data    = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, IOVAR_STR_BSS );
    CHECK_IOCTL_BUFFER( data );
    data[0] = (uint32_t) WWD_P2P_INTERFACE;
    data[1] = (uint32_t) 1; //BSS_UP;
    result  = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("unable to bring up BSS\r\n"));
    }

    besl_host_set_mac_address(&workspace->p2p_interface_address, WWD_STA_INTERFACE ); /* This is done so that deauthentication frames are sent with correct transmitter address */

    wwd_wifi_p2p_go_is_up = WICED_TRUE;

    workspace->p2p_interface = WWD_P2P_INTERFACE;
    memcpy( &workspace->device_info.p2p_device_address, &workspace->p2p_device_address, sizeof(besl_mac_t) );

    /* Bring up IP layer on P2P interface */
    result = wiced_ip_up( WWD_P2P_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &p2p_ip_settings );
    if ( result != WWD_SUCCESS )
    {
        workspace->p2p_current_state = P2P_STATE_FAILURE;
        workspace->p2p_result = BESL_P2P_ERROR_FAIL;
        return result;
    }

    host_rtos_delay_milliseconds( 10 ); // Delay to allow the DHCP thread to run or it may not be up when moving from negotiation to group owner

    workspace->p2p_wps_agent->is_p2p_registrar = 1;
    workspace->p2p_wps_agent->wps_agent_owner = workspace;
    memset(&workspace->p2p_wps_credential, 0, sizeof(besl_wps_credential_t));
    workspace->p2p_wps_credential.security = WICED_SECURITY_WPA2_AES_PSK;
    workspace->p2p_wps_credential.ssid.length = workspace->group_candidate.ssid_length;
    memcpy( workspace->p2p_wps_credential.ssid.value, workspace->group_candidate.ssid, workspace->p2p_wps_credential.ssid.length );
    workspace->p2p_wps_credential.passphrase_length = workspace->p2p_passphrase_length;
    memcpy( workspace->p2p_wps_credential.passphrase, workspace->p2p_passphrase, workspace->p2p_wps_credential.passphrase_length );
    result = besl_wps_init( workspace->p2p_wps_agent, workspace->wps_device_details, WPS_REGISTRAR_AGENT, WWD_P2P_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("besl_p2p_group_owner_start: error besl init %d\r\n", result));
        //stop_ap(0, NULL);
        return result;
    }
    workspace->p2p_wps_agent->wps_result_callback = workspace->p2p_wps_result_callback;

    workspace->p2p_capability |= P2P_GROUP_CAPABILITY_P2P_GROUP_OWNER;
    /* Check if a persistent group is being formed and if it is save the details. We don't check if it's new or not. */
    if ( workspace->form_persistent_group == 1 )
    {
        workspace->p2p_capability |= P2P_GROUP_CAPABILITY_P2P_PERSISTENT_GROUP;
        workspace->persistent_group.channel             = workspace->operating_channel.channel;
        workspace->persistent_group.security            = WICED_SECURITY_WPA2_AES_PSK;
        workspace->persistent_group.security_key_length = WSEC_MAX_PSK_LEN;
        memcpy( workspace->persistent_group.security_key, security_key, WSEC_MAX_PSK_LEN );
        workspace->persistent_group.SSID.length = workspace->group_candidate.ssid_length;
        memcpy( workspace->persistent_group.SSID.value, workspace->group_candidate.ssid, workspace->persistent_group.SSID.length );
    }
    workspace->configuration_timeout = 0;
    p2p_write_beacon_ie( workspace );
    p2p_write_probe_response_ie( workspace );

    workspace->p2p_current_state = P2P_STATE_GROUP_OWNER;
    workspace->i_am_group_owner  = 1;
    workspace->group_owner_is_up = 1;
    workspace->p2p_result        = BESL_SUCCESS;

    /*  Add P2P event handler */
    result = wwd_management_set_event_handler( p2p_group_owner_events, p2p_event_handler, workspace, WWD_P2P_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_ERROR(("Unable to set group owner event handler\n"));
    }
    wiced_assert("", result == WWD_SUCCESS);

    /* If the p2p thread is already running then we are here as a result of group owner negotiation and we don't want to start another thread. If the p2p thread
     * is not running then we are here as a result of starting an autonomous group owner and we do need to start the p2p thread.
     */
    if ( workspace->p2p_thread_running == 0 )
    {
        p2p_thread_start( workspace );
    }

    BESL_INFO( ( "P2P Group Owner starting. Group ID %02X:%02X:%02X:%02X:%02X:%02X %s\n",
            workspace->p2p_device_address.octet[0],
            workspace->p2p_device_address.octet[1],
            workspace->p2p_device_address.octet[2],
            workspace->p2p_device_address.octet[3],
            workspace->p2p_device_address.octet[4],
            workspace->p2p_device_address.octet[5],
            workspace->group_candidate.ssid ) );

    if ( workspace->p2p_group_formation_result_callback != NULL )
    {
        workspace->p2p_group_formation_result_callback( workspace );
    }

    return BESL_SUCCESS;
}

besl_result_t besl_p2p_client_enable_powersave( p2p_workspace_t* workspace, uint32_t power_save_mode )
{
    wiced_buffer_t buffer;
    uint32_t* data;

    /* Set legacy powersave mode - PM1 */
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER( data );
    *data = power_save_mode;
    return wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_PM, buffer, NULL, workspace->p2p_interface );
}

wiced_bool_t besl_p2p_group_owner_is_up( void )
{
    return wwd_wifi_p2p_go_is_up;
}

/* Callbacks */

void besl_p2p_register_p2p_device_connection_callback( p2p_workspace_t* workspace, void (*p2p_connection_request_callback)(p2p_discovered_device_t*) )
{
    workspace->p2p_connection_request_callback = p2p_connection_request_callback;
}

void besl_p2p_register_legacy_device_connection_callback( p2p_workspace_t* workspace, void (*p2p_legacy_device_connection_request_callback)(p2p_legacy_device_t*) )
{
    workspace->p2p_legacy_device_connection_request_callback = p2p_legacy_device_connection_request_callback;
}

void besl_p2p_register_group_formation_result_callback( p2p_workspace_t* workspace, void (*p2p_group_formation_result_callback)(void*) )
{
    workspace->p2p_group_formation_result_callback = p2p_group_formation_result_callback;
}

void besl_p2p_register_wpa2_client_association_callback( p2p_workspace_t* workspace, void (*p2p_wpa2_client_association_callback)(besl_mac_t*) )
{
    workspace->p2p_wpa2_client_association_callback = p2p_wpa2_client_association_callback;
}

void besl_p2p_register_wps_enrollee_association_callback( p2p_workspace_t* workspace, void (*p2p_wps_enrollee_association_callback)(besl_mac_t*) )
{
    workspace->p2p_wps_enrollee_association_callback = p2p_wps_enrollee_association_callback;
}

void besl_p2p_register_wps_result_callback( p2p_workspace_t* workspace, void (*p2p_wps_result_callback)(wps_result_t*) )
{
    workspace->p2p_wps_result_callback = p2p_wps_result_callback;
}

void besl_p2p_register_p2p_device_disassociation_callback( p2p_workspace_t* workspace, void (*p2p_device_disassociation_callback)(besl_mac_t*) )
{
    workspace->p2p_device_disassociation_callback = p2p_device_disassociation_callback;
}

void besl_p2p_register_legacy_device_disassociation_callback( p2p_workspace_t* workspace, void (*p2p_legacy_device_disassociation_callback)(besl_mac_t*) )
{
    workspace->p2p_legacy_device_disassociation_callback = p2p_legacy_device_disassociation_callback;
}


void p2p_host_add_vendor_ie( uint32_t interface, void* data, uint16_t data_length, uint32_t packet_mask )
{
    wwd_wifi_manage_custom_ie( interface, WICED_ADD_CUSTOM_IE, (uint8_t*) P2P_OUI, P2P_OUI_SUB_TYPE, data, data_length, packet_mask );
}

void p2p_host_remove_vendor_ie( uint32_t interface, void* data, uint16_t data_length, uint32_t packet_mask )
{
    wwd_wifi_manage_custom_ie( interface, WICED_REMOVE_CUSTOM_IE, (uint8_t*) P2P_OUI, P2P_OUI_SUB_TYPE, data, data_length, packet_mask );
}
