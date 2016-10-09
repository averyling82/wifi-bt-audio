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
 * int arguments/returns changed to int32_t
 * key length changed to measure in bits
 * Make arc4_crypt work on data in-place to match other ciphers
 */

/**
 *    An implementation of the ARCFOUR algorithm
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
 *    The ARCFOUR algorithm was publicly disclosed on 94/09.
 *
 *    http://groups.google.com/group/sci.crypt/msg/10a300c9d21afca0
 */

#include "arc4.h"

/*
 * ARC4 key schedule
 */
void arc4_setup( arc4_context *ctx, const unsigned char *key, int32_t keylen_bits )
{
    int32_t i, j, k, a;
    unsigned char *m;

    ctx->x = 0;
    ctx->y = 0;
    m = ctx->m;

    for (i = 0; i < 256; i++)
        m[i] = (unsigned char)i;

    j = k = 0;

    for (i = 0; i < 256; i++, k++) {
        if(k >= keylen_bits/8)
            k = 0;

        a = m[i];
        j = (j + a + key[k]) & 0xFF;
        m[i] = m[j];
        m[j] = (unsigned char)a;
    }
}

/*
 * ARC4 cipher function
 */
void arc4_crypt( arc4_context *ctx, int32_t buflen,
        unsigned char * input_output )
{
    int32_t i, x, y, a, b;
    unsigned char *m;

    x = ctx->x;
    y = ctx->y;
    m = ctx->m;

    for (i = 0; i < buflen; i++) {
        x = (x + 1) & 0xFF;
        a = m[x];
        y = (y + a) & 0xFF;
        b = m[y];

        m[x] = (unsigned char)b;
        m[y] = (unsigned char)a;

        input_output[i] = (unsigned char)
            (input_output[i] ^ m[(unsigned char)( a + b )]);
    }

    ctx->x = x;
    ctx->y = y;
}

#if defined(TROPICSSL_SELF_TEST)

#include <string.h>
#include <stdio.h>

/*
 * ARC4 tests vectors as posted by Eric Rescorla in sep. 1994:
 *
 * http://groups.google.com/group/comp.security.misc/msg/10a300c9d21afca0
 */
static const unsigned char arc4_test_key[3][8] = {
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static const unsigned char arc4_test_pt[3][8] = {
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static const unsigned char arc4_test_ct[3][8] = {
    {0x75, 0xB7, 0x87, 0x80, 0x99, 0xE0, 0xC5, 0x96},
    {0x74, 0x94, 0xC2, 0xE7, 0x10, 0x4B, 0x08, 0x79},
    {0xDE, 0x18, 0x89, 0x41, 0xA3, 0x37, 0x5D, 0x3A}
};

/*
 * Checkup routine
 */
int32_t arc4_self_test(int32_t verbose)
{
    int32_t i;
    unsigned char in_out_buf[8];
    arc4_context ctx;

    for (i = 0; i < 3; i++) {
        if (verbose != 0)
            printf("  ARC4 test #%ld: ", (long int)i + 1);

        memcpy(in_out_buf, arc4_test_pt[i], 8);

        arc4_setup( &ctx, (unsigned char *) arc4_test_key[i], 8 * 8 );
        arc4_crypt( &ctx, 8, in_out_buf );

        if (memcmp(in_out_buf, arc4_test_ct[i], 8) != 0) {
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
