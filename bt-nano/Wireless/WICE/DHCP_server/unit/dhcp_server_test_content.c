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

#include "dhcp_server_unit.h"

const dhcp_header_t normal_dhcp_discover_header =
{
        .opcode = BOOTP_OP_REQUEST,
        .hardware_type  = BOOTP_HTYPE_ETHERNET,
        .hardware_addr_len   = 6,

        .client_hardware_addr = { 0x02, 0x0A, 0xF7, 0x5d, 0x6a, 0x5d },
        .transaction_id = 1,


};


const char dhcp_magic[] = { DHCP_MAGIC_COOKIE };
const char dhcp_end[] = { DHCP_OPTION_END };
const char dhcp_discover[] = { DHCP_OPTION_CODE_DHCP_MESSAGE_TYPE, 1, DHCPDISCOVER };
const char dhcp_req_ip[] = { DHCP_OPTION_CODE_REQUEST_IP_ADDRESS, 4, 192, 168, 0, 100 };



const dhcp_option_t normal_dhcp_discover_options[] =
{

        MAKE_DHCP_OPTION( dhcp_magic ),
        MAKE_DHCP_OPTION( dhcp_discover ),
        MAKE_DHCP_OPTION( dhcp_end ),
        { NULL, 0 }
};

const dhcp_option_t dhcp_discover_w_request_options[] =
{

        MAKE_DHCP_OPTION( dhcp_magic ),
        MAKE_DHCP_OPTION( dhcp_discover ),
        MAKE_DHCP_OPTION( dhcp_req_ip ),
        MAKE_DHCP_OPTION( dhcp_end ),
        { NULL, 0 }
};
