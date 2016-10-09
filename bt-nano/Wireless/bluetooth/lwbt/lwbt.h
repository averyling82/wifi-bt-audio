#ifndef _LWBT_H_
#define _LWBT_H_

#ifdef _A2DP_SOUCRE_


#define _ATTR_LWBT_CODE_         __attribute__((section("LwbtCode")))
#define _ATTR_LWBT_DATA_         __attribute__((section("LwbtUartifData")))
#define _ATTR_LWBT_BSS_          __attribute__((section("LwbtUartifBss"),zero_init))

//#define _ATTR_LWBT_UARTIF_CODE_         __attribute__((section("LwbtUartifCode")))
//#define _ATTR_LWBT_UARTIF_DATA_         __attribute__((section("LwbtUartifData")))
//#define _ATTR_LWBT_UARTIF_BSS_          __attribute__((section("LwbtUartifBss"),zero_init))

//#define _ATTR_LWBT_INIT_CODE_         __attribute__((section("LwbtInitCode")))
//#define _ATTR_LWBT_INIT_DATA_         __attribute__((section("LwbtUartifData")))
//#define _ATTR_LWBT_INIT_BSS_          __attribute__((section("LwbtUartifBss"),zero_init))
#define _ATTR_LWBT_UARTIF_CODE_         __attribute__((section("LwbtCode")))
#define _ATTR_LWBT_UARTIF_DATA_         __attribute__((section("LwbtData")))
#define _ATTR_LWBT_UARTIF_BSS_          __attribute__((section("LwbtBss"),zero_init))

#define _ATTR_LWBT_INIT_CODE_         __attribute__((section("LwbtCode")))
#define _ATTR_LWBT_INIT_DATA_         __attribute__((section("LwbtData")))
#define _ATTR_LWBT_INIT_BSS_          __attribute__((section("LwbtBss"),zero_init))
#else

#define _ATTR_LWBT_CODE_         __attribute__((section("LwbtCode")))
#define _ATTR_LWBT_DATA_         __attribute__((section("LwbtData")))
#define _ATTR_LWBT_BSS_          __attribute__((section("LwbtBss"),zero_init))

#define _ATTR_LWBT_UARTIF_CODE_         __attribute__((section("LwbtCode")))
#define _ATTR_LWBT_UARTIF_DATA_         __attribute__((section("LwbtData")))
#define _ATTR_LWBT_UARTIF_BSS_          __attribute__((section("LwbtBss"),zero_init))

#define _ATTR_LWBT_INIT_CODE_         __attribute__((section("LwbtCode")))
#define _ATTR_LWBT_INIT_DATA_         __attribute__((section("LwbtData")))
#define _ATTR_LWBT_INIT_BSS_          __attribute__((section("LwbtBss"),zero_init))

//#define _ATTR_LWBT_CODE_         __attribute__((section("LwbtCode")))
//#define _ATTR_LWBT_DATA_         __attribute__((section("LwbtData")))
//#define _ATTR_LWBT_BSS_          __attribute__((section("LwbtBss"),zero_init))

#endif
#endif