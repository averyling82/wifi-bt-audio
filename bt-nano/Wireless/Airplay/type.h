#ifndef _TYPE_H
#define _TYPE_H

#if defined(__arm__) && defined(__ARMCC_VERSION)
typedef signed char  int8_t;
typedef signed short int16_t;
typedef signed int   int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

typedef unsigned int size_t;
typedef int ssize_t;

typedef uint8_t u8;
typedef uint16_t u16;
#elif defined(__arm__) && defined(__GNUC__)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef uint8_t u8;
typedef uint16_t u16;

#endif

#ifdef AES_LONG
typedef unsigned long u32;
#else
typedef unsigned int u32;
#endif

#define __be32 u32
#define __be16 u16
#define __u32 u32
#define __u8 u8

#ifndef byte
    typedef unsigned char   byte;
#endif
#ifndef word16
    typedef unsigned short  word16;
#endif
#ifndef word32
    typedef unsigned int    word32;
#endif

typedef unsigned short  sa_family_t;

#endif
