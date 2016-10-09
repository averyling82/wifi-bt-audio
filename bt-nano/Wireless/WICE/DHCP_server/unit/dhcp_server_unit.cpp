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
 *  Unit Tester for DHCP server
 *
 *  Runs a suite of tests on the DHCP server to attempt
 *  to discover bugs
 */

#include <stdio.h>
#include <string.h>
#include "gtest/gtest.h"
#include "dhcp_server.h"
#include "mock.h"
#include "wiced_management.h"
#include "wiced_tcpip.h"
#include "dhcp_server_unit.h"

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                    Macros
 ******************************************************/

#define RETCHK_START( retcheck_var_in )  \
        { \
            int count = 0; \
            int* retcheck_var = &(retcheck_var_in); \
            do \
            { \
                *retcheck_var = count; \
                count++;

#define RETCHK_ACTIVATED()  (*retcheck_var == -1)

#define RETCHK_EXPECT_EQ( expected, val )  if ( RETCHK_ACTIVATED( ) ) { EXPECT_EQ( (expected), (val) ); }
#define RETCHK_EXPECT_NE( expected, val )  if ( RETCHK_ACTIVATED( ) ) { EXPECT_NE( (expected), (val) ); }
#define RETCHK_EXPECT_GT( expected, val )  if ( RETCHK_ACTIVATED( ) ) { EXPECT_GT( (expected), (val) ); }

#define RETCHK_COUNT( ) count

#define RETCHK_END( ) \
            } while ( *retcheck_var == -1 ); \
            *retcheck_var = -1; \
        }


//typedef void (*send_handler_t)( wiced_packet_t* packet );


/******************************************************
 *                    Global Variables
 ******************************************************/

/******************************************************
 *                    Static Variables
 ******************************************************/
static wiced_packet_t* udp_receive_packet = NULL;

typedef struct
{
        wiced_udp_socket_t* socket;
        const wiced_ip_address_t* address;
        uint16_t port;
        wiced_packet_t* packet;
} udp_send_details_t;


static udp_send_details_t udp_send_packet = { NULL, NULL, 0, NULL };
static wiced_result_t  udp_receive_error_val = WICED_SUCCESS;
//send_handler_t send_handler = NULL;

static const wiced_ip_address_t ipv4_broadcast = { WICED_IPV4, { MAKE_IPV4_ADDRESS( 255, 255, 255, 255 ) }};
static const uint32_t local_ip =  MAKE_IPV4_ADDRESS( 192, 168,   0, 1 );
static const wiced_ip_setting_t ip_settings =  { { WICED_IPV4, { local_ip } },
                                                 { WICED_IPV4, { MAKE_IPV4_ADDRESS( 192, 168,   0,   1 ) } },
                                                 { WICED_IPV4, { MAKE_IPV4_ADDRESS( 255, 255, 255,   0 ) } } };

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static wiced_result_t unit_test_dhcp_wiced_udp_receive( wiced_udp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout );
static wiced_result_t unit_test_dhcp_wiced_udp_send( wiced_udp_socket_t* socket, const wiced_ip_address_t* address, uint16_t port, wiced_packet_t* packet );
static void validate_DHCP_Offer( void* data, const dhcp_header_t* discover_header, uint32_t buf_size );
static void validate_DHCP_ack( void* data, const dhcp_header_t* request_header, uint32_t buf_size );
static void validate_DHCP_nak( void* data, const dhcp_header_t* request_header, uint32_t buf_size );
static uint8_t* find_option( dhcp_header_t* request, uint8_t option_num, uint32_t buf_size );

/******************************************************
 *               Function Definitions
 ******************************************************/

/******************************************************
 *            wicedfs_init tests
 ******************************************************/

/* normal - each interface */
TEST(unit_test_dhcp_server_start, normal )
{
    wiced_result_t result;
    wiced_dhcp_server_t server;

    wiced_init( );

    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_STATIC_IP, &ip_settings );

    result = wiced_start_dhcp_server( &server, WICED_STA_INTERFACE );
    EXPECT_EQ( WICED_SUCCESS, result );

    result = wiced_stop_dhcp_server( &server);
    EXPECT_EQ( WICED_SUCCESS, result );

    wiced_network_down( WICED_STA_INTERFACE );

#if 0  // Simulator not working with AP yet

    wiced_network_up( WICED_AP_INTERFACE, WICED_USE_STATIC_IP, &ip_settings );

    result = wiced_start_dhcp_server( &server, WICED_AP_INTERFACE );
    EXPECT_EQ( WICED_SUCCESS, result );

    result = wiced_stop_dhcp_server( &server);
    EXPECT_EQ( WICED_SUCCESS, result );

    wiced_network_down( WICED_AP_INTERFACE );

    wiced_network_up( WICED_CONFIG_INTERFACE, WICED_USE_STATIC_IP, &ip_settings );

    result = wiced_start_dhcp_server( &server, WICED_CONFIG_INTERFACE );
    EXPECT_EQ( WICED_SUCCESS, result );

    result = wiced_stop_dhcp_server( &server);
    EXPECT_EQ( WICED_SUCCESS, result );


    wiced_network_down( WICED_CONFIG_INTERFACE );

#endif

    wiced_deinit( );
}


/* double init on same interface */
TEST(unit_test_dhcp_server_start, double_init )
{
    wiced_result_t result;
    wiced_dhcp_server_t server;

    wiced_init( );

    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_STATIC_IP, &ip_settings );

    result = wiced_start_dhcp_server( &server, WICED_STA_INTERFACE );
    EXPECT_EQ( WICED_SUCCESS, result );

    result = wiced_start_dhcp_server( &server, WICED_STA_INTERFACE );
    EXPECT_EQ( WICED_ADDRESS_IN_USE, result );


    result = wiced_stop_dhcp_server( &server);
    EXPECT_EQ( WICED_SUCCESS, result );

    wiced_network_down( WICED_STA_INTERFACE );

    wiced_deinit( );
}


/* when interface is not up */
TEST(unit_test_dhcp_server_start, interface_down )
{
    wiced_result_t result;
    wiced_dhcp_server_t server;

    wiced_init( );
    result = wiced_start_dhcp_server( &server, WICED_STA_INTERFACE );
    EXPECT_EQ( WICED_NOTUP, result );

    wiced_deinit( );
}

/* Null handle */
TEST(unit_test_dhcp_server_start, null_server_handle)
{
    wiced_result_t result;

    result = wiced_start_dhcp_server( NULL, WICED_STA_INTERFACE );

    EXPECT_EQ( WICED_BADARG, result );
}

/* bad interface id */
TEST(unit_test_dhcp_server_start, bad_interface_id )
{
    wiced_result_t result;
    wiced_dhcp_server_t server;

    result = wiced_start_dhcp_server( &server, (wiced_interface_t)( WICED_CONFIG_INTERFACE + 100 ) );

    EXPECT_EQ( WICED_BADARG, result );
}

/* Null handle */
TEST(unit_test_dhcp_server_stop, null_server_handle )
{
    wiced_result_t result;

    result = wiced_stop_dhcp_server( NULL );

    EXPECT_EQ( WICED_BADARG, result );
}




wiced_semaphore_t packet_sent_sem;
wiced_semaphore_t packet_available_sem;

class unit_test_dhcp_server :  public ::testing::Test
{

    protected:


    wiced_dhcp_server_t server;

    virtual void SetUp()
    {

        udp_receive_packet = NULL;
        udp_send_packet.address    = NULL;
        udp_send_packet.packet    = NULL;
        udp_send_packet.socket    = NULL;
        udp_send_packet.port    = 0;
//        send_handler = NULL;
        udp_receive_error_val = WICED_SUCCESS;
        wiced_rtos_init_semaphore( &packet_sent_sem );
        wiced_rtos_init_semaphore( &packet_available_sem );
        set_mock_function( wiced_udp_receive, unit_test_dhcp_wiced_udp_receive );
        set_mock_function( wiced_udp_send,    unit_test_dhcp_wiced_udp_send );

        wiced_init( );
        wiced_network_up( WICED_STA_INTERFACE, WICED_USE_STATIC_IP, &ip_settings );
        wiced_start_dhcp_server( &server, WICED_STA_INTERFACE );
    }
    virtual void TearDown()
    {
        wiced_stop_dhcp_server( &server );
        wiced_network_down( WICED_STA_INTERFACE );
        wiced_deinit( );

        reset_mock_function( wiced_udp_send );
        reset_mock_function( wiced_udp_receive );

        wiced_rtos_deinit_semaphore( &packet_available_sem );
        wiced_rtos_deinit_semaphore( &packet_sent_sem );
    }
};


/* Normal DHCP discover request */
TEST_F(unit_test_dhcp_server, normal_discover )
{
    wiced_result_t result;

    uint8_t* data_pointer;
    uint16_t available_space;
    int pos = 0;
    uint8_t* data;
    uint16_t data_length;
    uint16_t available_data_length;

    /* Get packet */
    result = wiced_packet_create( sizeof(normal_dhcp_discover_header), &udp_receive_packet, &data_pointer, &available_space );
    EXPECT_EQ( WICED_SUCCESS, result );

    /* Fill packet with DHCP Discover data */
    memcpy( data_pointer, &normal_dhcp_discover_header, sizeof(normal_dhcp_discover_header) );
    pos += sizeof(normal_dhcp_discover_header);
    for ( const dhcp_option_t* curr_option = normal_dhcp_discover_options; curr_option->bytes != NULL; curr_option++ )
    {
        memcpy( &data_pointer[pos], curr_option->bytes, curr_option->size );
        pos += curr_option->size;
    }

    /* Signal DHCP server that there is a packet available */
    udp_receive_error_val = WICED_SUCCESS;
    wiced_rtos_set_semaphore( &packet_available_sem );


    /* Wait till the DHCP server replies with Offer */
    result = wiced_rtos_get_semaphore( &packet_sent_sem, 100 );
    EXPECT_EQ( WICED_SUCCESS, result );

    /* Get the data from the offer packet */
    result = wiced_packet_get_data( udp_send_packet.packet, 0, &data, &data_length, &available_data_length );
    EXPECT_EQ( WICED_SUCCESS, result );

    /* Check that the DHCP server sent the offer to the correct address/port */
    EXPECT_EQ( 0, memcmp( &ipv4_broadcast, udp_send_packet.address, sizeof(wiced_ip_address_t) ) );
    EXPECT_EQ( 68, udp_send_packet.port );

    /* Check that the offer is valid */
    validate_DHCP_Offer( data, &normal_dhcp_discover_header, data_length );

    /* Delete the packet */
    result = wiced_packet_delete( udp_send_packet.packet );

    EXPECT_EQ( WICED_SUCCESS, result );
}


/* DHCP discover with requested address */
TEST_F(unit_test_dhcp_server, discover_w_request )
{
    wiced_result_t result;

    uint8_t* data_pointer;
    uint16_t available_space;
    int pos = 0;
    uint8_t* data;
    uint16_t data_length;
    uint16_t available_data_length;
    uint8_t* option_data;


    /* Get packet */
    result = wiced_packet_create( sizeof(normal_dhcp_discover_header), &udp_receive_packet, &data_pointer, &available_space );
    EXPECT_EQ( WICED_SUCCESS, result );

    /* Fill packet with DHCP Discover data */
    memcpy( data_pointer, &normal_dhcp_discover_header, sizeof(normal_dhcp_discover_header) );
    pos += sizeof(normal_dhcp_discover_header);
    for ( const dhcp_option_t* curr_option = dhcp_discover_w_request_options; curr_option->bytes != NULL; curr_option++ )
    {
        memcpy( &data_pointer[pos], curr_option->bytes, curr_option->size );
        pos += curr_option->size;
    }

    /* Signal DHCP server that there is a packet available */
    udp_receive_error_val = WICED_SUCCESS;
    wiced_rtos_set_semaphore( &packet_available_sem );


    /* Wait till the DHCP server replies with Offer */
    result = wiced_rtos_get_semaphore( &packet_sent_sem, 100 );
    EXPECT_EQ( WICED_SUCCESS, result );

    /* Get the data from the offer packet */
    result = wiced_packet_get_data( udp_send_packet.packet, 0, &data, &data_length, &available_data_length );
    EXPECT_EQ( WICED_SUCCESS, result );

    /* Check that the DHCP server sent the offer to the correct address/port */
    EXPECT_EQ( 0, memcmp( &ipv4_broadcast, udp_send_packet.address, sizeof(wiced_ip_address_t) ) );
    EXPECT_EQ( 68, udp_send_packet.port );

    /* Server is allowed to respond with Offer, Ack or Nak */
    option_data = find_option( (dhcp_header_t*)data, DHCP_OPTION_CODE_DHCP_MESSAGE_TYPE, data_length );
    EXPECT_NE( (uint8_t*)NULL, option_data );  /* DHCP_OFFER must contain DHCP Message Type Option */
    if ( ( option_data != NULL ) && ( option_data[0] == DHCPOFFER ) )
    {
        /* Check that the offer is valid */
        validate_DHCP_Offer( data, &normal_dhcp_discover_header, data_length );
    }
    else if ( ( option_data != NULL ) && ( option_data[0] == DHCPACK ) )
    {
        /* Check that the ACK is valid */
        validate_DHCP_ack( data, &normal_dhcp_discover_header, data_length );
    }
    else if ( ( option_data != NULL ) && ( option_data[0] == DHCPNAK ) )
    {
        /* Check that the NAK is valid */
        validate_DHCP_nak( data, &normal_dhcp_discover_header, data_length );
    }
    else
    {
        ADD_FAILURE(); /* Disallowed response type */
    }


    /* Delete the packet */
    result = wiced_packet_delete( udp_send_packet.packet );

    EXPECT_EQ( WICED_SUCCESS, result );
}















static void validate_DHCP_Offer( void* data, const dhcp_header_t* request_header, uint32_t buf_size )
{

    dhcp_header_t* dhcp_header;
    dhcp_header = (dhcp_header_t*) data;
    EXPECT_EQ( BOOTP_HTYPE_ETHERNET, dhcp_header->hardware_type );
    EXPECT_EQ( 6, dhcp_header->hardware_addr_len );
    EXPECT_EQ( BOOTP_OP_REPLY, dhcp_header->opcode );
    EXPECT_EQ( request_header->transaction_id, dhcp_header->transaction_id );
    EXPECT_NE( 0, dhcp_header->your_ip_addr[0] );
    EXPECT_EQ( 0, memcmp( request_header->client_hardware_addr, dhcp_header->client_hardware_addr, 6 ) );

    uint8_t* option_data;

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_REQUEST_IP_ADDRESS, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_OFFER must not contain Requested IP Address Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_LEASE_TIME, buf_size );
    EXPECT_NE( (uint8_t*)NULL, option_data );  /* DHCP_OFFER must contain Lease Time Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_DHCP_MESSAGE_TYPE, buf_size );
    EXPECT_NE( (uint8_t*)NULL, option_data );  /* DHCP_OFFER must contain DHCP Message Type Option */
    if ( option_data )
    {
        EXPECT_EQ( DHCPOFFER, *option_data ); /* DHCP Message Type must be DHCP_OFFER */
    }

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_PARAM_REQ_LIST, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_OFFER must not contain Parameter Request List Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_CLIENT_ID, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_OFFER must not contain Client Identifier Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_SERVER_ID, buf_size );
    EXPECT_NE( (uint8_t*)NULL, option_data );  /* DHCP_OFFER must contain Server Identifier Option */
    if ( option_data )
    {
        EXPECT_NE( 0, option_data[0] );   /* First octet of server address must be non-zero */
    }

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_MAX_MSG_SIZE, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_OFFER must not contain Maximum DHCP message size Option */
}


static void validate_DHCP_ack( void* data, const dhcp_header_t* request_header, uint32_t buf_size )
{

    dhcp_header_t* dhcp_header;
    dhcp_header = (dhcp_header_t*) data;
    EXPECT_EQ( BOOTP_HTYPE_ETHERNET, dhcp_header->hardware_type );
    EXPECT_EQ( 6, dhcp_header->hardware_addr_len );
    EXPECT_EQ( BOOTP_OP_REPLY, dhcp_header->opcode );
    EXPECT_EQ( request_header->transaction_id, dhcp_header->transaction_id );
    EXPECT_NE( 0, dhcp_header->your_ip_addr[0] );
    EXPECT_EQ( 0, memcmp( request_header->client_hardware_addr, dhcp_header->client_hardware_addr, 6 ) );

    uint8_t* option_data;

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_REQUEST_IP_ADDRESS, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_ACK must not contain Requested IP Address Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_LEASE_TIME, buf_size );
    EXPECT_NE( (uint8_t*)NULL, option_data );  /* DHCP_ACL must contain Lease Time Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_DHCP_MESSAGE_TYPE, buf_size );
    EXPECT_NE( (uint8_t*)NULL, option_data );  /* DHCP_ACK must contain DHCP Message Type Option */
    if ( option_data )
    {
        EXPECT_EQ( DHCPACK, *option_data ); /* DHCP Message Type must be DHCP_ACK */
    }

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_PARAM_REQ_LIST, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_ACK must not contain Parameter Request List Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_CLIENT_ID, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_ACK must not contain Client Identifier Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_SERVER_ID, buf_size );
    EXPECT_NE( (uint8_t*)NULL, option_data );  /* DHCP_ACK must contain Server Identifier Option */
    if ( option_data )
    {
        EXPECT_NE( 0, option_data[0] );   /* First octet of server address must be non-zero */
    }

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_MAX_MSG_SIZE, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_aCK must not contain Maximum DHCP message size Option */
}


int mem_zero_cmp( const void* addr, int len )
{
    while( len )
    {
        if ( *( (char*) addr ) != 0 )
        {
            return -1;
        }
    }
    return 0;
}


static void validate_DHCP_nak( void* data, const dhcp_header_t* request_header, uint32_t buf_size )
{

    dhcp_header_t* dhcp_header;
    dhcp_header = (dhcp_header_t*) data;
    uint8_t* option_data;

    EXPECT_EQ( BOOTP_HTYPE_ETHERNET, dhcp_header->hardware_type );
    EXPECT_EQ( 6, dhcp_header->hardware_addr_len );
    EXPECT_EQ( BOOTP_OP_REPLY, dhcp_header->opcode );
    EXPECT_EQ( request_header->transaction_id, dhcp_header->transaction_id );
    EXPECT_NE( 0, dhcp_header->your_ip_addr[0] );
    EXPECT_EQ( 0, memcmp( request_header->client_hardware_addr, dhcp_header->client_hardware_addr, 6 ) );

    EXPECT_EQ( 0, mem_zero_cmp( request_header->legacy, sizeof(request_header->legacy) ) ); /* file and sname fields must be zero for NAK */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_REQUEST_IP_ADDRESS, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_NAK must not contain Requested IP Address Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_LEASE_TIME, buf_size );
    EXPECT_EQ( (uint8_t*)NULL, option_data );  /* DHCP_NAK must not contain Lease Time Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_LEASE_TIME, buf_size );
    EXPECT_EQ( (uint8_t*)NULL, option_data );  /* DHCP_NAK must notcontain Lease Time Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_DHCP_MESSAGE_TYPE, buf_size );
    EXPECT_NE( (uint8_t*)NULL, option_data );  /* DHCP_NAK must contain DHCP Message Type Option */
    if ( option_data )
    {
        EXPECT_EQ( DHCPNAK, *option_data ); /* DHCP Message Type must be DHCP_NAK */
    }

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_PARAM_REQ_LIST, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_NAK must not contain Parameter Request List Option */

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_SERVER_ID, buf_size );
    EXPECT_NE( (uint8_t*)NULL, option_data );  /* DHCP_NAK must contain Server Identifier Option */
    if ( option_data )
    {
        EXPECT_NE( 0, option_data[0] );   /* First octet of server address must be non-zero */
    }

    option_data = find_option( dhcp_header, DHCP_OPTION_CODE_MAX_MSG_SIZE, buf_size );
    EXPECT_EQ( NULL, option_data );  /* DHCP_NAK must not contain Maximum DHCP message size Option */


    EXPECT_EQ( 0, 1 );  /* DHCP_NAK must not contain any other options */
}




static wiced_result_t unit_test_dhcp_wiced_udp_receive( wiced_udp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    wiced_result_t temp_result;
    wiced_result_t result;

    result = wiced_rtos_get_semaphore( &packet_available_sem, timeout );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    temp_result = udp_receive_error_val;
    udp_receive_error_val = WICED_SUCCESS;

    *packet = udp_receive_packet;
    udp_receive_packet = NULL;

    return temp_result;
}



static wiced_result_t unit_test_dhcp_wiced_udp_send( wiced_udp_socket_t* socket, const wiced_ip_address_t* address, uint16_t port, wiced_packet_t* packet )
{
    udp_send_packet.socket = socket;
    udp_send_packet.address = address;
    udp_send_packet.port = port;
    udp_send_packet.packet = packet;
    wiced_rtos_set_semaphore( &packet_sent_sem );

    return WICED_SUCCESS;
}


static uint8_t* find_option( dhcp_header_t* request, uint8_t option_num, uint32_t buf_size )
{
    uint8_t* option_ptr = ((uint8_t*) &request[1]) + 4;
    while ( ( option_ptr[0] != 0xff ) &&
            ( option_ptr[0] != option_num ) &&
            ( option_ptr < ( (unsigned char*) request ) + buf_size ) )
    {
        option_ptr += option_ptr[1] + 2;
    }
    if ( option_ptr[0] == option_num )
    {
        return &option_ptr[2];
    }
    return NULL;

}
