/*
********************************************************************************
*                   Copyright (c) 2008,yangwenjie
*                         All rights reserved.
*
* File Name£º   I2c.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _RTC_H_
#define _RTC_H_





//RTC_CTRL
#define RTC_COMPESSION_UPDATE           ((UINT32)0x00000001 << 3)
#define RTC_COMPESSION_SET              ((UINT32)0x00000001 << 3)
#define RTC_TIME_UPDATE                  ((UINT32)0x00000001 << 2)
#define RTC_TIME_NO_UPDATE              ((UINT32)0x00000000 << 2)
#define RTC_INT_ENABLE                   ((UINT32)0x00000001 << 1)
#define RTC_INT_DISABLE                 ((UINT32)0x00000000 << 1)
#define RTC_ENABLE                       ((UINT32)0x00000001 << 0)
#define RTC_DISABLE                      ((UINT32)0x00000000 << 0)

















#endif
