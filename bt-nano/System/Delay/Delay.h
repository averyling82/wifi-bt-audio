/*
********************************************************************************
*  Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                        All rights reserved.
*
* File Name£º   delay.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _DELAY_H_
#define _DELAY_H_

#undef  EXT
#ifdef  _IN_DELAY_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
//#ifdef DRIVERLIB_IRAM

extern void Delay10cyc(UINT32 count);
extern void Delay100cyc(UINT32 count);
extern void GetTimeHMS(uint32 TempSec, uint16 *pHour, uint8 *pMin, uint8 *pSec);

//#else
//
//typedef void (*pDelay10cyc)(UINT32 count);
//typedef void (*pDelay100cyc)(UINT32 count);
//typedef void (*pGetTimeHMS)(uint32 TempSec, uint16 *pHour, uint8 *pMin, uint8 *pSec);
//
//#define Delay10cyc(count)                      (((pDelay10cyc )(Addr_Delay10cyc ))(count))
//#define Delay100cyc(count)                     (((pDelay100cyc)(Addr_Delay100cyc))(count))
//#define GetTimeHMS(TempSec, pHour, pMin, pSec) (((pGetTimeHMS )(Addr_GetTimeHMS ))(TempSec, pHour, pMin, pSec))
//
//#endif

extern void DelayUs(UINT32 us);
extern void DelayMs(UINT32 ms);


/*
********************************************************************************
*
*                         End of delay.h
*
********************************************************************************
*/
#endif

