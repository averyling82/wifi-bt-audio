#ifndef __WLAN_ENDIAN_H_
#define __WLAN_ENDIAN_H_

#include "wlan_cfg.h"

#ifndef __u16
typedef unsigned short __u16;
#endif

#ifndef __u32
typedef unsigned long   __u32;
#endif

#ifndef __uint8
typedef unsigned char __uint8;
#endif

#ifndef __u64
typedef unsigned long long  __u64;
#endif

typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

__inline static __u16  ___swap16(__u16 x)
{
    __u16 __x = x;
    return
        ((__u16)(
             (((__u16)(__x) & (__u16)0x00ffU) << 8) |
             (((__u16)(__x) & (__u16)0xff00U) >> 8) ));

}

__inline static __u32  ___swap32(__u32 x)
{
    __u32 __x = (x);
    return ((__u32)(
                (((__u32)(__x) & (__u32)0x000000ffUL) << 24) |
                (((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) |
                (((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) |
                (((__u32)(__x) & (__u32)0xff000000UL) >> 24) ));
}

__inline static __u64  ___swap64(__u64 x)
{
    __u64 __x = (x);

    return
        ((__u64)( \
                  (__u64)(((__u64)(__x) & (__u64)0x00000000000000ffULL) << 56) | \
                  (__u64)(((__u64)(__x) & (__u64)0x000000000000ff00ULL) << 40) | \
                  (__u64)(((__u64)(__x) & (__u64)0x0000000000ff0000ULL) << 24) | \
                  (__u64)(((__u64)(__x) & (__u64)0x00000000ff000000ULL) <<  8) | \
                  (__u64)(((__u64)(__x) & (__u64)0x000000ff00000000ULL) >>  8) | \
                  (__u64)(((__u64)(__x) & (__u64)0x0000ff0000000000ULL) >> 24) | \
                  (__u64)(((__u64)(__x) & (__u64)0x00ff000000000000ULL) >> 40) | \
                  (__u64)(((__u64)(__x) & (__u64)0xff00000000000000ULL) >> 56) ));
    \
}

#ifndef __arch__swap16
__inline static __u16 __arch__swap16(__u16 x)
{
    return ___swap16(x);
}

#endif

#ifndef __arch__swap32
__inline static __u32 __arch__swap32(__u32 x)
{
    __u32 __tmp = (x) ;
    return ___swap32(__tmp);
}
#endif

#ifndef __arch__swap64

__inline static __u64 __arch__swap64(__u64 x)
{
    __u64 __tmp = (x) ;
    return ___swap64(__tmp);
}


#endif

#define __swap16(x) __fswap16(x)
#define __swap32(x) __fswap32(x)
#define __swap64(x) __fswap64(x)

__inline static __u16 __fswap16(__u16 x)
{
    return __arch__swap16(x);
}
__inline static __u32 __fswap32(__u32 x)
{
    return __arch__swap32(x);
}

#define swap16 __swap16
#define swap32 __swap32
#define swap64 __swap64
#define swap16p __swap16p
#define swap32p __swap32p
#define swap64p __swap64p
#define swap16s __swap16s
#define swap32s __swap32s
#define swap64s __swap64s


#if defined (CONFIG_LITTLE_ENDIAN)

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif
#ifndef __LITTLE_ENDIAN_BITFIELD
#define __LITTLE_ENDIAN_BITFIELD
#endif

#define __constant_htonl(x) ___constant_swap32((x))
#define __constant_ntohl(x) ___constant_swap32((x))
#define __constant_htons(x) ___constant_swap16((x))
#define __constant_ntohs(x) ___constant_swap16((x))
#define __constant_cpu_to_le64(x) ((__u64)(x))
#define __constant_le64_to_cpu(x) ((__u64)(x))
#define __constant_cpu_to_le32(x) ((__u32)(x))
#define __constant_le32_to_cpu(x) ((__u32)(x))
#define __constant_cpu_to_le16(x) ((__u16)(x))
#define __constant_le16_to_cpu(x) ((__u16)(x))
#define __constant_cpu_to_be64(x) ___constant_swap64((x))
#define __constant_be64_to_cpu(x) ___constant_swap64((x))
#define __constant_cpu_to_be32(x) ___constant_swap32((x))
#define __constant_be32_to_cpu(x) ___constant_swap32((x))
#define __constant_cpu_to_be16(x) ___constant_swap16((x))
#define __constant_be16_to_cpu(x) ___constant_swap16((x))
#define __cpu_to_le64(x) ((__u64)(x))
#define __le64_to_cpu(x) ((__u64)(x))
#define __cpu_to_le32(x) ((__u32)(x))
#define __le32_to_cpu(x) ((__u32)(x))
#define __cpu_to_le16(x) ((__u16)(x))
#define __le16_to_cpu(x) ((__u16)(x))
#define __cpu_to_be64(x) __swap64((x))
#define __be64_to_cpu(x) __swap64((x))
#define __cpu_to_be32(x) __swap32((x))
#define __be32_to_cpu(x) __swap32((x))
#define __cpu_to_be16(x) __swap16((x))
#define __be16_to_cpu(x) __swap16((x))
#define __cpu_to_le64p(x) (*(__u64*)(x))
#define __le64_to_cpup(x) (*(__u64*)(x))
#define __cpu_to_le32p(x) (*(__u32*)(x))
#define __le32_to_cpup(x) (*(__u32*)(x))
#define __cpu_to_le16p(x) (*(__u16*)(x))
#define __le16_to_cpup(x) (*(__u16*)(x))
#define __cpu_to_be64p(x) __swap64p((x))
#define __be64_to_cpup(x) __swap64p((x))
#define __cpu_to_be32p(x) __swap32p((x))
#define __be32_to_cpup(x) __swap32p((x))
#define __cpu_to_be16p(x) __swap16p((x))
#define __be16_to_cpup(x) __swap16p((x))
#define __cpu_to_le64s(x) do {} while (0)
#define __le64_to_cpus(x) do {} while (0)
#define __cpu_to_le32s(x) do {} while (0)
#define __le32_to_cpus(x) do {} while (0)
#define __cpu_to_le16s(x) do {} while (0)
#define __le16_to_cpus(x) do {} while (0)
#define __cpu_to_be64s(x) __swap64s((x))
#define __be64_to_cpus(x) __swap64s((x))
#define __cpu_to_be32s(x) __swap32s((x))
#define __be32_to_cpus(x) __swap32s((x))
#define __cpu_to_be16s(x) __swap16s((x))
#define __be16_to_cpus(x) __swap16s((x))

#elif defined (CONFIG_BIG_ENDIAN)

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif
#ifndef __BIG_ENDIAN_BITFIELD
#define __BIG_ENDIAN_BITFIELD
#endif

#define __constant_htonl(x) ((__u32)(x))
#define __constant_ntohl(x) ((__u32)(x))
#define __constant_htons(x) ((__u16)(x))
#define __constant_ntohs(x) ((__u16)(x))
#define __constant_cpu_to_le64(x) ___constant_swap64((x))
#define __constant_le64_to_cpu(x) ___constant_swap64((x))
#define __constant_cpu_to_le32(x) ___constant_swap32((x))
#define __constant_le32_to_cpu(x) ___constant_swap32((x))
#define __constant_cpu_to_le16(x) ___constant_swap16((x))
#define __constant_le16_to_cpu(x) ___constant_swap16((x))
#define __constant_cpu_to_be64(x) ((__u64)(x))
#define __constant_be64_to_cpu(x) ((__u64)(x))
#define __constant_cpu_to_be32(x) ((__u32)(x))
#define __constant_be32_to_cpu(x) ((__u32)(x))
#define __constant_cpu_to_be16(x) ((__u16)(x))
#define __constant_be16_to_cpu(x) ((__u16)(x))
#define __cpu_to_le64(x) __swap64((x))
#define __le64_to_cpu(x) __swap64((x))
#define __cpu_to_le32(x) __swap32((x))
#define __le32_to_cpu(x) __swap32((x))
#define __cpu_to_le16(x) __swap16((x))
#define __le16_to_cpu(x) __swap16((x))
#define __cpu_to_be64(x) ((__u64)(x))
#define __be64_to_cpu(x) ((__u64)(x))
#define __cpu_to_be32(x) ((__u32)(x))
#define __be32_to_cpu(x) ((__u32)(x))
#define __cpu_to_be16(x) ((__u16)(x))
#define __be16_to_cpu(x) ((__u16)(x))
#define __cpu_to_le64p(x) __swap64p((x))
#define __le64_to_cpup(x) __swap64p((x))
#define __cpu_to_le32p(x) __swap32p((x))
#define __le32_to_cpup(x) __swap32p((x))
#define __cpu_to_le16p(x) __swap16p((x))
#define __le16_to_cpup(x) __swap16p((x))
#define __cpu_to_be64p(x) (*(__u64*)(x))
#define __be64_to_cpup(x) (*(__u64*)(x))
#define __cpu_to_be32p(x) (*(__u32*)(x))
#define __be32_to_cpup(x) (*(__u32*)(x))
#define __cpu_to_be16p(x) (*(__u16*)(x))
#define __be16_to_cpup(x) (*(__u16*)(x))
#define __cpu_to_le64s(x) __swap64s((x))
#define __le64_to_cpus(x) __swap64s((x))
#define __cpu_to_le32s(x) __swap32s((x))
#define __le32_to_cpus(x) __swap32s((x))
#define __cpu_to_le16s(x) __swap16s((x))
#define __le16_to_cpus(x) __swap16s((x))
#define __cpu_to_be64s(x) do {} while (0)
#define __be64_to_cpus(x) do {} while (0)
#define __cpu_to_be32s(x) do {} while (0)
#define __be32_to_cpus(x) do {} while (0)
#define __cpu_to_be16s(x) do {} while (0)
#define __be16_to_cpus(x) do {} while (0)

#else

#error "Must be LITTLE/BIG Endian Host"

#endif


__inline static  uint16 le_read_cpu16(uint8* pdata)
{
    uint16 a0 = 0;
    uint16 a1 = 0;

    a0 = *pdata;
    a1 = *(pdata + 1);

#ifdef CONFIG_BIG_ENDIAN
    return (a1 |(a0 << 8));
#else
    return (a0 |(a1 << 8));
#endif
}

__inline static  uint32 le_read_cpu32(uint8* pdata)
{
    uint32 a0 = 0;
    uint32 a1 = 0;
    uint32 a2 = 0;
    uint32 a3 = 0;

    a0 = *pdata;
    a1 = *(pdata + 1);
    a2 = *(pdata + 2);
    a3 = *(pdata + 3);

#ifdef CONFIG_BIG_ENDIAN
    return (a3 |(a2 << 8) | (a1 << 16) | (a0 << 24));
#else
    return (a0 |(a1 << 8) | (a2 << 16) | (a3 << 24));
#endif
}

//le32_to_cpu
__inline static  uint16 le_write_cpu16(uint8* pdata)
{
    uint16 a0 = 0;
    uint16 a1 = 0;

    a0 = *pdata;
    a1 = *(pdata + 1);

#ifdef CONFIG_BIG_ENDIAN
    return (a0 |(a1 << 8));
#else
    return (a0 |(a1 << 8));
#endif
}

__inline static  uint32 le_write_cpu32(uint8* pdata)
{
    uint32 a0 = 0;
    uint32 a1 = 0;
    uint32 a2 = 0;
    uint32 a3 = 0;

    a0 = *pdata;
    a1 = *(pdata + 1);
    a2 = *(pdata + 2);
    a3 = *(pdata + 3);
#ifdef CONFIG_BIG_ENDIAN
    return (a0 |(a1 << 8) | (a2 << 16) | (a3 << 24));
#else
    return (a0 |(a1 << 8) | (a2 << 16) | (a3 << 24));
#endif
}

/*
 * inside the kernel, we can use nicknames;
 * outside of it, we must avoid POSIX namespace pollution...
 */
#define cpu_to_le64 __cpu_to_le64
#define le64_to_cpu __le64_to_cpu
#define cpu_to_le32 __cpu_to_le32
#define le32_to_cpu __le32_to_cpu
#define cpu_to_le16 __cpu_to_le16
#define le16_to_cpu __le16_to_cpu
#define cpu_to_be64 __cpu_to_be64
#define be64_to_cpu __be64_to_cpu
#define cpu_to_be32 __cpu_to_be32
#define be32_to_cpu __be32_to_cpu
#define cpu_to_be16 __cpu_to_be16
#define be16_to_cpu __be16_to_cpu
#define cpu_to_le64p __cpu_to_le64p
#define le64_to_cpup __le64_to_cpup
#define cpu_to_le32p __cpu_to_le32p
#define le32_to_cpup __le32_to_cpup
#define cpu_to_le16p __cpu_to_le16p
#define le16_to_cpup __le16_to_cpup
#define cpu_to_be64p __cpu_to_be64p
#define be64_to_cpup __be64_to_cpup
#define cpu_to_be32p __cpu_to_be32p
#define be32_to_cpup __be32_to_cpup
#define cpu_to_be16p __cpu_to_be16p
#define be16_to_cpup __be16_to_cpup
#define cpu_to_le64s __cpu_to_le64s
#define le64_to_cpus __le64_to_cpus
#define cpu_to_le32s __cpu_to_le32s
#define le32_to_cpus __le32_to_cpus
#define cpu_to_le16s __cpu_to_le16s
#define le16_to_cpus __le16_to_cpus
#define cpu_to_be64s __cpu_to_be64s
#define be64_to_cpus __be64_to_cpus
#define cpu_to_be32s __cpu_to_be32s
#define be32_to_cpus __be32_to_cpus
#define cpu_to_be16s __cpu_to_be16s
#define be16_to_cpus __be16_to_cpus

/*
 * Handle wlan_ntohl and suches. These have various compatibility
 * issues - like we want to give the prototype even though we
 * also have a macro for them in case some strange program
 * wants to take the address of the thing or something..
 *
 * Note that these used to return a "long" in libc5, even though
 * long is often 64-bit these days.. Thus the casts.
 *
 * They have to be macros in order to do the constant folding
 * correctly - if the argument passed into a inline function
 * it is no longer constant according to gcc..
 */

#undef wlan_ntohl
#undef wlan_ntohs
#undef wlan_htonl
#undef wlan_htons

extern uint32 wlan_ntohl(uint32);
extern uint32 wlan_htonl(uint32);
extern uint16 wlan_ntohs(uint16);
extern uint16 wlan_htons(uint16);

#define wlan_htonl(x) __cpu_to_be32(x)
#define wlan_ntohl(x) __be32_to_cpu(x)
#define wlan_htons(x) __cpu_to_be16(x)
#define wlan_ntohs(x) __be16_to_cpu(x)

#endif

