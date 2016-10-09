#ifndef WPAS_AES_H
#define WPAS_AES_H

typedef struct
{
    u32 erk[64];                    /* encryption round keys */
    u32 drk[64];                    /* decryption round keys */
    int32 nr;                               /* number of rounds */
} aes_context;


#define AES_BLOCKSIZE8  8
#define AES_BLK_SIZE        16      // # octets in an AES block
typedef union _aes_block        // AES cipher block
{
    uint32  x[AES_BLK_SIZE/4];      // access as 8-bit octets or 32-bit words
    uint8  b[AES_BLK_SIZE];
} aes_block;


void AES_WRAP(uint8 * plain, int32 plain_len,
              uint8 * iv, int32 iv_len,
              uint8 * kek,    int32 kek_len,
              uint8 *cipher, uint16 *cipher_len);

void AES_UnWRAP(uint8 * cipher, int32 cipher_len,
                uint8 * kek,   int32 kek_len,
                uint8 * plain);

#endif
