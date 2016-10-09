/*
********************************************************************************
*                   Copyright (c) 2008,yangwenjie
*                         All rights reserved.
*
* File Name£º   Hw_RTC.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_RTC_H_
#define _HW_RTC_H_

/*
*-------------------------------------------------------------------------------
*
*                         I2c memap register define
*
*-------------------------------------------------------------------------------
*/
typedef volatile struct RtcReg
{
    UINT32 RTC_CTRL;             // 0x00
    UINT32 RTC_TIME;             // 0x04
    UINT32 RTC_DATE;             // 0x08
    UINT32 RTC_TIME_SET;         // 0x0C
    UINT32 RTC_DATE_SET;         // 0x10
    UINT32 RTC_COMP;             // 0x14
    UINT32 RTC_TIME_ALARM;      // 0x18
    UINT32 RTC_DATE_ALARM;      // 0x1C
    UINT32 RTC_ALARM_EN;        // 0x20
    UINT32 RTC_ALARM_STATUS;   // 0x24
}RTCReg_t, *pRTCReg_t;
#define RTCReg                   	((RTCReg_t *) RTC_BASE)

/*
********************************************************************************
*
*                         End of hw_i2s.h
*
********************************************************************************
*/

#endif

