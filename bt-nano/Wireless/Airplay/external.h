#ifndef _EXTERNAL_H
#define _EXTERNAL_H

#include <stdio.h>
#include <lwip/sockets.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "type.h"

#ifndef IPPROTO_IPV6
#define IPPROTO_IPV6    41,        /* IPv6-in-IPv4 tunnelling        */
#endif

char *strtok_r(char *s, const char *delim, char **save_ptr);
char* strdup (const char *string);

const char * inet_ntop(int af, const void *src, char *dst, int size);

char *strsep_t(char **s, const char *ct);

int    gethostname(char *hostname, int len);

int nanosleep( const struct timeval * rqtp, struct timeval * rmtp );

long currentTime();

#endif
