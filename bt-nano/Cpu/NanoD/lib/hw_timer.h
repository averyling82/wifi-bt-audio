/*
********************************************************************************
*                   Copyright (c) 2008,anzhiguo
*                         All rights reserved.
*
* File Name£º   hw_timer.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_TIMER_H_
#define _HW_TIMER_H_

#undef  EXT
#ifdef  _IN_TIMER_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------

            external memory control memap register define

--------------------------------------------------------------------------------
*/
typedef volatile struct tagTIMER_REG{

    uint32 TimerLoadCount0;     // Load Count Register
    uint32 TimerLoadCount1;     // Load Count Register
    uint32 TimerCurrentValue0;  // Current Value Register
    uint32 TimerCurrentValue1;  // Current Value Register
    uint32 TimerControlReg;    // Control Register
    uint32 Reserved;           // End-of-Interrupt Register
    uint32 TimerIntStatus;     // Interrupt Status Register

} TIMER_REG,*pTIMER_REG;

typedef enum _TIMER_MODE
{
    TIMER_FREE_RUNNING = 0,
    TIMER_USER_DEFINED,
    TIMER_MODE_MAX
}eTIMER_MODE;

typedef enum _TIMER_NUM
{
    TIMER0 = 0,
    TIMER1,
    TIMER_MAX
}eTIMER_NUM;
/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the TimerControlReg .
*
*-------------------------------------------------------------------------------
*/
#define TIMR_ENABLE             ((UINT32)1<<0) //timer enable
#define TIMR_DISABLE            ((UINT32)0<<0) //timer disable

#define TIMR_USER_MODE         ((UINT32)1<<1)  // 0:free-running, 1:usermode
#define TIMR_FREE_RUN_MODE         ((UINT32)0<<1)  // 0:free-running, 1:usermode

#define TIMR_IMASK             ((UINT32)0<<2) //timer interrupt mask
#define TIMR_INOMASK           ((UINT32)1<<2) //timer interrupt mask
/*
********************************************************************************
*
*                         End of hw_timer.h
*
********************************************************************************
*/
#endif
