#ifndef WPAS_RC4_H
#define WPAS_RC4_H

#ifdef OPENSSL_NO_RC4
#error RC4 is disabled.
#endif

#define RC4_INT uint32

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct rc4_key_st
{
    RC4_INT x,y;
    RC4_INT data[256];
} RC4_KEY;


//const uint8 *RC4_options(void);
void RC4_set_key(RC4_KEY *key, int32 len, const uint8 *data);
void RC4(RC4_KEY *key, uint32 len, const uint8 *indata,
         uint8 *outdata);

#ifdef  __cplusplus
}
#endif

#endif
