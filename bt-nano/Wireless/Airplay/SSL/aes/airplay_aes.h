#ifndef AIRPLAY_AES_H
#define AIRPLAY_AES_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "type.h"

#define configASSERT( x )

#define INC_FREERTOS_H

#define AES_ENCRYPT    1
#define AES_DECRYPT    0

/* Because array size can't be a const in C, the following two are macros.
   Both sizes are in bytes. */
#define AES_MAXNR 14
#define AES_BLOCK_SIZE 16

struct aes_key_st {
#ifdef AES_LONG
    unsigned long rd_key[4 *(AES_MAXNR + 1)];
#else
    unsigned int rd_key[4 *(AES_MAXNR + 1)];
#endif
    int rounds;
};
typedef struct aes_key_st AES_KEY;

typedef void (*block128_f)(const unsigned char in[16],
            unsigned char out[16],
            const void *key);

# define GETU32(pt) (((u32)(pt)[0] << 24) ^ ((u32)(pt)[1] << 16) ^ ((u32)(pt)[2] <<  8) ^ ((u32)(pt)[3]))
# define PUTU32(ct, st) { (ct)[0] = (u8)((st) >> 24); (ct)[1] = (u8)((st) >> 16); (ct)[2] = (u8)((st) >>  8); (ct)[3] = (u8)(st); }

//aes_core.c
int private_AES_set_encrypt_key(const unsigned char *userKey, const int bits,
    AES_KEY *key);
int private_AES_set_decrypt_key(const unsigned char *userKey, const int bits,
    AES_KEY *key);

void AES_encrypt(const unsigned char *in, unsigned char *out,
    const AES_KEY *key);
void AES_decrypt(const unsigned char *in, unsigned char *out,
    const AES_KEY *key);

//aes_cbc.c
void CRYPTO_cbc128_encrypt(const unsigned char *in, unsigned char *out,
            size_t len, const void *key,
            unsigned char ivec[16], block128_f block);
void CRYPTO_cbc128_decrypt(const unsigned char *in, unsigned char *out,
            size_t len, const void *key,
            unsigned char ivec[16], block128_f block);

//aes.c
int AES_set_encrypt_key(const unsigned char *userKey, const int bits,
    AES_KEY *key);
int AES_set_decrypt_key(const unsigned char *userKey, const int bits,
    AES_KEY *key);

void AES_cbc_encrypt(const unsigned char *in, unsigned char *out,
             size_t len, const AES_KEY *key,
             unsigned char *ivec, const int enc);

#endif
