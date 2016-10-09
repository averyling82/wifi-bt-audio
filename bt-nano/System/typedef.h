/*
********************************************************************************
*             Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* File Name  typedef.h
*
* Description:  typedef.h -Macros define the data type
*
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

/*
*-------------------------------------------------------------------------------
*
*                           macro define
*
*-------------------------------------------------------------------------------
*/
#include <stdio.h>
#include "errcode.h"



#ifndef         FALSE
#define         FALSE                      0
#endif

#ifndef         TRUE
#define         TRUE                    (!FALSE)
#endif

#define API
#define FUN
#define READ
#define WRITE
#define COMMON
#define INIT
#define SHELL



#define BUSY                1
#define OK                  0
#define ERROR               (-1)
#define TIMEOUT             (-2)

#define RETURN_OK           0
#define RETURN_FAIL         (-1)
/*
--------------------------------------------------------------------------------

                        Struct Define

--------------------------------------------------------------------------------
*/

//type define
typedef     signed   int                    sint;
typedef     unsigned int                    uint;

typedef     signed char                     int8;
typedef     unsigned char                   INT8U;
typedef     unsigned char                   uchar;
typedef     signed short                    int16;
typedef     signed long                     int32;
typedef     signed long long                int64;

typedef     unsigned char                   uint8;
typedef     unsigned short                  uint16;
typedef     unsigned long                   uint32;
typedef     unsigned long long              uint64;

typedef     volatile signed char            vint8;
typedef     volatile signed short           vint16;
typedef     volatile signed long            vint32;

typedef     volatile unsigned char          vuint8;
typedef     volatile unsigned short         vuint16;
typedef     volatile unsigned long          vuint32;

typedef     signed const char               intc8;              /* Read Only */
typedef     signed const short              intc16;             /* Read Only */
typedef     signed const long               intc32;             /* Read Only */

typedef     unsigned const char             uintc8;             /* Read Only */
typedef     unsigned const short            uintc16;            /* Read Only */
typedef     unsigned const long             uintc32;            /* Read Only */

typedef     signed   int                    INT;
typedef     unsigned int                    UINT;

typedef     signed char                     INT8;
typedef     signed short                    INT16;
typedef     signed long                     INT32;

typedef     unsigned char                   UINT8;
typedef     unsigned short                  UINT16;
typedef     unsigned long                   UINT32;
typedef     unsigned long long              UINT64;

typedef     volatile signed char            VINT8;
typedef     volatile signed short           VINT16;
typedef     volatile signed long            VINT32;

typedef     volatile unsigned char          VUINT8;
typedef     volatile unsigned short         VUINT16;
typedef     volatile unsigned long          VUINT32;

typedef     signed const char               INTC8;              /* Read Only */
typedef     signed const short              INTC16;             /* Read Only */
typedef     signed const long               INTC32;             /* Read Only */

typedef     unsigned const char             UINTC8;             /* Read Only */
typedef     unsigned const short            UINTC16;            /* Read Only */
typedef     unsigned const long             UINTC32;            /* Read Only */

typedef     void (*pFunc)(void);

//typedef     unsigned char  BOOL;
#ifndef BOOL
#define BOOL int
#endif

typedef     int                             rk_err_t;            /* ERR TYPE*/
typedef     int                             rk_size_t;           /* SIZE TYPE*/
typedef     int                             RK_ERR_T;            /* ERR TYPE*/
typedef     int                             RK_SIZE_T;           /* SIZE TYPE*/
typedef unsigned short Ucs2;

#define     bool                            BOOL
#define     BOOLEAN                         BOOL


typedef void* HANDLE;
/*
********************************************************************************
*
*                         End of typedef.h
*
********************************************************************************
*/
#endif
