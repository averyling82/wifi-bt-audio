#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include "mdns.h"
#include "lwip/sockets.h"
#include "type.h"

//#define AIRPLAY_DEBUG
#ifdef AIRPLAY_DEBUG
//#define AIRPLAY_DEBUGF(format,...)    printf("FILE: %s, LINE: %d: "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define AIRPLAY_DEBUGF(format,...)      printf(format, ##__VA_ARGS__)
#else
#define AIRPLAY_DEBUGF(format,...)
#endif

// struct sockaddr_in6 is bigger than struct sockaddr. derp
#ifdef AF_INET6
#define SOCKADDR struct sockaddr_storage
#define SAFAMILY ss_family
#else
#define SOCKADDR struct sockaddr
#define SAFAMILY sa_family
#endif

#define RSA_MODE_AUTH (0)
#define RSA_MODE_KEY  (1)

typedef struct {
    char *password;
    char *apname;
    uint8_t hw_addr[6];
    int port;
    char *mdns_name;
    mdns_backend *mdns;
    int airplay_init_status;
} shairport_cfg;

extern shairport_cfg config;

uint8_t *base64_dec(char *src, int *outlen);
char *base64_enc(uint8_t *src,int srclen, int *outlen);
uint8_t *rsa_apply(uint8_t *input, int inlen, int *outlen, int mode);
void shairport_shutdown(void);
void shairport_startup_complete(void);
#endif // _COMMON_H

