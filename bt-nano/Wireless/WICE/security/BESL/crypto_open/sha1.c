/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/* Originally taken from TropicSSL
 * https://gitorious.org/tropicssl/
 * commit: 92bb3462dfbdb4568c92be19e8904129a17b1eed
 * Whitespace converted (Tab to 4 spaces, LF to CRLF)
 * int arguments/returns/variables changed to int32_t
 * add typecasts
 * fix whitespace alignment of macros
 * remove sha1_file
 * add const to test vectors
 * Modify calculations to allow a much smaller version of sha1 without the loop-unrolling
 * Weaken functions to allow smaller version of sha1
 */

/*
 *    FIPS-180-1 compliant SHA-1 implementation
 *
 *    Based on XySSL: Copyright (C) 2006-2008     Christophe Devine
 *
 *    Copyright (C) 2009    Paul Bakker <polarssl_maintainer at polarssl dot org>
 *
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions
 *    are met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the names of PolarSSL or XySSL nor the names of its contributors
 *        may be used to endorse or promote products derived from this software
 *        without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *    The SHA-1 standard was published by NIST in 1993.
 *
 *    http://www.itl.nist.gov/fipspubs/fip180-1.htm
 */

#include "sha1.h"
#include <stdint.h>
#include <string.h>
#include "platform_toolchain.h"

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
    {                                                   \
        (n) = ( (uint32_t) (b)[(i)    ] << 24 )    \
            | ( (uint32_t) (b)[(i) + 1] << 16 )    \
            | ( (uint32_t) (b)[(i) + 2] <<  8 )    \
            | ( (uint32_t) (b)[(i) + 3]       );   \
    }
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                              \
    {                                                    \
        (b)[(i)    ] = (unsigned char) ( (n) >> 24 );    \
        (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );    \
        (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );    \
        (b)[(i) + 3] = (unsigned char) ( (n)       );    \
    }
#endif



/* Rotate an integer value to the left */
#define ROL( input_value, number_of_bits ) ((input_value << number_of_bits) | ((input_value & 0xFFFFFFFF) >> (32 - number_of_bits)))

#define R(t)                                            \
(                                                       \
    temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^     \
           W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],      \
    ( W[t & 0x0F] = ROL(temp,1) )                       \
)

#define P(a,b,c,d,e,x,func,k)                           \
{                                                       \
    e += ROL(a,5) + func(b,c,d) + k + x; b = ROL(b,30); \
}

#define P2(a,b,c,d,e,x, func, k)                        \
{                                                       \
    P(a,b,c,d,e,x, func, k)                             \
    temp = e;                                           \
    e = d;                                              \
    d = c;                                              \
    c = b;                                              \
    b = a;                                              \
    a = temp;                                           \
}

static inline uint32_t sha1_0_20( uint32_t x, uint32_t y, uint32_t z )
{
    return (z ^ (x & (y ^ z)));
}

static inline uint32_t sha1_20_40_and_60_80( uint32_t x, uint32_t y, uint32_t z )
{
    return (x ^ y ^ z);
}

static inline uint32_t sha1_40_60( uint32_t x, uint32_t y, uint32_t z )
{
    return ((x & y) | (z & (x | y)));
}
/*
 * SHA-1 context setup
 */
WEAK void sha1_starts(sha1_context *ctx)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

WEAK void sha1_process(sha1_context *ctx, const unsigned char data[64])
{
    uint32_t temp, W[16], A, B, C, D, E;

    GET_ULONG_BE(W[0], data, 0);
    GET_ULONG_BE(W[1], data, 4);
    GET_ULONG_BE(W[2], data, 8);
    GET_ULONG_BE(W[3], data, 12);
    GET_ULONG_BE(W[4], data, 16);
    GET_ULONG_BE(W[5], data, 20);
    GET_ULONG_BE(W[6], data, 24);
    GET_ULONG_BE(W[7], data, 28);
    GET_ULONG_BE(W[8], data, 32);
    GET_ULONG_BE(W[9], data, 36);
    GET_ULONG_BE(W[10], data, 40);
    GET_ULONG_BE(W[11], data, 44);
    GET_ULONG_BE(W[12], data, 48);
    GET_ULONG_BE(W[13], data, 52);
    GET_ULONG_BE(W[14], data, 56);
    GET_ULONG_BE(W[15], data, 60);



    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];

    P( A, B, C, D, E, W[0] , sha1_0_20,            0x5A827999 );
    P( E, A, B, C, D, W[1] , sha1_0_20,            0x5A827999 );
    P( D, E, A, B, C, W[2] , sha1_0_20,            0x5A827999 );
    P( C, D, E, A, B, W[3] , sha1_0_20,            0x5A827999 );
    P( B, C, D, E, A, W[4] , sha1_0_20,            0x5A827999 );
    P( A, B, C, D, E, W[5] , sha1_0_20,            0x5A827999 );
    P( E, A, B, C, D, W[6] , sha1_0_20,            0x5A827999 );
    P( D, E, A, B, C, W[7] , sha1_0_20,            0x5A827999 );
    P( C, D, E, A, B, W[8] , sha1_0_20,            0x5A827999 );
    P( B, C, D, E, A, W[9] , sha1_0_20,            0x5A827999 );
    P( A, B, C, D, E, W[10], sha1_0_20,            0x5A827999 );
    P( E, A, B, C, D, W[11], sha1_0_20,            0x5A827999 );
    P( D, E, A, B, C, W[12], sha1_0_20,            0x5A827999 );
    P( C, D, E, A, B, W[13], sha1_0_20,            0x5A827999 );
    P( B, C, D, E, A, W[14], sha1_0_20,            0x5A827999 );
    P( A, B, C, D, E, W[15], sha1_0_20,            0x5A827999 );
    P( E, A, B, C, D, R(16), sha1_0_20,            0x5A827999 );
    P( D, E, A, B, C, R(17), sha1_0_20,            0x5A827999 );
    P( C, D, E, A, B, R(18), sha1_0_20,            0x5A827999 );
    P( B, C, D, E, A, R(19), sha1_0_20,            0x5A827999 );
    P( A, B, C, D, E, R(20), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( E, A, B, C, D, R(21), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( D, E, A, B, C, R(22), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( C, D, E, A, B, R(23), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( B, C, D, E, A, R(24), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( A, B, C, D, E, R(25), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( E, A, B, C, D, R(26), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( D, E, A, B, C, R(27), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( C, D, E, A, B, R(28), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( B, C, D, E, A, R(29), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( A, B, C, D, E, R(30), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( E, A, B, C, D, R(31), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( D, E, A, B, C, R(32), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( C, D, E, A, B, R(33), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( B, C, D, E, A, R(34), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( A, B, C, D, E, R(35), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( E, A, B, C, D, R(36), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( D, E, A, B, C, R(37), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( C, D, E, A, B, R(38), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( B, C, D, E, A, R(39), sha1_20_40_and_60_80, 0x6ED9EBA1 );
    P( A, B, C, D, E, R(40), sha1_40_60,           0x8F1BBCDC );
    P( E, A, B, C, D, R(41), sha1_40_60,           0x8F1BBCDC );
    P( D, E, A, B, C, R(42), sha1_40_60,           0x8F1BBCDC );
    P( C, D, E, A, B, R(43), sha1_40_60,           0x8F1BBCDC );
    P( B, C, D, E, A, R(44), sha1_40_60,           0x8F1BBCDC );
    P( A, B, C, D, E, R(45), sha1_40_60,           0x8F1BBCDC );
    P( E, A, B, C, D, R(46), sha1_40_60,           0x8F1BBCDC );
    P( D, E, A, B, C, R(47), sha1_40_60,           0x8F1BBCDC );
    P( C, D, E, A, B, R(48), sha1_40_60,           0x8F1BBCDC );
    P( B, C, D, E, A, R(49), sha1_40_60,           0x8F1BBCDC );
    P( A, B, C, D, E, R(50), sha1_40_60,           0x8F1BBCDC );
    P( E, A, B, C, D, R(51), sha1_40_60,           0x8F1BBCDC );
    P( D, E, A, B, C, R(52), sha1_40_60,           0x8F1BBCDC );
    P( C, D, E, A, B, R(53), sha1_40_60,           0x8F1BBCDC );
    P( B, C, D, E, A, R(54), sha1_40_60,           0x8F1BBCDC );
    P( A, B, C, D, E, R(55), sha1_40_60,           0x8F1BBCDC );
    P( E, A, B, C, D, R(56), sha1_40_60,           0x8F1BBCDC );
    P( D, E, A, B, C, R(57), sha1_40_60,           0x8F1BBCDC );
    P( C, D, E, A, B, R(58), sha1_40_60,           0x8F1BBCDC );
    P( B, C, D, E, A, R(59), sha1_40_60,           0x8F1BBCDC );
    P( A, B, C, D, E, R(60), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( E, A, B, C, D, R(61), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( D, E, A, B, C, R(62), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( C, D, E, A, B, R(63), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( B, C, D, E, A, R(64), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( A, B, C, D, E, R(65), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( E, A, B, C, D, R(66), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( D, E, A, B, C, R(67), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( C, D, E, A, B, R(68), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( B, C, D, E, A, R(69), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( A, B, C, D, E, R(70), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( E, A, B, C, D, R(71), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( D, E, A, B, C, R(72), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( C, D, E, A, B, R(73), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( B, C, D, E, A, R(74), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( A, B, C, D, E, R(75), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( E, A, B, C, D, R(76), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( D, E, A, B, C, R(77), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( C, D, E, A, B, R(78), sha1_20_40_and_60_80, 0xCA62C1D6 );
    P( B, C, D, E, A, R(79), sha1_20_40_and_60_80, 0xCA62C1D6 );

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
}




WEAK void sha1_process_small(sha1_context *ctx, const unsigned char data[64])
{
    uint32_t temp, W[16], A, B, C, D, E;

    GET_ULONG_BE( W[ 0], data,  0 );
    GET_ULONG_BE( W[ 1], data,  4 );
    GET_ULONG_BE( W[ 2], data,  8 );
    GET_ULONG_BE( W[ 3], data, 12 );
    GET_ULONG_BE( W[ 4], data, 16 );
    GET_ULONG_BE( W[ 5], data, 20 );
    GET_ULONG_BE( W[ 6], data, 24 );
    GET_ULONG_BE( W[ 7], data, 28 );
    GET_ULONG_BE( W[ 8], data, 32 );
    GET_ULONG_BE( W[ 9], data, 36 );
    GET_ULONG_BE( W[10], data, 40 );
    GET_ULONG_BE( W[11], data, 44 );
    GET_ULONG_BE( W[12], data, 48 );
    GET_ULONG_BE( W[13], data, 52 );
    GET_ULONG_BE( W[14], data, 56 );
    GET_ULONG_BE( W[15], data, 60 );



    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];

    char round = 0;

    while (round < 80)
    {
        if (round >= 16)
        {
            R( round );
        }
        if (round < 20)
        {
            P2(A,B,C,D,E, W[round & 0x0F],  sha1_0_20,             0x5A827999);
        }
        else if (round < 40)
        {
            P2(A,B,C,D,E, W[round & 0x0F],  sha1_20_40_and_60_80,  0x6ED9EBA1);
        }
        else if (round < 60)
        {
            P2(A,B,C,D,E, W[round & 0x0F],  sha1_40_60,            0x8F1BBCDC);
        }
        else
        {
            P2(A,B,C,D,E, W[round & 0x0F],  sha1_20_40_and_60_80,  0xCA62C1D6);
        }

        ++round;
    }

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
}

/*
 * SHA-1 process buffer
 */
WEAK void sha1_update(sha1_context *ctx, const unsigned char *input, int32_t ilen)
{
    int32_t fill;
    uint32_t left;

    if (ilen <= 0)
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if (ctx->total[0] < (uint32_t)ilen)
        ctx->total[1]++;

    if (left && ilen >= fill) {
        memcpy((void *)(ctx->buffer + left), (const void *)input, fill);
        sha1_process(ctx, ctx->buffer);
        input += fill;
        ilen -= fill;
        left = 0;
    }

    while (ilen >= 64) {
        sha1_process(ctx, input);
        input += 64;
        ilen -= 64;
    }

    if (ilen > 0) {
        memcpy((void *)(ctx->buffer + left), (const void *)input, ilen);
    }
}

static const unsigned char sha1_padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SHA-1 final digest
 */
WEAK void sha1_finish(sha1_context *ctx, unsigned char output[20])
{
    uint32_t last, padn;
    uint32_t high, low;
    unsigned char msglen[8];

    high = (ctx->total[0] >> 29)
        | (ctx->total[1] << 3);
    low = (ctx->total[0] << 3);

    PUT_ULONG_BE(high, msglen, 0);
    PUT_ULONG_BE(low, msglen, 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56) ? (56 - last) : (120 - last);

    sha1_update(ctx, sha1_padding, padn);
    sha1_update(ctx, msglen, 8);

    PUT_ULONG_BE(ctx->state[0], output, 0);
    PUT_ULONG_BE(ctx->state[1], output, 4);
    PUT_ULONG_BE(ctx->state[2], output, 8);
    PUT_ULONG_BE(ctx->state[3], output, 12);
    PUT_ULONG_BE(ctx->state[4], output, 16);
}

/*
 * output = SHA-1( input buffer )
 */
WEAK void sha1(const unsigned char *input, int32_t ilen, unsigned char output[20])
{
    sha1_context ctx;

    sha1_starts(&ctx);
    sha1_update(&ctx, input, ilen);
    sha1_finish(&ctx, output);

    memset(&ctx, 0, sizeof(sha1_context));
}



/*
 * SHA-1 HMAC context setup
 */
void sha1_hmac_starts(sha1_context *ctx, const unsigned char *key, uint32_t keylen)
{
    int32_t i;
    unsigned char sum[20];

    if (keylen > 64) {
        sha1(key, keylen, sum);
        keylen = 20;
        key = sum;
    }

    memset(ctx->ipad, 0x36, 64);
    memset(ctx->opad, 0x5C, 64);

    for (i = 0; i < keylen; i++) {
        ctx->ipad[i] = (unsigned char)(ctx->ipad[i] ^ key[i]);
        ctx->opad[i] = (unsigned char)(ctx->opad[i] ^ key[i]);
    }

    sha1_starts(ctx);
    sha1_update(ctx, ctx->ipad, 64);

    memset(sum, 0, sizeof(sum));
}

/*
 * SHA-1 HMAC process buffer
 */
void sha1_hmac_update(sha1_context *ctx, const unsigned char *input, uint32_t ilen)
{
    sha1_update(ctx, input, ilen);
}

/*
 * SHA-1 HMAC final digest
 */
void sha1_hmac_finish(sha1_context * ctx, unsigned char output[20])
{
    unsigned char tmpbuf[20];

    sha1_finish(ctx, tmpbuf);
    sha1_starts(ctx);
    sha1_update(ctx, ctx->opad, 64);
    sha1_update(ctx, tmpbuf, 20);
    sha1_finish(ctx, output);

    memset(tmpbuf, 0, sizeof(tmpbuf));
}

/*
 * output = HMAC-SHA-1( hmac key, input buffer )
 */
void sha1_hmac(const unsigned char *key, int32_t keylen,
           const unsigned char *input, int32_t ilen,
           unsigned char output[20])
{
    sha1_context ctx;

    sha1_hmac_starts(&ctx, key, keylen);
    sha1_hmac_update(&ctx, input, ilen);
    sha1_hmac_finish(&ctx, output);

    memset(&ctx, 0, sizeof(sha1_context));
}

#if defined(TROPICSSL_SELF_TEST)

#include <stdio.h>

/*
 * FIPS-180-1 test vectors
 */
static const unsigned char sha1_test_buf[3][57] = {
    {"abc"},
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"},
    {""}
};

static const int32_t sha1_test_buflen[3] = {
    3, 56, 1000
};

static const unsigned char sha1_test_sum[3][20] = {
    {
     0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E,
     0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D},
    {
     0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E, 0xBA, 0xAE,
     0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5, 0xE5, 0x46, 0x70, 0xF1},
    {
     0x34, 0xAA, 0x97, 0x3C, 0xD4, 0xC4, 0xDA, 0xA4, 0xF6, 0x1E,
     0xEB, 0x2B, 0xDB, 0xAD, 0x27, 0x31, 0x65, 0x34, 0x01, 0x6F}
};

/*
 * RFC 2202 test vectors
 */
static const unsigned char sha1_hmac_test_key[7][26] = {
    {
     "\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B"
     "\x0B\x0B\x0B\x0B"},
    {"Jefe"},
    {
     "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA"
     "\xAA\xAA\xAA\xAA"},
    {
     "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10"
     "\x11\x12\x13\x14\x15\x16\x17\x18\x19"},
    {
     "\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C"
     "\x0C\x0C\x0C\x0C"},
    {""},            /* 0xAA 80 times */
    {""}
};

static const int32_t sha1_hmac_test_keylen[7] = {
    20, 4, 20, 25, 20, 80, 80
};

static const unsigned char sha1_hmac_test_buf[7][74] = {
    {"Hi There"},
    {"what do ya want for nothing?"},
    {
     "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
     "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
     "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
     "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
     "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"},
    {
     "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
     "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
     "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
     "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
     "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"},
    {"Test With Truncation"},
    {"Test Using Larger Than Block-Size Key - Hash Key First"},
    {
     "Test Using Larger Than Block-Size Key and Larger"
     " Than One Block-Size Data"}
};

static const int32_t sha1_hmac_test_buflen[7] = {
    8, 28, 50, 50, 20, 54, 73
};

static const unsigned char sha1_hmac_test_sum[7][20] = {
    {
     0xB6, 0x17, 0x31, 0x86, 0x55, 0x05, 0x72, 0x64, 0xE2, 0x8B,
     0xC0, 0xB6, 0xFB, 0x37, 0x8C, 0x8E, 0xF1, 0x46, 0xBE, 0x00},
    {
     0xEF, 0xFC, 0xDF, 0x6A, 0xE5, 0xEB, 0x2F, 0xA2, 0xD2, 0x74,
     0x16, 0xD5, 0xF1, 0x84, 0xDF, 0x9C, 0x25, 0x9A, 0x7C, 0x79},
    {
     0x12, 0x5D, 0x73, 0x42, 0xB9, 0xAC, 0x11, 0xCD, 0x91, 0xA3,
     0x9A, 0xF4, 0x8A, 0xA1, 0x7B, 0x4F, 0x63, 0xF1, 0x75, 0xD3},
    {
     0x4C, 0x90, 0x07, 0xF4, 0x02, 0x62, 0x50, 0xC6, 0xBC, 0x84,
     0x14, 0xF9, 0xBF, 0x50, 0xC8, 0x6C, 0x2D, 0x72, 0x35, 0xDA},
    {
     0x4C, 0x1A, 0x03, 0x42, 0x4B, 0x55, 0xE0, 0x7F, 0xE7, 0xF2,
     0x7B, 0xE1},
    {
     0xAA, 0x4A, 0xE5, 0xE1, 0x52, 0x72, 0xD0, 0x0E, 0x95, 0x70,
     0x56, 0x37, 0xCE, 0x8A, 0x3B, 0x55, 0xED, 0x40, 0x21, 0x12},
    {
     0xE8, 0xE9, 0x9D, 0x0F, 0x45, 0x23, 0x7D, 0x78, 0x6D, 0x6B,
     0xBA, 0xA7, 0x96, 0x5C, 0x78, 0x08, 0xBB, 0xFF, 0x1A, 0x91}
};

/*
 * Checkup routine
 */
int32_t sha1_self_test(int32_t verbose)
{
    int32_t i, j, buflen;
    unsigned char buf[1024];
    unsigned char sha1sum[20];
    sha1_context ctx;

    /*
     * SHA-1
     */
    for (i = 0; i < 3; i++) {
        if (verbose != 0)
            printf("  SHA-1 test #%ld: ", (long int)( i + 1 ));

        sha1_starts(&ctx);

        if (i == 2) {
            memset(buf, 'a', buflen = 1000);

            for (j = 0; j < 1000; j++)
                sha1_update(&ctx, buf, buflen);
        } else
            sha1_update(&ctx, sha1_test_buf[i],
                    sha1_test_buflen[i]);

        sha1_finish(&ctx, sha1sum);

        if (memcmp(sha1sum, sha1_test_sum[i], 20) != 0) {
            if (verbose != 0)
                printf("failed\n");

            return (1);
        }

        if (verbose != 0)
            printf("passed\n");
    }

    if (verbose != 0)
        printf("\n");

    for (i = 0; i < 7; i++) {
        if (verbose != 0)
            printf("  HMAC-SHA-1 test #%ld: ", (long int)( i + 1 ));

        if (i == 5 || i == 6) {
            memset(buf, '\xAA', buflen = 80);
            sha1_hmac_starts(&ctx, buf, buflen);
        } else
            sha1_hmac_starts(&ctx, sha1_hmac_test_key[i],
                     sha1_hmac_test_keylen[i]);

        sha1_hmac_update(&ctx, sha1_hmac_test_buf[i],
                 sha1_hmac_test_buflen[i]);

        sha1_hmac_finish(&ctx, sha1sum);

        buflen = (i == 4) ? 12 : 20;

        if (memcmp(sha1sum, sha1_hmac_test_sum[i], buflen) != 0) {
            if (verbose != 0)
                printf("failed\n");

            return (1);
        }

        if (verbose != 0)
            printf("passed\n");
    }

    if (verbose != 0)
        printf("\n");

    return (0);
}

#endif
