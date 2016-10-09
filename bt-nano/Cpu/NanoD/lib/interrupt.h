/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name：   interrupt.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#undef  EXT
#ifdef _IN_INTTERUPT_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/
//中断服务程序类型定义
#if defined(__arm__) && defined(__ARMCC_VERSION)
typedef void(*ExecFunPtr)(void) __irq;
#elif defined(__arm__) && defined(__GNUC__)
typedef void(*ExecFunPtr)(void);
#define __irq ;
#else
#error Unknown compiling tools
#endif

//section define
#define _ATTR_VECTTAB_AP_              __attribute__((section("ap_vect")))
#define _ATTR_VECTTAB_BB_             __attribute__((section("bb_vect")))


//中断优先级定义
#define NUM_PRIORITY_BITS           4

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
//中断ID号定义
typedef enum
{
    //Cortex M3 System Intterupt
    FAULT_ID0_REV = (UINT32)0,                                  //SP Point
    FAULT_ID1_REV,                                              //Reset
    FAULT_ID2_NMI,                                              //NMI fault
    FAULT_ID3_HARD,                                             //Hard fault
    FAULT_ID4_MPU,                                              //MPU fault
    FAULT_ID5_BUS,                                              //Bus fault
    FAULT_ID6_USAGE,                                            //Usage fault
    FAULT_ID7_REV,                                              //
    FAULT_ID8_REV,                                              //
    FAULT_ID9_REV,                                              //
    FAULT_ID10_REV,                                             //
    FAULT_ID11_SVCALL,                                          //SVCall
    FAULT_ID12_DEBUG,                                           //Debug monitor
    FAULT_ID13_REV,                                             //
    FAULT_ID14_PENDSV,                                          //PendSV
    FAULT_ID15_SYSTICK,                                         //System Tick

    /*nanoD bsp interrupt number*/
    INT_ID_SFC,
    INT_ID_SYNTH,
    INT_ID_EBC,
    INT_ID_EMMC,
    INT_ID_SDMMC,
    INT_ID_USBC,
    INT_ID_DMA,
    INT_ID_IMDCT,
    INT_ID_WDT,
    INT_ID_MAILBOX0,
    INT_ID_MAILBOX1,
    INT_ID_MAILBOX2,
    INT_ID_MAILBOX3,
    INT_ID_REV0,
    INT_ID_REV1,
    INT_ID_REV2,
    INT_ID_PWM1,
    INT_ID_PWM0,
    INT_ID_TIMER1,
    INT_ID_TIMER0,
    INT_ID_SRADC,
    INT_ID_UART5,
    INT_ID_UART4,
    INT_ID_UART3,
    INT_ID_UART2,
    INT_ID_UART1,
    INT_ID_UART0,
    INT_ID_SPI1,
    INT_ID_SPI0,
    INT_ID_I2C2,
    INT_ID_I2C1,
    INT_ID_I2C0,
    INT_ID_I2S1,
    INT_ID_I2S0,
    INT_ID_HIFI,
    INT_ID_PMU,
    INT_ID_GPIO2,
    INT_ID_GPIO1,
    INT_ID_GPIO0,
    INT_ID_VOP,
    INT_ID_DMA2,

    NUM_INTERRUPTS                                              //Interrupts Total Nums

} INTERRUPT_ID;


/*
*-------------------------------------------------------------------------------
*
*                           Functon Declaration
*
*-------------------------------------------------------------------------------
*/

extern void   IntMasterEnable(void);
extern void   IntMasterDisable(void);
extern void   FaultMasterEnable(void);
extern void   FaultMasterDisable(void);
extern void   IntPriorityGroupingSet(UINT32 ulBits);
extern UINT32 IntPriorityGroupingGet(void);
extern void   IntPrioritySet(UINT32 ulInterrupt, UINT32 ucPriority);
extern UINT32 IntPriorityGet(UINT32 ulInterrupt);
extern void   IntEnable(UINT32 ulInterrupt);
extern void   IntDisable(UINT32 ulInterrupt);
extern void   IntPendingSet(UINT32 ulInterrupt);
extern void   IntPendingClear(UINT32 ulInterrupt);
extern BOOL   IntISRPendingCheck(void);
extern UINT32 IntVectPendingGet(void);
extern UINT32 IntVectActiveGet(void);
extern uint32 IsrDisable(UINT32 ulInterrupt);
extern void   IsrEnable(UINT32 ulInterrupt, uint32 IsEnable);
extern __irq void IntDefaultHandler(void);
extern void IntRegister(UINT32 ulInterrupt, ExecFunPtr(*pfnHandler)(void));
extern void IntUnregister(UINT32 ulInterrupt);
extern uint32 UserIsrDisable(void);
extern void UserIsrEnable(uint32 data);
extern void UserIsrRequest(void);
extern void IrqPriorityInit(void);

extern void   IntMasterEnable2(void);
extern void   IntMasterDisable2(void);
extern void   FaultMasterEnable2(void);
extern void   FaultMasterDisable2(void);
extern void   IntPriorityGroupingSet2(UINT32 ulBits);
extern UINT32 IntPriorityGroupingGet2(void);
extern void   IntPrioritySet2(UINT32 ulInterrupt, UINT32 ucPriority);
extern UINT32 IntPriorityGet2(UINT32 ulInterrupt);
extern void   IntEnable2(UINT32 ulInterrupt);
extern void   IntDisable2(UINT32 ulInterrupt);
extern void   IntPendingSet2(UINT32 ulInterrupt);
extern void   IntPendingClear2(UINT32 ulInterrupt);
extern BOOL   IntISRPendingCheck2(void);
extern UINT32 IntVectPendingGet2(void);
extern UINT32 IntVectActiveGet2(void);
extern uint32 IsrDisable2(UINT32 ulInterrupt);
extern void   IsrEnable2(UINT32 ulInterrupt, uint32 IsEnable);
extern __irq void IntDefaultHandler2(void);
extern void IntRegister2(UINT32 ulInterrupt, ExecFunPtr(*pfnHandler)(void));
extern void IntUnregister2(UINT32 ulInterrupt);
extern uint32 UserIsrDisable2(void);
extern void UserIsrEnable2(uint32 data);
extern void UserIsrRequest2(void);
extern void IrqPriorityInit2(void);



/*
********************************************************************************
*
*                         End of interrupt.h
*
********************************************************************************
*/
#endif

