/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Delay.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_DELAY_

#include "RKOS.h"
extern chip_freq_t chip_freq2;
/*
--------------------------------------------------------------------------------
  Function name : void Delay100cyc(UINT16 count)
  Author        : ZHengYongzhi
  Description   : delay 10 cycle.
                  one while period has x time of cycles.

  Input         :
  Return        : TRUE

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void Delay10cyc2(UINT32 count)
{
    uint32 cycle;

    cycle = count * 2;

    while(--cycle);
}

/*
--------------------------------------------------------------------------------
  Function name : void Delay100cyc(UINT16 count)
  Author        : ZHengYongzhi
  Description   : delay 100 time cycle.

  Input         :
  Return        : TRUE

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void Delay100cyc2(UINT32 count)
{
    uint32 cycle;

    cycle = count * 20;

    while(--cycle);
}
//#endif

/*
--------------------------------------------------------------------------------
  Function name : void DelayMs_nops(UINT32 msec)
  Author        : yangwenjie
  Description   : how many us that software will to delay

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#if defined(__arm__) && defined(__ARMCC_VERSION)
__asm void ASMDelay2(uint32 i)
{
ASMDelayxx
    SUBS    R0, R0, #1
    BHI     ASMDelayxx
    BX      LR
}
#elif defined(__arm__) && defined(__GNUC__)
void ASMDelay2(uint32 i)
{
ASMDelayxx:
__asm__ volatile(
    "SUBS    R0, R0, #1\n\t"
    "BHI     ASMDelayxx\n\t"
    "BX      LR\n\t"
);
}
#else
#error Unknown compiling tools.
#endif


void DelayUs2(UINT32 us)
{
    uint32 SysFreq;
    volatile uint32 i;

    SysFreq = chip_freq2.stclk_cal_core / 1000000;
    i = (SysFreq*us)/5;

	if(i)
        ASMDelay2(i);
}

/*
--------------------------------------------------------------------------------
  Function name : void DelayMs(UINT32 mSec)
  Author        : yangwenjie
  Description   : how many ms that software will to delay

  Input         :
  Return        : ÎÞ

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void DelayMs2(UINT32 ms)
{
    ms *= chip_freq2.hclk_cal_core / (1000 * 3);

    while (--ms);
}
/*
--------------------------------------------------------------------------------
  Function name : GetTimeHMS(UINT32 TempSec, UINT16 *pHour, UINT16 *pMin, UINT16 *pSec)
  Author        : ZhengYongzhi
  Description   : get the hour,minute,second according to input second.

  Input         :
  Return        : TRUE

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void GetTimeHMS2(uint32 TempSec, uint16 *pHour, uint8 *pMin, uint8 *pSec)
{
    *pHour = (uint16)(TempSec / 3600);

    *pMin  = (uint8)((TempSec % 3600) / 60);

    *pSec  = (uint8)(TempSec % 60);
}
//#endif
/*
********************************************************************************
*
*                         End of Delay.c
*
********************************************************************************
*/

