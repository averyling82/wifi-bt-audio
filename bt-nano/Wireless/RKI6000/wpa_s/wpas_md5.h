#ifndef WPAS_MD5_H
#define WPAS_MD5_H

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this

software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */


/* POINTER defines a generic pointer type */
typedef uint8 *POINTER;

/* MD5 context. */
typedef struct
{
    uint32 state[4];                                   /* state (ABCD) */
    uint32 count[2];        /* number of bits, modulo 2^64 (lsb first) */
    uint8 buffer[64];                         /* input buffer */
} MD5_CTX;

void wlan_MD5_Init(MD5_CTX *);
void wlan_MD5_Update(MD5_CTX *, uint8 *, uint32);
void wlan_MD5_Final(uint8 [16], MD5_CTX *);
//void hmac_md5(uint8 *, int32, uint8 *  , int32, uint8 *  );


#endif
