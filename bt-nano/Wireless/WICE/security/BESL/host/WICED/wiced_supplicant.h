/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#include "supplicant_structures.h"
#include "wwd_constants.h"
#include "besl_host.h"

#ifdef __cplusplus
extern "C" {
#endif

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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

besl_result_t besl_supplicant_init                   ( supplicant_workspace_t* workspace, eap_type_t eap_type, wwd_interface_t interface );
besl_result_t besl_supplicant_deinit                 ( supplicant_workspace_t* workspace );
besl_result_t besl_supplicant_start                  ( supplicant_workspace_t* workspace );
besl_result_t besl_supplicant_wait_till_complete     ( supplicant_workspace_t* workspace );
besl_result_t supplicant_host_send_eap_tls_fragments ( supplicant_workspace_t* workspace, uint8_t* buffer, uint16_t length );
besl_result_t supplicant_receive_eap_tls_packet      ( void* workspace_in, tls_packet_t** packet, uint32_t timeout );
void          besl_supplicant_set_identity           ( supplicant_workspace_t* workspace, const char* eap_identity, uint32_t eap_identity_length );
void          supplicant_host_consume_tls_bytes      ( tls_packet_t* packet, int32_t number_of_bytes );
//void          wiced_supplicant_thread_main           ( uint32_t arg );
besl_result_t supplicant_tls_agent_init              ( tls_agent_workspace_t* workspace );
besl_result_t supplicant_tls_agent_deinit            ( tls_agent_workspace_t* workspace );
besl_result_t supplicant_send_eap_tls_packet         ( supplicant_workspace_t* workspace, tls_agent_event_message_t* tls_agent_message, uint32_t timeout );
besl_result_t supplicant_outgoing_pop                ( void* workspace, besl_event_message_t*   outgoing_packet );
besl_result_t supplicant_outgoing_push               ( void* workspace, besl_event_message_t* message );
besl_result_t supplicant_host_get_tls_data           ( besl_packet_t eapol_packet, uint16_t offset, uint8_t** data, uint16_t* fragment_available_data_length, uint16_t *total_available_data_length );
besl_result_t supplicant_tls_agent_finish_connect    ( supplicant_workspace_t* workspace );
besl_result_t supplicant_process_event               ( supplicant_workspace_t* workspace, besl_event_message_t* message);
besl_result_t supplicant_tls_agent_start             ( supplicant_workspace_t* workspace );

#ifdef __cplusplus
} /*extern "C" */
#endif
