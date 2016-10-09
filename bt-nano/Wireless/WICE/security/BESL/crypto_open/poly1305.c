/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/*
 * From https://github.com/floodyberry/poly1305-donna
 * License: "MIT or PUBLIC DOMAIN"
 */

#include <string.h>
#include <stdint.h>
#include "poly1305.h"

#if defined(POLY1305_32BIT)
#include "poly1305-donna-32.h"
#elif defined(POLY1305_64BIT)
#include "poly1305-donna-64.h"
#else

/* auto detect between 32bit / 64bit */
#define HAS_SIZEOF_INT128_64BIT (defined(__SIZEOF_INT128__) && defined(__LP64__))
#define HAS_MSVC_64BIT (defined(_MSC_VER) && defined(_M_X64))
#define HAS_GCC_4_4_64BIT (defined(__GNUC__) && defined(__LP64__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 4))))

#if (HAS_SIZEOF_INT128_64BIT || HAS_MSVC_64BIT || HAS_GCC_4_4_64BIT)
#include "poly1305-donna-64.h"
#else
#include "poly1305-donna-32.h"
#endif

#endif

void
poly1305_update(poly1305_context *context, const unsigned char *message_data, size_t bytes) {
    poly1305_state_internal_t *internal_state = (poly1305_state_internal_t *)context;
    size_t i;

    /* handle leftover */
    if (internal_state->leftover) {
        size_t want = (poly1305_block_size - internal_state->leftover);
        if (want > bytes)
            want = bytes;
        for (i = 0; i < want; i++)
            internal_state->buffer[internal_state->leftover + i] = message_data[i];
        bytes -= want;
        message_data += want;
        internal_state->leftover += want;
        if (internal_state->leftover < poly1305_block_size)
            return;
        poly1305_blocks(internal_state, internal_state->buffer, poly1305_block_size);
        internal_state->leftover = 0;
    }

    /* process full blocks */
    if (bytes >= poly1305_block_size) {
        size_t want = (bytes & ~(poly1305_block_size - 1));
        poly1305_blocks(internal_state, message_data, want);
        message_data += want;
        bytes -= want;
    }

    /* store leftover */
    if (bytes) {
        for (i = 0; i < bytes; i++)
            internal_state->buffer[internal_state->leftover + i] = message_data[i];
        internal_state->leftover += bytes;
    }
}

void
poly1305_auth(unsigned char mac[16], const unsigned char *message_data, size_t bytes, const unsigned char key[32]) {
    poly1305_context context;
    poly1305_init(&context, key);
    poly1305_update(&context, message_data, bytes);
    poly1305_finish(&context, mac);
}

int
poly1305_verify(const unsigned char mac1[16], const unsigned char mac2[16]) {
    size_t i;
    unsigned int dif = 0;
    for (i = 0; i < 16; i++)
        dif |= (mac1[i] ^ mac2[i]);
    dif = (dif - 1) >> ((sizeof(unsigned int) * 8) - 1);
    return (dif & 1);
}


/* test a few basic operations */
int
poly1305_power_on_self_test(void) {
    /* example from nacl */
    static const unsigned char nacl_key[32] = {
        0xee,0xa6,0xa7,0x25,0x1c,0x1e,0x72,0x91,
        0x6d,0x11,0xc2,0xcb,0x21,0x4d,0x3c,0x25,
        0x25,0x39,0x12,0x1d,0x8e,0x23,0x4e,0x65,
        0x2d,0x65,0x1f,0xa4,0xc8,0xcf,0xf8,0x80,
    };

    static const unsigned char nacl_msg[131] = {
        0x8e,0x99,0x3b,0x9f,0x48,0x68,0x12,0x73,
        0xc2,0x96,0x50,0xba,0x32,0xfc,0x76,0xce,
        0x48,0x33,0x2e,0xa7,0x16,0x4d,0x96,0xa4,
        0x47,0x6f,0xb8,0xc5,0x31,0xa1,0x18,0x6a,
        0xc0,0xdf,0xc1,0x7c,0x98,0xdc,0xe8,0x7b,
        0x4d,0xa7,0xf0,0x11,0xec,0x48,0xc9,0x72,
        0x71,0xd2,0xc2,0x0f,0x9b,0x92,0x8f,0xe2,
        0x27,0x0d,0x6f,0xb8,0x63,0xd5,0x17,0x38,
        0xb4,0x8e,0xee,0xe3,0x14,0xa7,0xcc,0x8a,
        0xb9,0x32,0x16,0x45,0x48,0xe5,0x26,0xae,
        0x90,0x22,0x43,0x68,0x51,0x7a,0xcf,0xea,
        0xbd,0x6b,0xb3,0x73,0x2b,0xc0,0xe9,0xda,
        0x99,0x83,0x2b,0x61,0xca,0x01,0xb6,0xde,
        0x56,0x24,0x4a,0x9e,0x88,0xd5,0xf9,0xb3,
        0x79,0x73,0xf6,0x22,0xa4,0x3d,0x14,0xa6,
        0x59,0x9b,0x1f,0x65,0x4c,0xb4,0x5a,0x74,
        0xe3,0x55,0xa5
    };

    static const unsigned char nacl_mac[16] = {
        0xf3,0xff,0xc7,0x70,0x3f,0x94,0x00,0xe5,
        0x2a,0x7d,0xfb,0x4b,0x3d,0x33,0x05,0xd9
    };

    /* generates a final value of (2^130 - 2) == 3 */
    static const unsigned char wrap_key[32] = {
        0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    };

    static const unsigned char wrap_msg[16] = {
        0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
    };

    static const unsigned char wrap_mac[16] = {
        0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    };

    /*
        mac of the macs of messages of length 0 to 256, where the key and messages
        have all their values set to the length
    */
    static const unsigned char total_key[32] = {
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0xff,0xfe,0xfd,0xfc,0xfb,0xfa,0xf9,
        0xff,0xff,0xff,0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xff,0xff
    };

    static const unsigned char total_mac[16] = {
        0x64,0xaf,0xe2,0xe8,0xd6,0xad,0x7b,0xbd,
        0xd2,0x87,0xf9,0x7c,0x44,0x62,0x3d,0x39
    };

    poly1305_context ctx;
    poly1305_context total_ctx;
    unsigned char all_key[32];
    unsigned char all_msg[256];
    unsigned char mac[16];
    size_t i, j;
    int result = 1;

    for (i = 0; i < sizeof(mac); i++)
        mac[i] = 0;
    poly1305_auth(mac, nacl_msg, sizeof(nacl_msg), nacl_key);
    result &= poly1305_verify(nacl_mac, mac);

    for (i = 0; i < sizeof(mac); i++)
        mac[i] = 0;
    poly1305_init(&ctx, nacl_key);
    poly1305_update(&ctx, nacl_msg +   0, 32);
    poly1305_update(&ctx, nacl_msg +  32, 64);
    poly1305_update(&ctx, nacl_msg +  96, 16);
    poly1305_update(&ctx, nacl_msg + 112,  8);
    poly1305_update(&ctx, nacl_msg + 120,  4);
    poly1305_update(&ctx, nacl_msg + 124,  2);
    poly1305_update(&ctx, nacl_msg + 126,  1);
    poly1305_update(&ctx, nacl_msg + 127,  1);
    poly1305_update(&ctx, nacl_msg + 128,  1);
    poly1305_update(&ctx, nacl_msg + 129,  1);
    poly1305_update(&ctx, nacl_msg + 130,  1);
    poly1305_finish(&ctx, mac);
    result &= poly1305_verify(nacl_mac, mac);

    for (i = 0; i < sizeof(mac); i++)
        mac[i] = 0;
    poly1305_auth(mac, wrap_msg, sizeof(wrap_msg), wrap_key);
    result &= poly1305_verify(wrap_mac, mac);

    poly1305_init(&total_ctx, total_key);
    for (i = 0; i < 256; i++) {
        /* set key and message to 'i,i,i..' */
        for (j = 0; j < sizeof(all_key); j++)
            all_key[j] = i;
        for (j = 0; j < i; j++)
            all_msg[j] = i;
        poly1305_auth(mac, all_msg, i, all_key);
        poly1305_update(&total_ctx, mac, 16);
    }
    poly1305_finish(&total_ctx, mac);
    result &= poly1305_verify(total_mac, mac);

    return result;
}



/* these vectors from http://tools.ietf.org/html/draft-agl-tls-chacha20poly1305-04 */

static const uint8_t poly1305_tls_test0_input_data[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t poly1305_tls_test1_input_data[] =
{
    0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21,
};

typedef struct
{
    const uint8_t* input_data;
    uint32_t       input_data_length;
    uint8_t        key[32];
    uint8_t        expected_tag[16];
} poly1305_tls_test_t;

static const poly1305_tls_test_t poly1305_tls_tests[] =
{
    {
        .input_data = poly1305_tls_test0_input_data,
        .input_data_length = sizeof( poly1305_tls_test0_input_data ),
        .key = {
                   0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
                   0x33, 0x32, 0x2d, 0x62, 0x79, 0x74, 0x65, 0x20,
                   0x6b, 0x65, 0x79, 0x20, 0x66, 0x6f, 0x72, 0x20,
                   0x50, 0x6f, 0x6c, 0x79, 0x31, 0x33, 0x30, 0x35,
               },
        .expected_tag = {
                   0x49, 0xec, 0x78, 0x09, 0x0e, 0x48, 0x1e, 0xc6,
                   0xc2, 0x6b, 0x33, 0xb9, 0x1c, 0xcc, 0x03, 0x07,
               },
    },
    {
        .input_data = poly1305_tls_test1_input_data,
        .input_data_length = sizeof( poly1305_tls_test1_input_data ),
        .key = {
                   0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
                   0x33, 0x32, 0x2d, 0x62, 0x79, 0x74, 0x65, 0x20,
                   0x6b, 0x65, 0x79, 0x20, 0x66, 0x6f, 0x72, 0x20,
                   0x50, 0x6f, 0x6c, 0x79, 0x31, 0x33, 0x30, 0x35,
               },
        .expected_tag = {
                   0xa6, 0xf7, 0x45, 0x00, 0x8f, 0x81, 0xc9, 0x16,
                   0xa2, 0x0d, 0xcc, 0x74, 0xee, 0xf2, 0xb2, 0xf0,
               },
    }
};



int test_poly1305_tls( void )
{
    volatile int test_number;
    for( test_number = 0; test_number < sizeof( poly1305_tls_tests ) / sizeof( poly1305_tls_test_t ); test_number++ )
    {
        unsigned char tag[16];
        poly1305_auth( tag, poly1305_tls_tests[test_number].input_data, poly1305_tls_tests[test_number].input_data_length, poly1305_tls_tests[test_number].key );
        if ( 0 != memcmp( poly1305_tls_tests[test_number].expected_tag, tag, sizeof( tag ) ) )
        {
            return -2;
        }
    }
    return 0;
}
