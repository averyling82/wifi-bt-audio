#ifndef WPAS_HMAC_H
#define WPAS_HMAC_H

/* The SHS block size and message digest sizes, in bytes */

#define SHS_DATASIZE    64
#define SHS_DIGESTSIZE  20

#ifndef SHA_DIGESTSIZE
#define SHA_DIGESTSIZE  20
#endif

#ifndef SHA_BLOCKSIZE
#define SHA_BLOCKSIZE   64
#endif

#ifndef MD5_DIGESTSIZE
#define MD5_DIGESTSIZE  16
#endif

#ifndef MD5_BLOCKSIZE
#define MD5_BLOCKSIZE   64
#endif

#define SHA1HashSize 20

#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif

typedef uint8 BYTE;

typedef struct
{
    uint32 digest[ 5 ];            /* Message digest */
    uint32 countLo, countHi;       /* 64-bit bit count */
    uint32 data[ 16 ];             /* SHS data buffer */
    int32 Endianness;
} SHA_CTX;

//typedef uint16    int_least16_t;
/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct SHA1Context
{
    uint32 Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */

    uint32 Length_Low;            /* Message length in bits      */
    uint32 Length_High;           /* Message length in bits      */

    /* Index into message block array   */
    uint16 Message_Block_Index;
    uint8 Message_Block[64];      /* 512-bit message blocks      */

    int32 Computed;               /* Is the digest computed?         */
    int32 Corrupted;             /* Is the message digest corrupted? */
} SHA1Context;

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};
#endif


void SHAInit(SHA_CTX *);
void SHAUpdate(SHA_CTX *, BYTE *buffer, int32 count);
void SHAFinal(BYTE *output, SHA_CTX *);

int32 SHA1Reset(  SHA1Context *);
int32 SHA1Input(  SHA1Context *,
                  const uint8 *,
                  uint32);
int32 SHA1Result( SHA1Context *,
                  uint8 Message_Digest[SHA1HashSize]);

void SHA1PadMessage(SHA1Context *);
void SHA1ProcessMessageBlock(SHA1Context *);

void hmac_sha(
    uint8*  k,     /* secret key */
    int32   lk,    /* length of the key in bytes */
    uint8*  d,     /* data */
    int32   ld,    /* length of data in bytes */
    uint8*  out,   /* output buffer, at least "t" bytes */
    int32   t
);

void hmac_sha1(uint8 *text, int32 text_len, uint8 *key,
               int32 key_len, uint8 *digest);

void hmac_md5(uint8 *text, int32 text_len, uint8 *key,
              int32 key_len, void * digest);

#endif
