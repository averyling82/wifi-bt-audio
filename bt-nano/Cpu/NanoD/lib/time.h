/*
********************************************************************************
*                   Copyright (c) 2008,anzhiguo
*                         All rights reserved.
*
* File Name：   timer.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#undef  EXT
#ifdef  _IN_TIMER_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
/********************************************************************
**                      对外函数接口声明                            *
********************************************************************/
extern uint32 TimerStart(eTIMER_NUM timerNum);
extern uint32 TimerStop(eTIMER_NUM timerNum);              //停止TIMER
extern uint64 TimerGetCount(eTIMER_NUM timerNum);          //查询当前计数值
extern uint32 TimerSetCount(eTIMER_NUM timerNum,uint64 usTick);
extern uint32 TimerInit(eTIMER_NUM timerNum,eTIMER_MODE mode);

//extern uint32 TimerUpdateApbFreq(uint32 APBfreq);           //APB CLK改变时调整计数初值
extern uint32 TimerClrInt(eTIMER_NUM timerNum);

//For test
extern uint32 TimerIntMask(eTIMER_NUM timerNum);              //MASK Timer中断
extern uint32 TimerIntUnmask(eTIMER_NUM timerNum);            //UNMASK Timer中断
//pFunc       g_timerIRQ[2];  //保存TIMER中断回调函数
//uint32      g_timerTick[2]; //保存TIMER定时毫秒数

//#else

//typedef void   (*pTimerEnable)(void);
//typedef void   (*pTimerDisable)(void);
//typedef uint32 (*pTimerPeriodGet)(UINT32 PCLK);
//typedef UINT32 (*pTimer_GetCount)(void);
//typedef void   (*pTimerIntEnable)(void);
//typedef void   (*pTimerIntDisable)(void);
//typedef uint32 (*pTimerClearIntFlag)(void);
//typedef UINT32 (*pTimerIntStateGet)(void);
//
//#define TimerEnable()        (((pTimerEnable      )(Addr_TimerEnable      ))())
//#define TimerDisable()       (((pTimerDisable     )(Addr_TimerDisable     ))())
//#define TimerPeriodGet(PCLK) (((pTimerPeriodGet   )(Addr_TimerPeriodGet   ))(PCLK))
//#define Timer_GetCount()     (((pTimer_GetCount   )(Addr_Timer_GetCount   ))())
//#define TimerIntEnable()     (((pTimerIntEnable   )(Addr_TimerIntEnable   ))())
//#define TimerIntDisable()    (((pTimerIntDisable  )(Addr_TimerIntDisable  ))())
//#define TimerClearIntFlag()  (((pTimerClearIntFlag)(Addr_TimerClearIntFlag))())
//#define TimerIntStateGet()   (((pTimerIntStateGet )(Addr_TimerIntStateGet ))())
//
//#endif

/*
********************************************************************************
*
*                         End of timer.h
*
********************************************************************************
*/
#endif
