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
#include "besl_host_rtos_structures.h"
#include "p2p_structures.h"
#include "wiced_p2p.h"
#include "p2p_host_interface.h"
#include "../../internal/wwd_sdpcm.h"
#include "wiced_wps.h"
#include "wps_host_interface.h"
#include "../internal/wiced_internal_api.h"
#include "wps_p2p_interface.h"
#include "p2p_frame_writer.h"
#include "wwd_buffer_interface.h"
#include "wiced_utilities.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define CHECK_IOCTL_BUFFER( buff )  if ( buff == NULL ) {  wiced_assert("Allocation failed\n", 0 == 1); return BESL_BUFFER_ALLOC_FAIL; }

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_NUMBER_OF_P2P_MESSAGES    (20)
#define P2P_THREAD_STACK_SIZE         (5*1024)
#define P2P_MAX_DISCOVERABLE_INTERVAL (3)
#define P2P_BEACON_INTERVAL_MS        (100)
#define P2P_LISTEN_MODE_DWELL_TIME    (300)     /* Milliseconds */
#define DOT11_CAP_PRIVACY             (0x0010)  /* d11 cap. privacy */
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

static void           p2p_thread_main                 ( uint32_t arg );
static besl_result_t  p2p_scan                        ( p2p_workspace_t* workspace, uint16_t scan_action, wiced_bool_t all_channels );
static wwd_result_t   p2p_set_discovery_state         ( p2p_discovery_state_t state, uint32_t channel );
static void           p2p_discover                    ( p2p_workspace_t* workspace );
static besl_result_t  p2p_start_client                ( p2p_workspace_t* workspace );
static besl_result_t  p2p_group_owner_stop            ( p2p_workspace_t* workspace );
static besl_result_t  p2p_group_client_stop           ( p2p_workspace_t* workspace );
static void           p2p_abort_scan                  ( p2p_workspace_t* workspace );
static besl_result_t  p2p_join_group_owner            ( p2p_workspace_t* workspace );
static void           p2p_start_timer                 ( void* workspace, uint32_t timeout );
void                  p2p_wps_internal_result_callback( wps_result_t* result );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static host_thread_type_t p2p_thread;
static host_queue_type_t  p2p_message_queue;
static p2p_message_t      p2p_message_queue_buffer        [ MAX_NUMBER_OF_P2P_MESSAGES ];
#ifdef RTOS_USE_STATIC_THREAD_STACK
static uint8_t            p2p_thread_stack                [ P2P_THREAD_STACK_SIZE ];
#else
#define p2p_thread_stack (NULL)
#endif /* ifdef RTOS_USE_STATIC_THREAD_STACK */
static const wwd_event_num_t p2p_group_client_events[] = { WLC_E_ESCAN_RESULT, WLC_E_PROBREQ_MSG, WLC_E_ACTION_FRAME, WLC_E_ACTION_FRAME_COMPLETE, WLC_E_DEAUTH, WLC_E_DEAUTH_IND, WLC_E_DISASSOC, WLC_E_DISASSOC_IND, WLC_E_NONE };

const wwd_event_num_t p2p_discovery_events[]    = { WLC_E_ESCAN_RESULT, WLC_E_P2P_DISC_LISTEN_COMPLETE, WLC_E_PROBREQ_MSG, WLC_E_ACTION_FRAME, WLC_E_ACTION_FRAME_COMPLETE, WLC_E_NONE };
const wwd_event_num_t p2p_group_owner_events[]  = { WLC_E_ACTION_FRAME, WLC_E_ASSOC_IND, WLC_E_REASSOC_IND, WLC_E_ACTION_FRAME_COMPLETE, WLC_E_DEAUTH, WLC_E_DEAUTH_IND, WLC_E_DISASSOC, WLC_E_DISASSOC_IND, WLC_E_PROBREQ_MSG, WLC_E_NONE };

/******************************************************
 *               Function Definitions
 ******************************************************/

void p2p_thread_start( p2p_workspace_t* workspace )
{
    /* Create the message queue */
    memset( p2p_message_queue_buffer , 0, sizeof(p2p_message_queue_buffer) );
    host_rtos_init_queue(&p2p_message_queue, p2p_message_queue_buffer, sizeof(p2p_message_queue_buffer), sizeof(p2p_message_t));

    /* Create the P2P thread */
    workspace->p2p_initialised = 1;
#ifdef RTOS_USE_STATIC_THREAD_STACK
    memset( p2p_thread_stack, 0, P2P_THREAD_STACK_SIZE );
#endif /* ifdef RTOS_USE_STATIC_THREAD_STACK */
    host_rtos_create_thread_with_arg( &p2p_thread, p2p_thread_main, "p2p_thread", p2p_thread_stack, P2P_THREAD_STACK_SIZE, RTOS_HIGHER_PRIORTIY_THAN(RTOS_DEFAULT_THREAD_PRIORITY), (uint32_t)workspace );
}

static void p2p_thread_main( uint32_t arg )
{
    p2p_workspace_t*              workspace = (p2p_workspace_t*)arg;
    wwd_result_t                  result;
    p2p_message_t                 message;
    wiced_time_t                  current_time;
    uint8_t                       tie_breaker[2];
    p2p_discovered_device_t       discovery_target;
    p2p_discovered_device_t       discovery_requestor;
    p2p_client_info_descriptor_t* associated_device;
    p2p_discovered_device_t*      peer_device;
    besl_mac_t*                   associated_legacy_device;
    wps_result_t*                 wps_result;

    if ( workspace->group_owner_is_up == 1 )
    {
        p2p_start_timer( workspace, 5000 );
    }
    else if ( workspace->group_client_is_up == 0 )
    {
        workspace->p2p_result = BESL_IN_PROGRESS;
    }
    workspace->p2p_thread_running = 1;

    while ( workspace->p2p_current_state != P2P_STATE_ABORTED )
    {
        uint32_t time_to_wait;

        /* Check for group formation timeout */
        if ( ( workspace->group_owner_is_up == 0 ) && ( workspace->group_client_is_up == 0 ) )
        {
            if ( host_rtos_get_time( ) > ( workspace->group_formation_start_time + workspace->group_formation_timeout ) )
            {
                if ( workspace->p2p_current_state == P2P_STATE_NEGOTIATING )
                {
                    if ( workspace->initiate_negotiation == 1 )
                    {
                        p2p_stop( workspace );
                    }
                    else
                    {
                        workspace->p2p_current_state = P2P_STATE_DISCOVERY; /* If we didn't initiate group formation then go back to discovery */
                    }
                }
                else if ( workspace->looking_for_group_owner == 1 )
                {
                    p2p_stop( workspace );
                }
            }
        }

        if ( workspace->timer_timeout > 0 )
        {
            current_time = host_rtos_get_time( );
            if ( ( workspace->timer_reference + workspace->timer_timeout ) <= current_time )
            {
                time_to_wait = 0;
            }
            else
            {
                time_to_wait = workspace->timer_timeout - (current_time - workspace->timer_reference);
            }
        }
        else
        {
            time_to_wait = WICED_NEVER_TIMEOUT;
        }

        if ( host_rtos_pop_from_queue(&p2p_message_queue, &message, time_to_wait ) != WWD_SUCCESS )
        {
            /* Create a timeout message */
            message.type = P2P_EVENT_TIMER_TIMEOUT;
            message.data = 0;
            workspace->timer_timeout = 0;
        }

        switch(message.type)
        {
            case P2P_EVENT_SCAN_COMPLETE:
                if ( workspace->p2p_current_state == P2P_STATE_DISCOVERY )
                    {
                    if ( workspace->looking_for_group_owner == 1 )
                    {
                        // XXX should check if device is actually a GO
                        workspace->candidate_device = besl_p2p_host_find_device( workspace, (besl_mac_t*)&workspace->group_candidate.p2p_device_address );
                        if ( workspace->candidate_device != NULL )
                        {
                            if ( !NULL_MAC( workspace->candidate_device->p2p_device_address.octet) )
                            {
                                memcpy(&workspace->group_candidate.operating_channel, &workspace->candidate_device->operating_channel, sizeof(p2p_channel_info_t));
                                memcpy(&workspace->group_candidate.bssid, &workspace->candidate_device->p2p_interface_address, sizeof(besl_mac_t));
                                besl_p2p_send_action_frame( workspace, workspace->candidate_device, p2p_write_provision_discovery_request, (uint32_t)workspace->candidate_device->operating_channel.channel, WICED_P2P_INTERFACE, 500 );
                            }
                            else
                            {
                                p2p_scan( workspace, WL_SCAN_ACTION_START, WICED_TRUE ); // Scan all channels when looking for an existing group owner
                            }
                        }
                        else
                        {
                            p2p_scan( workspace, WL_SCAN_ACTION_START, WICED_TRUE ); // Scan all channels when looking for an existing group owner
                        }
                    }
                    else
                    {
                        if ( workspace->initiate_negotiation == 1 )
                        {
                            workspace->candidate_device = besl_p2p_host_find_device( workspace, (besl_mac_t*)&workspace->group_candidate.p2p_device_address );
                            if ( !NULL_MAC(workspace->candidate_device->p2p_device_address.octet) )
                            {
                                BESL_DEBUG(("Found target device\n"));
                                message.type = P2P_EVENT_FOUND_TARGET_DEVICE;
                                message.data = NULL;
                                host_rtos_push_to_queue(&p2p_message_queue, &message, WICED_NEVER_TIMEOUT);
                            }
                            else
                            {
                                p2p_discover(workspace);
                            }
                        }
                        else
                        {
                            p2p_discover(workspace);
                        }
                    }
                }
                break;

            case P2P_EVENT_DISCOVERY_COMPLETE:
            case P2P_EVENT_START_REQUESTED:
                workspace->p2p_current_state = P2P_STATE_DISCOVERY;
                p2p_set_discovery_state( P2P_DISCOVERY_STATE_LISTEN, 0 );
                result = p2p_scan( workspace, WL_SCAN_ACTION_START, WICED_TRUE ); /* Scan social channels or scan all channels? GOs may not be on social channels. */
                break;

            case P2P_EVENT_START_NEGOTIATION:
                /* Create random tie breaker bit for the first negotiation request */
                besl_host_random_bytes(tie_breaker, 2); /* Have to ask for a minimum of two bytes */
                workspace->group_owner_tie_breaker = tie_breaker[0] & 0x01;
                workspace->group_formation_start_time = host_rtos_get_time( );
                p2p_discover(workspace);
                break;

            case P2P_EVENT_FIND_GROUP_OWNER:
                workspace->looking_for_group_owner = 1;
                workspace->group_formation_start_time = host_rtos_get_time( );
                break;

            case P2P_EVENT_PACKET_TO_BE_SENT:
                if ( ( workspace->sent_negotiation_request == 1 ) ||
                     ( workspace->sent_negotiation_confirm == 1 ) ||
                     ( workspace->sent_go_discoverability_request == 1 ) ||
                     ( workspace->sent_provision_discovery_request == 1 ) ||
                     ( workspace->sent_invitation_request == 1 ) )
                {
                    p2p_start_timer( workspace, 350 ); /* This value needs to be offset to the other devices listen interval and longer than our listen mode dwell time */
                }

                /* Abort scan to cancel previous action frame's dwell time and specifically set channel or our negotiation phase is unreliable */
                if ( workspace->p2p_current_state != P2P_STATE_GROUP_OWNER )
                {
                    p2p_abort_scan( workspace );
                    if ( workspace->current_channel != 0 )
                    {
                        result = wwd_wifi_set_channel( WWD_STA_INTERFACE, workspace->current_channel );
                    }
                }
                /* Send the packet */
                result = wwd_sdpcm_send_iovar( SDPCM_SET, (wiced_buffer_t) message.data, NULL, WWD_STA_INTERFACE );
                if (result != WWD_SUCCESS)
                {
                    /*  Packet has been lost.. Maybe. Don't think we can recover it though */
                    BESL_DEBUG(("P2P negotiating and lost the packet\r\n"));
                }
                if ( workspace->p2p_current_state == P2P_STATE_DISCOVERY )
                {
                    p2p_discover(workspace);
                }
                break;

            case P2P_EVENT_NEGOTIATION_COMPLETE:
                BESL_INFO( ("P2P negotiation complete...\n") );

                workspace->looking_for_group_owner = 0;

                /*  Remove P2P event handler */
                result = wwd_management_set_event_handler( p2p_discovery_events, NULL, workspace, WWD_STA_INTERFACE );
                wiced_assert("", result == WWD_SUCCESS);

                p2p_abort_scan( workspace ); /* Abort scan or the GO may not be able to come up */
                p2p_remove_ies( workspace );

                /*  Disable discovery */
                wiced_buffer_t buffer;
                uint32_t* data;

                p2p_set_discovery_state( P2P_DISCOVERY_STATE_SCAN, 0 );
                data = wwd_sdpcm_get_iovar_buffer(&buffer, 4, "p2p_disc");
                if ( data == NULL )
                {
                    wiced_assert("Allocation failed\n", 0 == 1);
                    return;
                }
                *data = 0;
                result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE);
                if (result != WWD_SUCCESS)
                {
                    BESL_DEBUG(("Unable to disable discovery\r\n"));
                }
                wiced_assert("", result == WWD_SUCCESS);

                if ( workspace->i_am_group_owner == 1 )
                {
                    besl_p2p_group_owner_start( workspace );
                    /* Start the registrar if we're not reinvoking a group via the invitation procedure */
                    if ( workspace->reinvoking_group == 0 )
                    {
                        besl_p2p_start_registrar();
                    }
                }
                else
                {
                    if ( workspace->group_candidate.configuration_timeout > 0 )
                    {
                        host_rtos_delay_milliseconds(workspace->group_candidate.configuration_timeout * 10); /* Run the delay here so we can still ACK negotiation confirmations */
                    }
                    workspace->p2p_current_state = P2P_STATE_NEGOTIATION_COMPLETE;
                    p2p_start_client( workspace );
                }
                break;

            case P2P_EVENT_STOP_REQUESTED:
                BESL_DEBUG(("Stopping P2P thread\n"));
                /* Free WPS agent if necessary */
                if ( workspace->p2p_wps_agent != NULL )
                {
                    if ( workspace->p2p_wps_agent->wps_result != WPS_NOT_STARTED )
                    {
                        if ( besl_wps_abort( workspace->p2p_wps_agent ) == BESL_SUCCESS )
                        {
                            BESL_DEBUG(("besl_p2p_deinit: P2P WPS agent stopped\r\n"));
                        }
                    }
                    host_rtos_delay_milliseconds( 10 ); /* Delay to allow the WPS thread to complete */
                    besl_wps_deinit( workspace->p2p_wps_agent );
                    besl_host_free( workspace->p2p_wps_agent );
                    workspace->p2p_wps_agent = NULL;
                }
                workspace->p2p_current_state = P2P_STATE_ABORTED;
                break;

            case P2P_EVENT_TIMER_TIMEOUT:
                if ( ( ( workspace->sent_negotiation_confirm == 1 ) || ( workspace->sent_negotiation_request == 1 ) ) && ( workspace->p2p_current_state == P2P_STATE_NEGOTIATING ) )
                {
                    workspace->sent_negotiation_confirm = 0;
                    besl_p2p_send_action_frame( workspace, workspace->candidate_device, p2p_write_negotiation_request, (uint32_t)workspace->candidate_device->listen_channel, WICED_P2P_INTERFACE, 2 );
                }
                else if ( workspace->sent_go_discoverability_request == 1 )
                {
                    memcpy( &discovery_target.p2p_device_address, &workspace->discovery_target->p2p_interface_address, sizeof( besl_mac_t ) );
                    besl_p2p_send_action_frame( workspace, &discovery_target, p2p_write_go_discoverability_request, workspace->current_channel, workspace->p2p_interface, 2 );
                }
                else if ( workspace->sent_provision_discovery_request == 1 )
                {
                    besl_p2p_send_action_frame( workspace, workspace->candidate_device, p2p_write_provision_discovery_request, (uint32_t)workspace->candidate_device->operating_channel.channel, WICED_P2P_INTERFACE, 500 );
                }
                else if ( workspace->sent_invitation_request == 1)
                {
                    besl_p2p_send_action_frame( workspace, workspace->candidate_device, p2p_write_invitation_request, workspace->current_channel, workspace->p2p_interface, 2 );
                    /* The timer is single threaded so kick off another 5 second interval in case we overwrote the last one */
                    if ( workspace->p2p_current_state == P2P_STATE_GROUP_OWNER )
                    {
                        p2p_start_timer( workspace, 5000 );
                    }
                }
                else if ( workspace->group_owner_is_up == 1 )
                {
                    p2p_update_devices( workspace );
                    p2p_start_timer( workspace, 5000 );
                }
                break;

            case P2P_EVENT_FOUND_TARGET_DEVICE:
                workspace->p2p_current_state = P2P_STATE_NEGOTIATING;
                BESL_DEBUG(("Device listen channel %u\n", (unsigned int)workspace->candidate_device->listen_channel));
                besl_p2p_send_action_frame( workspace, workspace->candidate_device, p2p_write_negotiation_request, (uint32_t)workspace->candidate_device->listen_channel, WICED_P2P_INTERFACE, 2 );
                workspace->candidate_device->status = P2P_DEVICE_WAS_INVITED_TO_FORM_GROUP;
                break;

            case P2P_EVENT_START_REGISTRAR:
                    /* Cleanup the WPS workspace, advertise selected registrar and then kick off the registrar */
                if (workspace->p2p_wps_agent->wps_result == WPS_IN_PROGRESS )
                {
                    if ( workspace->p2p_wps_agent->wps_mode == WPS_PBC_MODE )
                    {
                        WPRINT_APP_INFO(("Restarting 2 minute window\n"));
                        besl_wps_restart( workspace->p2p_wps_agent );
                    }
                    else
                    {
                        WPRINT_APP_INFO(("P2P thread: WPS already running %u\n", (unsigned int)workspace->p2p_wps_agent->wps_result));
                    }
                    break;
                }
                else if ( workspace->p2p_wps_agent->wps_result != WPS_NOT_STARTED )
                {
                    besl_wps_deinit( workspace->p2p_wps_agent );
                    memset( workspace->p2p_wps_agent, 0, sizeof(wps_agent_t) );
                    workspace->p2p_wps_agent->is_p2p_registrar = 1;
                    workspace->p2p_wps_agent->wps_agent_owner = workspace;
                    besl_wps_init( workspace->p2p_wps_agent, workspace->wps_device_details, WPS_REGISTRAR_AGENT, WWD_P2P_INTERFACE );
                    wps_register_result_callback( workspace->p2p_wps_agent, workspace->p2p_wps_result_callback );
                }

                wps_internal_init( workspace->p2p_wps_agent, WWD_P2P_INTERFACE, workspace->p2p_wps_mode, workspace->p2p_wps_pin, &workspace->p2p_wps_credential, 1 );
                p2p_write_probe_response_ie( workspace ); // Re-add the p2p IE after the WPS IE
                workspace->p2p_wps_agent->device_password_id = workspace->p2p_wps_device_password_id;

                /* Run WPS state machine in its own thread */
                besl_p2p_wps_start( workspace->p2p_wps_agent );
                host_rtos_delay_milliseconds( 10 ); /* Delay required to allow WPS thread to run and initialize */
                break;

            case P2P_EVENT_DEVICE_AWAKE:
                if ( workspace->sent_go_discoverability_request == 1 )
                {
                    workspace->sent_go_discoverability_request = 0;
                    memcpy( &discovery_requestor.p2p_device_address, &workspace->discovery_requestor, sizeof( besl_mac_t ) );
                    discovery_requestor.dialog_token = workspace->discovery_dialog_token;
                    discovery_requestor.status = 0; // Success
                    besl_p2p_send_action_frame( workspace, &discovery_requestor, p2p_write_device_discoverability_response, (uint32_t)workspace->operating_channel.channel, workspace->p2p_interface, 0 );
                }
                break;

            case P2P_EVENT_P2P_DEVICE_ASSOCIATED:
                /* Rewrite the probe response to add P2P client descriptors */
                if ( ( workspace->p2p_wpa2_client_association_callback != NULL ) && ( message.data != NULL ) )
                {
                    workspace->p2p_wpa2_client_association_callback( message.data );
                }
                host_rtos_delay_milliseconds( 10 ); /* Delay required to allow WPS thread to finish its rewriting of the probe response ie */
                p2p_write_probe_response_ie( workspace );
                break;

            case P2P_EVENT_LEGACY_DEVICE_ASSOCIATED:
                if ( ( workspace->p2p_wpa2_client_association_callback != NULL ) && ( message.data != NULL ) )
                {
                    workspace->p2p_wpa2_client_association_callback( message.data );
                }
                break;

            case P2P_EVENT_WPS_ENROLLEE_ASSOCIATED:
                if ( ( workspace->p2p_wps_enrollee_association_callback != NULL ) && ( message.data != NULL ) )
                {
                    workspace->p2p_wps_enrollee_association_callback( message.data );
                }
                break;

            case P2P_EVENT_CONNECTION_REQUESTED:
                BESL_DEBUG( ("P2P connection or invitation request\r\n") );
                /* Inform the user interface that a device wants to connect */
                if ( ( workspace->p2p_connection_request_callback != NULL ) && ( message.data != NULL ) )
                {
                    BESL_DEBUG(("Calling connection handler\r\n"));
                    workspace->p2p_connection_request_callback( message.data );
                }
                if ( workspace->p2p_connection_request_callback == NULL )
                {
                    BESL_DEBUG(("Connection request handler null\r\n"));
                }
                if ( message.data == NULL )
                {
                    BESL_DEBUG(("Message data null\r\n"));
                }

                break;

            case P2P_EVENT_LEGACY_DEVICE_CONNECTION_REQUEST:
                if ( ( workspace->p2p_current_state == P2P_STATE_GROUP_OWNER ) && ( workspace->p2p_legacy_device_connection_request_callback != NULL ) && ( message.data != NULL ) )
                {
                    workspace->p2p_legacy_device_connection_request_callback( (p2p_legacy_device_t*)message.data );
                }
                break;

            case P2P_EVENT_DEVICE_DISASSOCIATED:
                if ( message.data != NULL )
                {
                    if ( workspace->p2p_current_state == P2P_STATE_GROUP_OWNER )
                    {
                        /* If the current enrollee is disassociating then reset the registrar state machine */
                        if ( workspace->p2p_wps_agent != NULL )
                        {
                            besl_wps_reset_registrar( workspace->p2p_wps_agent, (besl_mac_t*)message.data );
                        }
                        /* If it's an associated P2P device then clear the entry in the associated P2P device list and rewrite the P2P IE in probe responses */
                        associated_device = p2p_host_find_associated_p2p_device( workspace, message.data );
                        if ( associated_device != NULL )
                        {
                            memset( associated_device, 0, sizeof( p2p_client_info_descriptor_t ) );
                            if ( workspace->associated_p2p_device_count > 0 )
                            {
                                workspace->associated_p2p_device_count--;
                            }
                            p2p_write_probe_response_ie( workspace );
                            if ( workspace->p2p_device_disassociation_callback != NULL )
                            {
                                workspace->p2p_device_disassociation_callback( message.data );
                            }
                        }
                        else
                        {
                            associated_legacy_device = p2p_host_find_associated_legacy_device( workspace, message.data );
                            if ( associated_legacy_device != NULL )
                            {
                                memset( associated_legacy_device, 0, sizeof( besl_mac_t ) );
                                if ( workspace->p2p_legacy_device_disassociation_callback != NULL )
                                {
                                    workspace->p2p_legacy_device_disassociation_callback( message.data );
                                }
                            }
                        }
                    }
                    else if ( workspace->p2p_current_state == P2P_STATE_GROUP_CLIENT )
                    {
                        /* client callback is tbd */
                    }
                    /* If the device is in the p2p peer list clear the entry. */
                    peer_device = besl_p2p_host_find_device( workspace, message.data );
                    if ( peer_device != NULL )
                    {
                        peer_device->status = P2P_DEVICE_INVALID;
                    }
                }
                break;

            case P2P_EVENT_WPS_ENROLLEE_COMPLETED:
                if ( message.data != NULL )
                {
                    wps_result = (wps_result_t*)message.data;
                    if ( *wps_result != WPS_COMPLETE )
                    {
                        workspace->p2p_result = BESL_P2P_ERROR_FAIL;
                        p2p_stop( workspace );
                    }
                    else
                    {
                        BESL_INFO( ("WPS successful for P2P client\r\n") );
                        p2p_join_group_owner( workspace );
                    }
                }
                break;

            default:
                break;
        }
    }

    /* Remove P2P related information elements from management frames */
    p2p_remove_ies( workspace );

    /*  Remove P2P event handler */
    if ( workspace->group_owner_is_up == 1 )
    {
        result = wwd_management_set_event_handler( p2p_group_owner_events, NULL, workspace, WICED_P2P_INTERFACE );
        p2p_group_owner_stop( workspace );
    }
    else if ( workspace->group_client_is_up == 1 )
    {
        result = wwd_management_set_event_handler( p2p_group_client_events, NULL, workspace, WICED_P2P_INTERFACE );
        /* If the WPS handshake failed then we end up here */
        if ( workspace->p2p_result == BESL_P2P_ERROR_FAIL )
        {
            if ( workspace->p2p_group_formation_result_callback != NULL )
            {
                workspace->p2p_group_formation_result_callback( workspace );
            }
        }
        p2p_group_client_stop( workspace );
    }
    else
    {
        wiced_buffer_t buffer;
        uint32_t* data;

        /*  Disable discovery */
        wwd_wifi_abort_scan();

        p2p_set_discovery_state( P2P_DISCOVERY_STATE_SCAN, 0 );
        data = wwd_sdpcm_get_iovar_buffer( &buffer, 4, "p2p_disc" );
        if ( data == NULL )
        {
            wiced_assert("Allocation failed\n", 0 == 1);
            return;
        }
        *data = 0;
        result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE);
        if (result != WWD_SUCCESS)
        {
            BESL_DEBUG(("P2P client: Unable to disable discovery\r\n"));
        }
        wiced_assert("", result == WWD_SUCCESS);
        if ( workspace->p2p_group_formation_result_callback != NULL )
        {
            workspace->p2p_group_formation_result_callback( workspace );
        }

        result = wwd_management_set_event_handler( p2p_discovery_events, NULL, workspace, WICED_STA_INTERFACE );
    }
    wiced_assert("", result == WWD_SUCCESS);
    p2p_clear_event_queue();
    workspace->p2p_thread_running = 0;

    host_rtos_finish_thread( &p2p_thread );
    WICED_END_OF_CURRENT_THREAD( );
}

besl_result_t p2p_stop( p2p_workspace_t* workspace )
{
    p2p_message_t message;

    /* Stop timer and drain message queue so we don't block */
    p2p_stop_timer( workspace );
    p2p_clear_event_queue();

    message.type = P2P_EVENT_STOP_REQUESTED;
    message.data = NULL;
    host_rtos_push_to_queue( &p2p_message_queue, &message, WICED_NEVER_TIMEOUT );

    return BESL_SUCCESS;
}

static besl_result_t p2p_start_client( p2p_workspace_t* workspace )
{
    wwd_result_t             result;
    wiced_buffer_t           buffer;
    wiced_buffer_t           response;
    uint32_t*                data;

    if ( workspace->reinvoking_group == 0 )
    {
        if ( workspace->p2p_wps_agent != NULL )
        {
            BESL_DEBUG(("wps agent not null\r\n"));
            besl_host_free( workspace->p2p_wps_agent );
            workspace->p2p_wps_agent = NULL;
        }
        workspace->p2p_wps_agent = besl_host_calloc( "p2p client wps agent", 1, sizeof(wps_agent_t) );
        if ( workspace->p2p_wps_agent == NULL )
        {
            workspace->p2p_current_state = P2P_STATE_FAILURE;
            workspace->p2p_result = BESL_ERROR_OUT_OF_MEMORY;
            return BESL_ERROR_OUT_OF_MEMORY;
        }
    }

    /* Clear the p2p event queue before bringing up the client */
    p2p_clear_event_queue();
    workspace->group_client_is_up = 1;

    /* Turn off all the other Wi-Fi interfaces */
    wiced_network_down( WWD_STA_INTERFACE );
    wiced_network_down( WWD_AP_INTERFACE );

    /* Query the AP interface to ensure that it is up */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 36, IOVAR_STR_BSSCFG_SSID );
    CHECK_IOCTL_BUFFER( data );
    memset( data, 0, 36 );
    data[ 0 ] = (uint32_t) WWD_AP_INTERFACE;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE );
    wiced_assert("", result == WWD_SUCCESS);

    /* Override MAC address */
    data = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(besl_mac_t), "p2p_da_override" );
    CHECK_IOCTL_BUFFER( data );
    memset( data, 0, sizeof(besl_mac_t) );
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE );
    wiced_assert("", result == WWD_SUCCESS);

    /* Create P2P interface */
    wl_p2p_if_t* p2p_if = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_p2p_if_t), "p2p_ifadd" );
    CHECK_IOCTL_BUFFER( p2p_if );
    p2p_if->interface_type = P2P_CLIENT_MODE;
    p2p_if->chan_spec = workspace->group_candidate.operating_channel.channel | WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE;
    memcpy( &p2p_if->mac_address, &workspace->p2p_interface_address, sizeof(besl_mac_t) );
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE );
    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("p2p_ifadd fail when creating client\r\n"));
    }

    /* Check that we can read the p2p interface. It comes up as a side effect of this call... */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) sizeof(besl_mac_t), "p2p_if" );
    CHECK_IOCTL_BUFFER( data );
    memcpy(data, &workspace->p2p_interface_address, sizeof(besl_mac_t));
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

    wwd_wifi_set_block_ack_window_size( WWD_P2P_INTERFACE );

    if ( workspace->reinvoking_group == 0 )
    {
        /* Start WPS */
        workspace->p2p_wps_agent->device_password_id = workspace->p2p_wps_device_password_id;
        workspace->p2p_wps_agent->wps_mode = workspace->p2p_wps_mode;
        workspace->p2p_wps_agent->is_p2p_enrollee = 1;
        besl_wps_init( workspace->p2p_wps_agent, workspace->wps_device_details, WPS_ENROLLEE_AGENT, workspace->p2p_interface);
        BESL_DEBUG(("past wps init\r\n"));
        wps_internal_init(workspace->p2p_wps_agent, workspace->p2p_interface, workspace->p2p_wps_mode, workspace->p2p_wps_pin, &workspace->wps_credential, 1);
        BESL_DEBUG(("past wps internal init\r\n"));
        wps_register_result_callback( workspace->p2p_wps_agent, workspace->p2p_wps_result_callback );

        /*  Add the P2P IE to the probe request after WPS IE */
        p2p_write_probe_request_ie( workspace );

        /* Add the P2P IE to the association request after the WPS IE */
        p2p_write_association_request_ie( workspace );

        /*  Create the AP details */
        workspace->group_owner.scan_result.channel = workspace->group_candidate.operating_channel.channel;
        workspace->current_channel     = (uint32_t)workspace->group_candidate.operating_channel.channel;
        memcpy( &workspace->operating_channel, &workspace->group_candidate.operating_channel, sizeof( p2p_channel_info_t ) );
        BESL_DEBUG( ("GO operating channel %u\r\n", (unsigned int)workspace->group_candidate.operating_channel.channel) );
        memcpy(&workspace->group_owner.scan_result.BSSID, &workspace->group_candidate.bssid, sizeof(besl_mac_t));
        workspace->group_owner.scan_result.SSID.length = workspace->group_candidate.ssid_length;
        memcpy(workspace->group_owner.scan_result.SSID.value, workspace->group_candidate.ssid, workspace->group_candidate.ssid_length);
        workspace->group_owner.scan_result.security = WICED_SECURITY_WPS_SECURE;
        workspace->group_owner.scan_result.band = WICED_802_11_BAND_2_4GHZ;
        workspace->p2p_wps_agent->directed_wps_max_attempts = 0xFFFFFFFF;
        workspace->p2p_wps_agent->ap = &workspace->group_owner;

        /* Run the WPS state machine in its own thread */
        wps_register_internal_result_callback( workspace->p2p_wps_agent, p2p_wps_internal_result_callback );
        workspace->p2p_current_state = P2P_STATE_CONNECTION_WPS_ENROLLEE;
        besl_p2p_wps_start( workspace->p2p_wps_agent );
        host_rtos_delay_milliseconds( 10 ); // Delay required to allow WPS thread to run and initialize
    }
    else
    {
        workspace->group_owner.scan_result.channel = workspace->group_candidate.operating_channel.channel;
        workspace->current_channel = (uint32_t)workspace->group_candidate.operating_channel.channel;
        memcpy(&workspace->group_owner.scan_result.BSSID, &workspace->group_candidate.bssid, sizeof(besl_mac_t));
        workspace->group_owner.scan_result.SSID.length = workspace->group_candidate.ssid_length;
        memcpy(workspace->group_owner.scan_result.SSID.value, workspace->group_candidate.ssid, workspace->group_candidate.ssid_length);
        workspace->group_owner.scan_result.band = WICED_802_11_BAND_2_4GHZ;
        result = p2p_join_group_owner(  workspace );
    }
    return result;
}

static besl_result_t p2p_join_group_owner( p2p_workspace_t* workspace )
{
    wwd_result_t result;
    int a;

    if ( workspace->reinvoking_group == 0 )
    {
        workspace->p2p_passphrase_length = workspace->wps_credential.passphrase_length;
        memcpy( workspace->p2p_passphrase, workspace->wps_credential.passphrase, workspace->p2p_passphrase_length );
    }

    /* Rewrite the IEs that go into the probe request */
    workspace->p2p_wps_device_password_id = WPS_DEFAULT_DEVICEPWDID;
    p2p_write_wps_probe_request_ie( workspace);
    p2p_write_probe_request_ie( workspace );

    /* Try a few times to join the AP with the credentials we've just received */
    workspace->group_owner.scan_result.security = WICED_SECURITY_WPA2_AES_PSK;
    result = WICED_PENDING;
    BESL_INFO( ("About to join group owner using join specific\r\n") );

    for ( a = 0; a < 3 && result != WWD_SUCCESS; ++a )
    {
        result = wwd_wifi_join_specific( &workspace->group_owner.scan_result, workspace->p2p_passphrase, workspace->p2p_passphrase_length, NULL, workspace->p2p_interface );
    }

    if ( result == WWD_SUCCESS )
    {
        BESL_INFO( ("P2P group formation complete as client device. Group ID %s %02X:%02X:%02X:%02X:%02X:%02X\n", workspace->group_candidate.ssid,
            workspace->group_candidate.bssid.octet[0],
            workspace->group_candidate.bssid.octet[1],
            workspace->group_candidate.bssid.octet[2],
            workspace->group_candidate.bssid.octet[3],
            workspace->group_candidate.bssid.octet[4],
            workspace->group_candidate.bssid.octet[5] ) );

        besl_host_set_mac_address(&workspace->p2p_interface_address, WWD_STA_INTERFACE ); // If this isn't done we don't respond to provision discovery

        if ( wiced_ip_up(workspace->p2p_interface, WICED_USE_EXTERNAL_DHCP_SERVER, NULL) != WICED_SUCCESS )
        {
            BESL_DEBUG( ("IP failed to get an address\r\n") );
            workspace->p2p_result = BESL_P2P_ERROR_FAIL;
            p2p_stop( workspace );
        }
        else
        {
            host_rtos_delay_milliseconds( 10 ); // Delay to allow the DHCP thread to run
            workspace->p2p_current_state = P2P_STATE_GROUP_CLIENT;

            /*  Add P2P event handler */
            result = wwd_management_set_event_handler( p2p_group_client_events, p2p_event_handler, workspace, WICED_P2P_INTERFACE );
            if ( result != WWD_SUCCESS )
            {
                BESL_DEBUG(("Unable to set group client event handler\r\n"));
            }
            workspace->p2p_result = BESL_SUCCESS;
        }
    }
    else
    {
        BESL_DEBUG( ("WPA2 handshake failed when joining GO\r\n") );
        workspace->p2p_result = BESL_P2P_ERROR_FAIL;
        p2p_stop( workspace );
    }

    if ( workspace->p2p_group_formation_result_callback != NULL )
    {
        workspace->p2p_group_formation_result_callback( workspace );
    }

    return workspace->p2p_result;
}


static void p2p_discover( p2p_workspace_t* workspace )
{
    wiced_result_t result;

    REFERENCE_DEBUG_ONLY_VARIABLE(result);

    result = p2p_set_discovery_state( P2P_DISCOVERY_STATE_LISTEN, workspace->listen_channel.channel );
    wiced_assert("", result == WICED_SUCCESS);
}

besl_result_t p2p_host_send_message( p2p_message_t* message, uint32_t timeout_ms )
{
    return host_rtos_push_to_queue( &p2p_message_queue, message, timeout_ms );
}

static besl_result_t p2p_scan( p2p_workspace_t* workspace, uint16_t scan_action, wiced_bool_t all_channels )
{
    wiced_buffer_t buffer;
    wl_p2p_scan_t* p2p_scan;
    besl_mac_t     bcast = {{255, 255, 255, 255, 255, 255}};

    if ( workspace->scan_in_progress == 1 )
    {
        BESL_DEBUG(("p2p_scan: scan already in progress\r\n"));
        //return BESL_SUCCESS;
    }
    workspace->scan_in_progress = 1;

    /*  Begin p2p scan of the "escan" variety */
    p2p_scan = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_p2p_scan_t) + 6, "p2p_scan" );
    CHECK_IOCTL_BUFFER( p2p_scan );
    memset( p2p_scan, 0, sizeof(wl_p2p_scan_t) );

    /* Fill in the appropriate details of the scan parameters structure */
    p2p_scan->type                      = 'E';
    besl_host_random_bytes((uint8_t*)&p2p_scan->escan.sync_id, sizeof(p2p_scan->escan.sync_id));
    p2p_scan->escan.version             = htod32(ESCAN_REQ_VERSION);
    p2p_scan->escan.action              = htod16(WL_SCAN_ACTION_START);
    p2p_scan->escan.params.scan_type    = (int8_t) WICED_SCAN_TYPE_ACTIVE;
    memcpy(&p2p_scan->escan.params.bssid, &bcast, sizeof(besl_mac_t));
    p2p_scan->escan.params.bss_type     = (int8_t) WICED_BSS_TYPE_ANY;
    p2p_scan->escan.params.nprobes      = htod32(2);
    p2p_scan->escan.params.active_time  = htod32(40);
    p2p_scan->escan.params.passive_time = (int32_t) -1;
    p2p_scan->escan.params.home_time    = htod32(60);
    if ( all_channels == WICED_TRUE )
    {
        p2p_scan->escan.params.channel_num  = (int32_t)0;
    }
    else
    {
        p2p_scan->escan.params.channel_num  = (int32_t)3;
        p2p_scan->escan.params.channel_list[0] = 1;
        p2p_scan->escan.params.channel_list[1] = 6;
        p2p_scan->escan.params.channel_list[2] = 11;
    }
    p2p_scan->escan.params.ssid.SSID_len = sizeof( P2P_WILDCARD_SSID ) - 1;
    memcpy( p2p_scan->escan.params.ssid.SSID, P2P_WILDCARD_SSID, sizeof( P2P_WILDCARD_SSID ) - 1 );

    if ( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE ) != WWD_SUCCESS )
    {
        BESL_ERROR(("Failed to start P2P scan\n"));
        return BESL_P2P_ERROR_FAIL;
    }

    return BESL_SUCCESS;
}


static wwd_result_t p2p_set_discovery_state( p2p_discovery_state_t state, uint32_t channel )
{
    uint32_t*      data;
    wiced_buffer_t buffer;
    wl_p2p_disc_st_t discovery_mode;

    discovery_mode.state = state;
    if (state == P2P_DISCOVERY_STATE_LISTEN)
    {
        uint16_t listen_ms;
        besl_host_random_bytes( (uint8_t*)&listen_ms, 2 );
        listen_ms = ( 1 + (listen_ms % P2P_MAX_DISCOVERABLE_INTERVAL ) ) * P2P_BEACON_INTERVAL_MS;

        discovery_mode.chanspec      = channel | WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE;
        discovery_mode.dwell_time_ms = 500;//listen_ms;
    }
    else
    {
        discovery_mode.chanspec      = channel | WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE;
        discovery_mode.dwell_time_ms = 0;
    }

    data = wwd_sdpcm_get_iovar_buffer(&buffer, sizeof(wl_p2p_disc_st_t), "p2p_state");
    CHECK_IOCTL_BUFFER( data );
    memcpy( data, &discovery_mode, sizeof(wl_p2p_disc_st_t) );
    return wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE);
}

static besl_result_t p2p_group_owner_stop( p2p_workspace_t* workspace )
{
    uint32_t*      data;
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wwd_result_t   result;

    wiced_network_down( WICED_P2P_INTERFACE );
    host_rtos_delay_milliseconds( 10 );

    /* Restore original MAC address */
    besl_host_set_mac_address(&workspace->original_mac_address, WICED_STA_INTERFACE );

    /* Query bss state (does it exist? if so is UP?) */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_BSS );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) WWD_P2P_INTERFACE;
    result = wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE );
    if ( ( result != WWD_SUCCESS ) &&
         ( result != WWD_WLAN_NOTFOUND ) )
    {
        BESL_ERROR( ("p2p interface not found\n") );
        return result;
    }

    if ( result == WWD_WLAN_NOTFOUND )
    {
        /* P2P interface does not exist - i.e. it is down */
        return BESL_SUCCESS;
    }
    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    if ( data[0] != (uint32_t) 1 )
    {
        /* P2P interface indicates it is not up - i.e. it is down */
        host_buffer_release( response, WWD_NETWORK_RX );
        BESL_DEBUG( ("p2p interface is not up\n") );
        return BESL_SUCCESS;
    }

    host_buffer_release( response, WWD_NETWORK_RX );

    data = wwd_sdpcm_get_iovar_buffer(&buffer, sizeof(wiced_mac_t), "p2p_ifdel" );
    CHECK_IOCTL_BUFFER( data );
    memcpy( data, &workspace->p2p_interface_address, sizeof(wiced_mac_t) );
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG(("couldn't delete p2p i/f\r\n"));
    }

    return BESL_SUCCESS;

}


static besl_result_t p2p_group_client_stop( p2p_workspace_t* workspace )
{
    uint32_t*      data;
    wiced_buffer_t buffer;
    wwd_result_t   result;

    wiced_network_down( WICED_P2P_INTERFACE );
    host_rtos_delay_milliseconds( 10 );

    /* Restore original MAC address */
    besl_host_set_mac_address(&workspace->original_mac_address, WICED_STA_INTERFACE );

    /* Delete the P2P interface */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, sizeof(wiced_mac_t), "p2p_ifdel" );
    CHECK_IOCTL_BUFFER( data );
    memcpy( data, &workspace->p2p_interface_address, sizeof(wiced_mac_t) );
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    if ( result != WWD_SUCCESS )
    {
        BESL_DEBUG( ("Failed to delete p2p i/f\r\n") );
    }
    else
    {
        BESL_DEBUG( ("p2p_group_client_stop: deleted p2p i/f\r\n") );
    }

    return BESL_SUCCESS;
}


static void p2p_start_timer( void* workspace, uint32_t timeout )
{
    p2p_workspace_t* host = (p2p_workspace_t*)workspace;
    host->timer_reference = host_rtos_get_time( );
    host->timer_timeout   = timeout;
}

void p2p_stop_timer( void* workspace )
{
    p2p_workspace_t* host = (p2p_workspace_t*)workspace;
    host->timer_timeout = 0;
}

static void p2p_abort_scan( p2p_workspace_t* workspace )
{
    if ( workspace->scan_in_progress == 0 )
    {
        BESL_DEBUG(("p2p_abort_scan: scan not in progress\r\n"));
    }
    else
    {
        wwd_wifi_abort_scan();
        workspace->scan_aborted = 1;
    }
}

void p2p_wps_internal_result_callback( wps_result_t* result )
{
    p2p_message_t message;
    wps_result_t  *wps_result = result;

    message.type = P2P_EVENT_WPS_ENROLLEE_COMPLETED;
    message.data = wps_result;
    host_rtos_push_to_queue(&p2p_message_queue, &message, WICED_NEVER_TIMEOUT);
}

besl_result_t p2p_deinit( p2p_workspace_t* workspace )
{
    if ( workspace->p2p_thread_running == 1 )
    {
        BESL_DEBUG(("besl_p2p_deinit: stopping p2p thread\r\n"));
        /* Stop and delete the P2P thread */
        p2p_stop(workspace);
        host_rtos_delay_milliseconds( 10 ); // Delay to allow the P2P thread to complete
    }
    if ( host_rtos_join_thread(&p2p_thread) != WWD_SUCCESS )
    {
        BESL_DEBUG(("besl_p2p_deinit: failed to join thread\r\n"));
    }
    if ( host_rtos_delete_terminated_thread(&p2p_thread) != WWD_SUCCESS )
    {
        BESL_DEBUG(("besl_p2p_deinit: failed to delete thread\r\n"));
    }

    /* Delete the message queue and clean up */
    host_rtos_deinit_queue( &p2p_message_queue );
    memset(p2p_message_queue_buffer, 0, sizeof (host_queue_type_t) * MAX_NUMBER_OF_P2P_MESSAGES );
    memset(&p2p_thread, 0, sizeof(p2p_thread));
    memset(workspace, 0, sizeof(p2p_workspace_t));
    host_rtos_delay_milliseconds( 10 ); // Delay to allow the WPS and P2P threads to complete

    return BESL_SUCCESS;
}

void p2p_clear_event_queue( void )
{
    p2p_message_t message;

    /* Clear the p2p event queue */
    while ( host_rtos_pop_from_queue( &p2p_message_queue, &message, 0 ) == WWD_SUCCESS )
    {
        ;
    }
}
