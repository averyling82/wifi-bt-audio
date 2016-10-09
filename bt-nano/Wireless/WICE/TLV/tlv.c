/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include "BspConfig.h"
#ifdef _DRIVER_WIFI__
#define NOT_INCLUDE_OTHER

#include <string.h>
#include "tlv.h"
#include "wiced_utilities.h"
//#pragma arm section code = "ap6181wifiCode", rodata = "ap6181wifiCode", rwdata = "ap6181wifidata", zidata = "ap6181wifidata"

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
 *               Static Function Declarations
 ******************************************************/

static uint32_t tlv_hton32_ptr(uint8_t* in, uint8_t* out);
static uint16_t tlv_hton16_ptr(uint8_t* in, uint8_t* out);

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

tlv8_data_t* tlv_find_tlv8( const uint8_t* message, uint32_t message_length, uint8_t type )
{
    tlv8_data_t* tlv = (tlv8_data_t*) message;
    while ( ( (uint8_t*) tlv - message ) < message_length - sizeof(tlv8_header_t) )
    {
        if ( tlv->type == type )
        {
            return tlv;
        }
        tlv = (tlv8_data_t*)((uint8_t*)tlv + sizeof(tlv8_header_t) + tlv->length);
    }
    return 0;
}

tlv16_data_t* tlv_find_tlv16( const uint8_t* message, uint32_t message_length, uint16_t type )
{
    tlv16_data_t* tlv = (tlv16_data_t*) message;
    while ( ( (uint8_t*) tlv - message ) < message_length - sizeof(tlv16_header_t) )
    {
        if ( htobe16( WICED_READ_16( &tlv->type ) ) == type )
        {
            return tlv;
        }
        tlv = (tlv16_data_t*)( (uint8_t*)tlv + sizeof(tlv16_header_t) + htobe16( WICED_READ_16( &tlv->length ) ) );
    }
    return 0;
}

tlv_result_t tlv_read_value ( uint16_t type, const uint8_t* message, uint16_t message_length, void* value, uint16_t value_size, tlv_data_type_t data_type )
{
    tlv16_data_t* tlv = tlv_find_tlv16( message, message_length, type );
    if (tlv == 0)
    {
        return TLV_NOT_FOUND;
    }

    switch (data_type)
    {
        case TLV_UINT16:
            tlv_hton16_ptr((uint8_t*) tlv->data, value );
            break;

        case TLV_UINT32:
            tlv_hton32_ptr((uint8_t*) tlv->data, value );
            break;

        default:
            memcpy( value, tlv->data, value_size );
            break;
    }

    return TLV_SUCCESS;
}

uint8_t* tlv_write_value( uint8_t* buffer, uint16_t type, uint16_t length, const const void* data, tlv_data_type_t data_type )
{
    tlv16_data_t* tlv = (tlv16_data_t*) buffer;
    WICED_WRITE_16( &tlv->type,   htobe16(type)   );
    WICED_WRITE_16( &tlv->length, htobe16(length) );
    switch (data_type)
    {
        case TLV_UINT16:
            tlv_hton16_ptr((uint8_t*) data, tlv->data);
            break;

        case TLV_UINT32:
            tlv_hton32_ptr((uint8_t*) data, tlv->data);
            break;

        default:
            memcpy(tlv->data, data, length);
            break;
    }
    return buffer + sizeof(tlv16_header_t) + length;
}

uint8_t* tlv_write_header( uint8_t* buffer, uint16_t type, uint16_t length )
{
    tlv16_header_t* tlv = (tlv16_header_t*)buffer;
    WICED_WRITE_16( &tlv->type,   htobe16(type)   );
    WICED_WRITE_16( &tlv->length, htobe16(length) );
    return buffer + sizeof(tlv16_header_t);
}



static uint32_t tlv_hton32_ptr(uint8_t * in, uint8_t * out)
{
    uint32_t temp;
    temp = WICED_READ_32(in);
    temp = htobe32(temp);
    WICED_WRITE_32(out, temp);
    return temp;
}

static uint16_t tlv_hton16_ptr(uint8_t * in, uint8_t * out)
{
    uint16_t temp;
    temp = WICED_READ_16(in);
    temp = htobe16(temp);
    WICED_WRITE_16(out, temp);
    return temp;
}

//#pragma arm section code
#endif
