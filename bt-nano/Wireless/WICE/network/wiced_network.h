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

#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "../security/BESL/host/WICED/tls_types.h"
#include "../lwip-1.4.1/src/include/ipv4/lwip/ip_addr.h"
#include "../Lwip/lwip-1.4.1/src/include/lwip/netif.h"
#include "wiced_result.h"
#include "wiced_wifi.h"
#include "../WWD/include/network/wwd_network_constants.h"
#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#define LWIP_TO_WICED_ERR( lwip_err )  ((lwip_err >= ERR_ISCONN)? lwip_to_wiced_result[ -lwip_err ] : WICED_UNKNOWN_NETWORK_STACK_ERROR )

#define IP_HANDLE(interface)   (*wiced_ip_handle[(interface)&3])

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_TCP_PAYLOAD_SIZE    ( WICED_PAYLOAD_MTU - TCP_HLEN - IP_HLEN - WICED_PHYSICAL_HEADER )
#define MAX_UDP_PAYLOAD_SIZE    ( WICED_PAYLOAD_MTU - UDP_HLEN - IP_HLEN - WICED_PHYSICAL_HEADER )
#define MAX_IP_PAYLOAD_SIZE     ( WICED_PAYLOAD_MTU - IP_HLEN - WICED_PHYSICAL_HEADER )


#define IP_STACK_SIZE               (4*1024)
#define DHCP_STACK_SIZE             (1280)

#define WICED_ANY_PORT              (0)

#define wiced_packet_pools          (NULL)

#define WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS    (5)
#define WICED_MAXIMUM_NUMBER_OF_ACCEPT_SOCKETS            (MEMP_NUM_NETCONN)

#define WICED_LINK_CHECK( interface )      do { if ( netif_is_up( &IP_HANDLE(interface) ) != 1){ return WICED_NOTUP; }} while(0)
#define WICED_LINK_CHECK_TCP_SOCKET( socket_in )   WICED_LINK_CHECK((socket_in)->interface)
#define WICED_LINK_CHECK_UDP_SOCKET( socket_in )   WICED_LINK_CHECK((socket_in)->interface)

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    WICED_TCP_STANDARD_SOCKET,
    WICED_TCP_SECURE_SOCKET,
} wiced_tcp_socket_type_t;

typedef enum
{
    WICED_SOCKET_CLOSED,
    WICED_SOCKET_CLOSING,
    WICED_SOCKET_CONNECTING,
    WICED_SOCKET_CONNECTED,
    WICED_SOCKET_DATA_PENDING,
    WICED_SOCKET_ERROR
} wiced_socket_state_t;

typedef enum
{
    WICED_TCP_DISCONNECT_CALLBACK_INDEX = 0,
    WICED_TCP_RECEIVE_CALLBACK_INDEX    = 1,
    WICED_TCP_CONNECT_CALLBACK_INDEX    = 2,
} wiced_tcp_callback_index_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct netbuf wiced_packet_t;

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_tcp_socket_type_t     type;
    int                         socket;
    struct netconn*             conn_handler;
    struct netconn*             accept_handler;
    ip_addr_t                   local_ip_addr;
    wiced_bool_t                is_bound;
    int                         interface;
    wiced_tls_simple_context_t* tls_context;
    wiced_bool_t                context_malloced;
    uint32_t                    callbacks[3];
    void*                       arg;
} wiced_tcp_socket_t;

typedef struct
{
    wiced_tcp_socket_t   listen_socket;
    wiced_tcp_socket_t   accept_socket       [WICED_MAXIMUM_NUMBER_OF_ACCEPT_SOCKETS];
    wiced_socket_state_t accept_socket_state [WICED_MAXIMUM_NUMBER_OF_ACCEPT_SOCKETS];
    int                  data_pending_on_socket;
    uint16_t             port;
} wiced_tcp_server_t;

typedef struct
{
    int             socket;
    struct netconn* conn_handler;
    struct netconn* accept_handler;
    ip_addr_t       local_ip_addr;
    wiced_bool_t    is_bound;
    int             interface;
    uint32_t        receive_callback;
    void*           arg;
} wiced_udp_socket_t;

typedef struct
{
    wiced_tcp_socket_type_t  type;
    int                      socket;
    wiced_tls_context_t      context;
    wiced_tls_session_t      session;
    wiced_tls_certificate_t* certificate;
    wiced_tls_key_t*         key;
} wiced_tls_socket_t;

typedef wiced_result_t (*wiced_tcp_socket_callback_t)( wiced_tcp_socket_t* socket, void* arg );
typedef wiced_result_t (*wiced_udp_socket_callback_t)( wiced_udp_socket_t* socket, void* arg );

/******************************************************
 *                 Global Variables
 ******************************************************/

/* Note: These objects are for internal use only! */
extern xTaskHandle     wiced_thread_handle;
extern struct netif*   wiced_ip_handle[3];
extern struct dhcp     wiced_dhcp_handle;

extern wiced_result_t lwip_to_wiced_result[];

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif
