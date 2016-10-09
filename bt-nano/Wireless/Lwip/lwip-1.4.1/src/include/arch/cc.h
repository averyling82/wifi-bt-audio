/* Copyright (C) 2012 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: cc.h
Desc: - Architecture environment, some compiler specific, some
        environment specific (probably should move env stuff
        to sys_arch.h.)
Author: dgl
Date: 2013-11-20
Notes:
*/
#ifndef __CC_H__
#define __CC_H__

#define LWIP_DEBUG

/*
  Typedefs for the types used by lwip -
    u8_t, s8_t, u16_t, s16_t, u32_t, s32_t, mem_ptr_t
*/
typedef     unsigned char        u8_t;
typedef        signed char            s8_t;
typedef     unsigned short        u16_t;
typedef     signed short        s16_t;
typedef     unsigned long        u32_t;
typedef     signed long            s32_t;
typedef     u32_t                mem_ptr_t;
typedef     int                 sys_prot_t;

#define BYTE_ORDER LITTLE_ENDIAN

/*
  Compiler hints for packing lwip's structures -
    PACK_STRUCT_FIELD(x)
    PACK_STRUCT_STRUCT
    PACK_STRUCT_BEGIN
    PACK_STRUCT_END
*/
#if defined(__arm__) && defined(__ARMCC_VERSION)
    //
    // Setup PACKing macros for KEIL/RVMDK Tools
    //
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(x) x
#endif
//-----------20150827-----------------
#if defined(__arm__) && defined(__GNUC__)
    //
    // Setup PACKing macros for KEIL/RVMDK Tools
    //
    #define PACK_STRUCT_BEGIN
    #define PACK_STRUCT_STRUCT __attribute__((packed))
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(x) x
#endif
//------------------------------------

/*
  Platform specific diagnostic output -
    LWIP_PLATFORM_DIAG(x)    - non-fatal, print a message.
    LWIP_PLATFORM_ASSERT(x)  - fatal, print message and abandon execution.
    Portability defines for printf formatters:
    U16_F, S16_F, X16_F, U32_F, S32_F, X32_F, SZT_F

*/
#define LWIP_PLATFORM_DIAG(x)             rk_printf x   //not use
#define LWIP_PLATFORM_ASSERT(x)            printf("%s", x)

#define U16_F     "u"
#define S16_F     "d"
#define X16_F     "x"
#define U32_F     "u"
#define S32_F     "d"
#define X32_F     "x"
#define SZT_F     "u"



/*
  "lightweight" synchronization mechanisms -
    SYS_ARCH_DECL_PROTECT(x) - declare a protection state variable.
    SYS_ARCH_PROTECT(x)      - enter protection mode.
    SYS_ARCH_UNPROTECT(x)    - leave protection mode.
*/

//#define SYS_ARCH_DECL_PROTECT(x)
//#define SYS_ARCH_PROTECT(x)
//#define SYS_ARCH_UNPROTECT(x)

/* make lwip/arch.h define the codes which are used throughout. */
#define LWIP_PROVIDE_ERRNO


#endif /* __CC_H__*/
