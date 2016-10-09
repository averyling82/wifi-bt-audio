/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   interrupt.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-7-21          1.0
*    desc:
********************************************************************************
*/
#define _IN_INTTERUPT2_

#include <typedef.h>
#include "interrupt.h"
#include <DriverInclude.h>

#if defined(__arm__) && defined(__ARMCC_VERSION)
typedef void(*ExecFunPtr)(void) __irq;
#elif defined(__arm__) && (__GNUC__)
typedef void(*ExecFunPtr)(void) ;
#else
#error Unknown compiling tools.
#endif

#define _ATTR_VECTTAB_BB_             __attribute__((section("bb_vect")))

typedef UINT32       (*p__MRS_MSP)(void);

extern int Main2(void);
extern int PowerOn_Reset2(void);
extern UINT32 Image$$BB_MAIN_STACK$$ZI$$Limit;

#if defined(__arm__) && defined(__ARMCC_VERSION)
__asm __irq void IntDefaultHandler2(void);
__attribute__((section("BBMainStack"))) __align(4) uint32 MainStack2[4064]= {0x55aa55aa,0x55aa55aa,0x55aa55aa};
#elif defined(__arm__) && (__GNUC__)
void IntDefaultHandler2(void);
__attribute__((section("BBMainStack"))) uint32 MainStack2[4064] __attribute__((aligned(4))) = {0x55aa55aa,0x55aa55aa,0x55aa55aa};
#else
#error Unknown compiling tools.
#endif
/*
--------------------------------------------------------------------------------
  Function name :
  Author        : ZHengYongzhi
  Description   : CortexM3 interrupt vector table

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VECTTAB_BB_
ExecFunPtr exceptions_table2[NUM_INTERRUPTS] = {

    /* Core Fixed interrupts start here...*/

    (ExecFunPtr)(&Image$$BB_MAIN_STACK$$ZI$$Limit),
    (ExecFunPtr)Main2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,

    //RKNano Process Intterupt
    IntDefaultHandler2,                                          //00 int_dma
    IntDefaultHandler2,                                          //01 uart
    IntDefaultHandler2,                                          //02 sd mmc
    IntDefaultHandler2,                                          //03 pwm1
    IntDefaultHandler2,                                          //04 pwm2
    IntDefaultHandler2,                                          //05 imdct36
    IntDefaultHandler2,                                          //06 synthesize
    IntDefaultHandler2,                                          //07 usb
    IntDefaultHandler2,                                          //08 i2c
    IntDefaultHandler2,                                          //09 i2s
    IntDefaultHandler2,                                          //10 gpio
    IntDefaultHandler2,                                          //11 spi
    IntDefaultHandler2,                                          //12 pwm0
    IntDefaultHandler2,                                          //13 timer
    IntDefaultHandler2,                                          //14 sar-adc
    IntDefaultHandler2,                                          //15 reserved
    IntDefaultHandler2,                                          //16 reserved
    IntDefaultHandler2,                                          //17 reserved
    IntDefaultHandler2,                                          //18 rtc
    IntDefaultHandler2,                                          //19 reserved
    IntDefaultHandler2,                                          //20 ool_bat_snsen
    IntDefaultHandler2,                                          //21 reserved
    IntDefaultHandler2,                                          //22 ool_pause
    IntDefaultHandler2,                                          //23 ool_PlayOn
    IntDefaultHandler2,                                          //24 pwr_5v_ready
    IntDefaultHandler2,                                          //25 pwr_uvlo_vdd
    IntDefaultHandler2,                                          //26 pwr_uvp
    IntDefaultHandler2,                                          //27 pwr_ovp
    IntDefaultHandler2,                                          //28 pwr_ot
    IntDefaultHandler2,                                          //29 pwr_oc
    IntDefaultHandler2,                                          //30 pwr_charge
    IntDefaultHandler2,                                          //31 reserved
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2,
    IntDefaultHandler2

};
                                                                //Interrupts Total Nums
/*
--------------------------------------------------------------------------------
  Function name :  IntDefaultHandler(void)
  Author        :  ZHengYongzhi
  Description   :  default interrupt service program.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

#if defined(__arm__) && defined(__ARMCC_VERSION)
__asm uint32 __MSR_XPSR2(void)
{
    MRS     R0, XPSR
    BX      R14
}
#elif defined(__arm__) && (__GNUC__)
uint32 __MSR_XPSR2(void)
{
__asm__(
    "MRS     R0, XPSR\n\t"
    "BX      R14\n\t"
);
}
#else
#error Unknown compiling tools.
#endif

#if defined(__arm__) && defined(__ARMCC_VERSION)
__asm __irq void IntDefaultHandler2(void)
{

    MOV R0, #0x50 ;'P'
    BL UART_SEND_BYTE
    MOV R0, #0x43 ;'C'
    BL UART_SEND_BYTE
    MOV R0, #0x3A ;':'
    BL UART_SEND_BYTE

    LDR R4, [SP, #0x18]
    BL PRINT_REG

    MOV R0, #0x0D ;
    BL UART_SEND_BYTE

    MOV R0, #0x0A ;
    BL UART_SEND_BYTE


    MOV R0, #0x4c ;'L'
    BL UART_SEND_BYTE
    MOV R0, #0x52 ;'R'
    BL UART_SEND_BYTE
    MOV R0, #0x3A ;':'
    BL UART_SEND_BYTE

    LDR R4, [SP, #0x14]
    BL PRINT_REG

    MOV R0, #0x0D ;
    BL UART_SEND_BYTE

    MOV R0, #0x0A ;
    BL UART_SEND_BYTE

    MOV R0, #0x4d ;'M'
    BL UART_SEND_BYTE
    MOV R0, #0x53 ;'S'
    BL UART_SEND_BYTE
    MOV R0, #0x50 ;'P'
    BL UART_SEND_BYTE
    MOV R0, #0x3A ;':'
    BL UART_SEND_BYTE

    MRS R4, MSP
    BL PRINT_REG

    MOV R0, #0x0D ;
    BL UART_SEND_BYTE

    MOV R0, #0x0A ;
    BL UART_SEND_BYTE


    MOV R0, #0x50 ;'P'
    BL UART_SEND_BYTE
    MOV R0, #0x53 ;'S'
    BL UART_SEND_BYTE
    MOV R0, #0x50 ;'P'
    BL UART_SEND_BYTE
    MOV R0, #0x3A ;':'
    BL UART_SEND_BYTE

    MRS R4, PSP
    BL PRINT_REG

    MOV R0, #0x0D ;
    BL UART_SEND_BYTE

    MOV R0, #0x0A ;
    BL UART_SEND_BYTE

HERE
    B        HERE




PRINT_REG
    PUSH {LR}
    MOV R5, #0x08
NEXT1
    MOV R0, R4
    LSR R0, #28
    CMP R0, #0x09
    BLS LSA
    ADD R0, #0x37

    B HIA
LSA
    ADD R0, #0x30
HIA
    BL UART_SEND_BYTE
    LSL R4, #4
    SUB R5, #1
    CBZ R5, NEXT2
    B NEXT1
NEXT2
    POP {LR}
    BX LR


UART_SEND_BYTE
    LDR   R1, =DEBUG_UART_ADDR ;UART
    LDR   R2, [R1, #0x7C] ;USR
    MOV   R3, #0X02
    AND        R2, R3
    CBNZ   R2, UART_SEND_BYTE_END
    B UART_SEND_BYTE
UART_SEND_BYTE_END
    STR  R0, [R1]  ;THR
    BX LR

}
#elif defined(__arm__) && (__GNUC__)
void IntDefaultHandler2(void)
{
__asm__(
    "MOV R0, #0x50\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x43\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x3A\n\t"
    "BL UART_SEND_BYTE\n\t"

    "LDR R4, [SP, #0x18]\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #0x0D\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MOV R0, #0x0A\n\t"
    "BL UART_SEND_BYTE\n\t"


    "MOV R0, #0x4c\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x52\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x3A\n\t"
    "BL UART_SEND_BYTE\n\t"

    "LDR R4, [SP, #0x14]\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #0x0D\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MOV R0, #0x0A\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MOV R0, #0x4d\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x53\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x50\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x3A\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R4, MSP\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #0x0D\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MOV R0, #0x0A\n\t"
    "BL UART_SEND_BYTE\n\t"


    "MOV R0, #0x50\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x53\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x50\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #0x3A\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R4, PSP\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #0x0D\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MOV R0, #0x0A\n\t"
    "BL UART_SEND_BYTE\n\t"

    "HERE:B        HERE\n\t"




    "PRINT_REG:PUSH {LR}\n\t"
    "MOV R5, #0x08\n\t"
    "NEXT1:MOV R0, R4\n\t"
    "LSR R0, #28\n\t"
    "CMP R0, #0x09\n\t"
    "BLS LSA\n\t"
    "ADD R0, #0x37\n\t"

    "B HIA\n\t"
    "LSA:ADD R0, #0x30\n\t"
    "HIA:BL UART_SEND_BYTE\n\t"
    "LSL R4, #4\n\t"
    "SUB R5, #1\n\t"
    "CBZ R5, NEXT2\n\t"
    "B NEXT1\n\t"
    "NEXT2:POP {LR}\n\t"
    "BX LR\n\t"


    "UART_SEND_BYTE:LDR   R1, =0x400B0000\n\t"
    "LDR   R2, [R1, #0x7C]\n\t"
    "MOV   R3, #0X02\n\t"
    "AND        R2, R3\n\t"
    "CBNZ   R2, UART_SEND_BYTE_END\n\t"
    "B UART_SEND_BYTE\n\t"
    "UART_SEND_BYTE_END:STR  R0, [R1]\n\t"
    "BX LR\n\t"
);
}
#else
#error Unknown compiling tools.
#endif

/*
--------------------------------------------------------------------------------
  Function name :  IntMasterEnable(void)
  Author        :  ZHengYongzhi
  Description   :  Enable processor interrupts.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntMasterEnable2(void)
{
    __SETPRIMASK2();
}

/*
--------------------------------------------------------------------------------
  Function name :  IntMasterEnable(void)
  Author        :  ZHengYongzhi
  Description   :  Disable processor interrupts.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntMasterDisable2(void)
{
    __RESETPRIMASK2();
}

/*
--------------------------------------------------------------------------------
  Function name :  IntMasterEnable(void)
  Author        :  ZHengYongzhi
  Description   :  Enable processor interrupts.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void FaultMasterEnable2(void)
{
    __RESETFAULTMASK2();
}

/*
--------------------------------------------------------------------------------
  Function name :  IntMasterEnable(void)
  Author        :  ZHengYongzhi
  Description   :  Disable processor interrupts.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void FaultMasterDisable2(void)
{
    __SETFAULTMASK2();
}


/*
--------------------------------------------------------------------------------
  Function name : IntRegister(UINT32 ulInterrupt, void (*pfnHandler)(void))
  Author        : ZHengYongzhi
  Description   : interrupts register

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntRegister2(UINT32 ulInterrupt, ExecFunPtr(*pfnHandler)(void))
{
    UINT32 ulIdx;

    // Check the arguments.
//    ASSERT(ulInterrupt < NUM_INTERRUPTS);
//
//    // Make sure that the RAM vector table is correctly aligned.
//    ASSERT(((UINT32)exceptions_table & 0x000003ff) == 0);

    // See if the RAM vector table has been initialized.
    if(nvic->VectorTableOffset != (UINT32)exceptions_table2)
    {
        // Initiale the RAM vector table.
        exceptions_table2[0] = (ExecFunPtr)&Image$$BB_MAIN_STACK$$ZI$$Limit;
        exceptions_table2[1] = (ExecFunPtr)Main2;
        for(ulIdx = 2; ulIdx < NUM_INTERRUPTS; ulIdx++)
        {
            exceptions_table2[ulIdx] = (ExecFunPtr)IntDefaultHandler2;
        }

        // Point NVIC at the RAM vector table.
        nvic->VectorTableOffset = (UINT32)exceptions_table2;
    }


    // Save the interrupt handler.
    exceptions_table2[ulInterrupt] = (ExecFunPtr)pfnHandler;
}

/*
--------------------------------------------------------------------------------
  Function name : IntUnregister(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : interrupts Unregister

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntUnregister2(UINT32 ulInterrupt)
{
    //
    // Check the arguments.
    //
//    ASSERT(ulInterrupt < NUM_INTERRUPTS);

    //
    // Reset the interrupt handler.
    //
    exceptions_table2[ulInterrupt] = (ExecFunPtr)IntDefaultHandler2;
}


/*
--------------------------------------------------------------------------------
  Function name : void IntPriorityGroupingSet(UINT32 ulBits)
  Author        : ZHengYongzhi
  Description   : priority is grouping setting
                  This function specifies the split between preemptable priority
                  levels and subpriority levels in the interrupt priority speci-
                  fication.  The range of the grouping values are dependent upon
                  the hardware implementation.

  Input         : ulBits specifies the number of bits of preemptable priority.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntPriorityGroupingSet2(UINT32 ulBits)
{
    //
    // Set the priority grouping.
    //
    nvic->APIntRst = NVIC_APINTRST_VECTKEY | ((7 - ulBits) << 8);
}

/*
--------------------------------------------------------------------------------
  Function name : void IntPriorityGroupingSet(UINT32 ulBits)
  Author        : ZHengYongzhi
  Description   : get priority grouping.
                  This function returns the split between preemptable priority
                  levels and subpriority levels in the interrupt priority spe-
                  cification.
  Input         :
  Return        : The number of bits of preemptable priority.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

UINT32 IntPriorityGroupingGet2(void)
{
    UINT32 ulValue;

    //
    // Read the priority grouping.
    //
    ulValue = nvic->APIntRst & NVIC_APINTRST_PRIGROUP_MASK;

    //
    // Return the number of priority bits.
    //
    return(7 - (ulValue >> 8));
}

/*
--------------------------------------------------------------------------------
  Function name : void IntPrioritySet(UINT32 ulInterrupt, unsigned char ucPriority)
  Author        : ZHengYongzhi
  Description   : Sets the priority of an interrupt.
                  This function is used to set the priority of an interrupt.
  Input         : ulInterrupt -- INT_ID;   ucPriority -- Priority Num.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntPrioritySet2(UINT32 ulInterrupt, UINT32 ucPriority)
{
    UINT32 ulBitOffset;
    UINT32 *pRegister;

    //
    // Set the interrupt priority.
    //
    ulBitOffset   = (ulInterrupt & 0x03) * 8;

    if (ulInterrupt < 16 ) {
        pRegister = (UINT32*)((uint32)nvic->SystemPriority - 4);
    } else {
        pRegister = (UINT32 *)nvic->Irq.Priority;
        ulInterrupt = ulInterrupt - 16;
    }
    pRegister = pRegister + (ulInterrupt >> 2);

    *pRegister = *pRegister & (~(0xFF << ulBitOffset)) | (ucPriority << ulBitOffset);
}

/*
--------------------------------------------------------------------------------
  Function name :  UINT32 IntPriorityGet(UINT32 ulInterrupt)
  Author        :  ZHengYongzhi
  Description   :  Gets the priority of an interrupt.
                   This function gets the priority of an interrupt.
  Input         :  ulInterrupt -- INT_ID
  Return        :  Returns the interrupt priority.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


UINT32 IntPriorityGet2(UINT32 ulInterrupt)
{
    UINT32 ulBitOffset;
    UINT32 *pRegister;

    //
    // Return the interrupt priority.
    //
    ulBitOffset   = (ulInterrupt & 0x03) * 8;

    if (ulInterrupt < 16 ) {
        pRegister = (UINT32 *)(nvic->SystemPriority - 4);
    } else {
        pRegister = (UINT32 *)nvic->Irq.Priority;
    }
    pRegister = pRegister + (ulInterrupt & 0x03);

    return((*pRegister >> ulBitOffset) & 0xFF);
}

/*
--------------------------------------------------------------------------------
  Function name : void IntEnable(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : Enables an interrupt.
                  The specified interrupt is enabled in the interrupt controller.
  Input         : ulInterrupt -- INT_ID
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntEnable2(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt to enable.
    //
    if(ulInterrupt == FAULT_ID4_MPU)
    {
        //
        // Enable the MemManage interrupt.
        //
        nvic->SystemHandlerCtrlAndState |= NVIC_SYSHANDCTRL_MEMFAULTENA;
    }
    else if(ulInterrupt == FAULT_ID5_BUS)
    {
        //
        // Enable the bus fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState |= NVIC_SYSHANDCTRL_BUSFAULTENA;
    }
    else if(ulInterrupt == FAULT_ID6_USAGE)
    {
        //
        // Enable the usage fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState |= NVIC_SYSHANDCTRL_USGFAULTENA;
    }
    else if(ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Enable the System Tick interrupt.
        //
        nvic->SysTick.Ctrl |= NVIC_SYSTICKCTRL_TICKINT;
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Enable the general interrupt.
        //
        nvic->Irq.Enable[(ulInterrupt - 16) / 32] = 1 << (ulInterrupt - 16) % 32;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void IntDisable(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : Disables an interrupt.
                  The specified interrupt is Disabled in the interrupt controller.
  Input         : ulInterrupt -- INT_ID
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntDisable2(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt to Disable.
    //
    if(ulInterrupt == FAULT_ID4_MPU)
    {
        //
        // Disable the MemManage interrupt.
        //
        nvic->SystemHandlerCtrlAndState &= ~NVIC_SYSHANDCTRL_MEMFAULTENA;
    }
    else if(ulInterrupt == FAULT_ID5_BUS)
    {
        //
        // Disable the bus fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState &= ~NVIC_SYSHANDCTRL_BUSFAULTENA;
    }
    else if(ulInterrupt == FAULT_ID6_USAGE)
    {
        //
        // Disable the usage fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState &= ~NVIC_SYSHANDCTRL_USGFAULTENA;
    }
    else if(ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Disable the System Tick interrupt.
        //
        nvic->SysTick.Ctrl &= ~NVIC_SYSTICKCTRL_TICKINT;
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Disable the general interrupt.
        //
        nvic->Irq.Disable[(ulInterrupt - 16) / 32] = 1 << (ulInterrupt - 16) % 32;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void IntPendingSet(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : Set pending bit of the interrupt.

  Input         :
  Return        : Pending IRQ Channel Identifier.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntPendingSet2(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt pend to Set.
    //
    if(ulInterrupt == FAULT_ID2_NMI)
    {
        //
        // Set the NMI interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_NMIPENDSET;
    }
    else if(ulInterrupt == FAULT_ID14_PENDSV)
    {
        //
        // Set the PendSV interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSVSET;
    }
    else if(ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Set the System Tick interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSTSET;
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Set the general interrupt pend.
        //
        nvic->Irq.SetPend[(ulInterrupt - 16) / 32] = 1 << (ulInterrupt - 16) % 32;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void IntPendClear(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : Clear pending bit of the irq.

  Input         :
  Return        : Pending IRQ Channel Identifier.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


void IntPendingClear2(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt pend to Set.
    //
    if(ulInterrupt == FAULT_ID14_PENDSV)
    {
        //
        // Set the PendSV interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSVCLR;
    }
    else if(ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Set the System Tick interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSTCLR;
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Set the general interrupt pend.
        //
        nvic->Irq.ClearPend[(ulInterrupt - 16) / 32] = 1 << (ulInterrupt - 16) % 32;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : BOOL IntISRPendingCheck(void)
  Author        : ZHengYongzhi
  Description   : Check interrupt pending or not.

  Input         :
  Return        : pending or not.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


BOOL IntISRPendingCheck2(void)
{
    //
    // Check the arguments.
    //
    return ((BOOL)(nvic->INTcontrolState & NVIC_INTCTRLSTA_ISRPENDING));
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 IntVectPendingGet(void)
  Author        : ZHengYongzhi
  Description   : Get interrupt number of the highest priority pending ISR.

  Input         :
  Return        : Return interrupt number of the highest priority pending ISR.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

UINT32 IntVectPendingGet2(void)
{
    //
    // Check the arguments.
    //
    return ((nvic->INTcontrolState & NVIC_INTCTRLSTA_ISRPENDING) >> 12);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 IntVectActiveGet(void)
  Author        : ZHengYongzhi
  Description   : Get interrupt number of the currently running ISR.

  Input         :
  Return        : Return interrupt number of the currently running ISR.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


UINT32 IntVectActiveGet2(void)
{
    //
    // Check the arguments.
    //
    return (nvic->INTcontrolState & NVIC_INTCTRLSTA_VECTACTIVE_MASK);
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 IsrDisable(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : close interrupt temporarily,go to use with IsrEnable.

  Input         :
  Return        : current interrupt status.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


uint32 IsrDisable2(UINT32 ulInterrupt)
{
    uint32 IsEnable;

    //
    // Determine the interrupt to enable.
    //
    if(ulInterrupt == FAULT_ID4_MPU)
    {
        //
        // Is the MemManage interrupt Enabled.
        //
        IsEnable = (nvic->SystemHandlerCtrlAndState & NVIC_SYSHANDCTRL_MEMFAULTENA);
    }
    else if(ulInterrupt == FAULT_ID5_BUS)
    {
        //
        // Is the bus fault interrupt Enabled.
        //
        IsEnable = (nvic->SystemHandlerCtrlAndState & NVIC_SYSHANDCTRL_BUSFAULTENA);
    }
    else if(ulInterrupt == FAULT_ID6_USAGE)
    {
        //
        // Is the usage fault interrupt Enabled.
        //
        IsEnable = (nvic->SystemHandlerCtrlAndState & NVIC_SYSHANDCTRL_USGFAULTENA);
    }
    else if(ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Is the System Tick interrupt Enabled.
        //
        IsEnable = (nvic->SysTick.Ctrl & NVIC_SYSTICKCTRL_TICKINT);
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Is the general interrupt Enabled.
        //
        IsEnable = (nvic->Irq.Enable[(ulInterrupt - 16) / 32] & (1 << (ulInterrupt - 16) % 32));
    }

    IntDisable2(ulInterrupt);

    return(IsEnable);

}

/*
--------------------------------------------------------------------------------
  Function name : void IsrEnable(UINT32 ulInterrupt, uint32 IsEnable)
  Author        : ZHengYongzhi
  Description   : open interrupt and it act in concert with IsrDisable.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


void IsrEnable2(UINT32 ulInterrupt, uint32 IsEnable)
{
    if (IsEnable)
    {
        IntEnable2(ulInterrupt);
    }
}


/*
--------------------------------------------------------------------------------
  Function name : void UserIsrEnable(uint32 data)
  Author        : ZhengYongzhi
  Description   : PendSV作为软中断，不能进行Disable和Enable，使用信号量MSG_USER_ISR_ENABLE
                  进行PendSV的使能标志，和UserIsrDisable配合使用，打开时需要判断FAULT_ID14_PENDSV
                  的状态。使用这两个函数之前必须对信号量MSG_USER_ISR_ENABLE和MSG_USER_ISR_STATUES
                  进行初始化
  Input         :
  Return        : 无

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

uint32 IrqPriorityTab2[NUM_INTERRUPTS] =
{
    //  group        SubPriority                                 //Cortex M3 System Intterupt
    -3,                                 //SP Point                FAULT_ID0_REV,
    -3,                                 //Reset                   FAULT_ID1_REV,
    -2,                                 //NMI fault               FAULT_ID2_NMI,
    -1,                                 //Hard fault              FAULT_ID3_HARD,
    ((0x00 << 6) | (0x00 << 3)),        //MPU fault               FAULT_ID4_MPU,
    ((0x00 << 6) | (0x00 << 3)),        //Bus fault               FAULT_ID5_BUS,
    ((0x00 << 6) | (0x00 << 3)),        //Usage fault             FAULT_ID6_USAGE,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID7_REV,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID8_REV,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID9_REV,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID10_REV,
    ((0xFF << 6) | (0xFF << 3)),        //SVCall                  FAULT_ID11_SVCALL,
    ((0xFF << 6) | (0xFF << 3)),        //Debug monitor           FAULT_ID12_DEBUG,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID13_REV,
    ((0x03 << 6) | (0x00 << 3)),        //PendSV                  FAULT_ID14_PENDSV,
    ((0x02 << 6) | (0x01 << 3)),        //System Tick             FAULT_ID15_SYSTICK,

    //RKNano Process Int
    ((0x02 << 6) | (0x01 << 3)),        //00 sfc                  INT_ID16_SFC,
    ((0x01 << 6) | (0x01 << 3)),        //01 synth                INT_ID17_SYNTH,
    ((0x02 << 6) | (0x07 << 3)),        //02 ebc                  INT_ID18_EBC,
    ((0x02 << 6) | (0x00 << 3)),        //03 emmc                 INT_ID19_EMMC,
    ((0x02 << 6) | (0x00 << 3)),        //04 sdmmc                INT_ID20_SDMMC,
    ((0x02 << 6) | (0x00 << 3)),        //05 usbc                 INT_ID21_USBC,
    ((0x02 << 6) | (0x00 << 3)),        //06 dma                  INT_ID22_DMA,
    ((0x01 << 6) | (0x01 << 3)),        //07 imdct                INT_ID23_IMDCT,
    ((0x02 << 6) | (0x02 << 3)),        //08 wdt                  INT_ID24_WDT,
    ((0x00 << 6) | (0x00 << 3)),        //09 mailbox 0             INT_ID25_MAILBOX0,
    ((0x02 << 6) | (0x02 << 3)),        //10 mailbox 1            INT_ID26_MAILBOX1,
    ((0x02 << 6) | (0x02 << 3)),        //11 mailbox 2             INT_ID27_MAILBOX2,
    ((0x02 << 6) | (0x02 << 3)),        //12 mailbox 3             INT_ID28_MAILBOX3,
    ((0x02 << 6) | (0x04 << 3)),        //13 pwm4                 INT_ID29_PWM4,
    ((0x02 << 6) | (0x05 << 3)),        //14 pwm3                 INT_ID30_PWM3,
    ((0x02 << 6) | (0x06 << 3)),        //15 pwm2                 INT_ID31_PWM2,
    ((0x02 << 6) | (0x07 << 3)),        //16 pwm1                 INT_ID32_PWM1,
    ((0x01 << 6) | (0x02 << 3)),        //17 pwm0                 INT_ID33_PWM0,
    ((0xFF << 6) | (0xFF << 3)),        //18 timer 1               INT_ID34_TIMER1,
    ((0xFF << 6) | (0xFF << 3)),        //19 timer 0               INT_ID35_TIMER0,
    ((0xFF << 6) | (0xFF << 3)),        //20 sradc                INT_ID36_SRADC,
    ((0x02 << 6) | (0x07 << 3)),        //21 uart5                INT_ID37_UART5,
    ((0xFF << 6) | (0xFF << 3)),        //22 uart4                INT_ID38_UART4,
    ((0x02 << 6) | (0x07 << 3)),        //23 uart3                INT_ID39_UART3,
    ((0xFF << 6) | (0xFF << 3)),        //24 uart2                INT_ID40_UART2,
    ((0x02 << 6) | (0x07 << 3)),        //25 uart1                INT_ID41_UART1,
    ((0x02 << 6) | (0x07 << 3)),        //26 uart0                INT_ID42_UART0,
    ((0x02 << 6) | (0x07 << 3)),        //27 spi 1                  INT_ID43_SP1,
    ((0x02 << 6) | (0x07 << 3)),        //28 spi 0                  INT_ID44_SP0,
    ((0x02 << 6) | (0x07 << 3)),        //29 i2c2                 INT_ID45_I2C2,
    ((0x02 << 6) | (0x07 << 3)),        //30 i2c1                 INT_ID46_I2C1,
    ((0x02 << 6) | (0x07 << 3)),        //31 i2c0                 INT_ID47_I2C0,
    ((0x02 << 6) | (0x07 << 3)),        //32 i2s1                 INT_ID48_I2S1,
    ((0x02 << 6) | (0x07 << 3)),        //33 i2s0                 INT_ID49_I2S0,
    ((0xFF << 6) | (0xFF << 3)),        //34 hifi                 INT_ID50_HIFI,
    ((0xFF << 6) | (0xFF << 3)),        //35 pmu                  INT_ID51_PMU,
    ((0xFF << 6) | (0xFF << 3)),        //36 gpio2                INT_ID52_GPIO2,
    ((0xFF << 6) | (0xFF << 3)),        //37 gpio1                INT_ID53_GPIO1,
    ((0xFF << 6) | (0xFF << 3)),        //38 gpio0                INT_ID54_GPIO0,
    ((0x02 << 6) | (0x02 << 3)),        //39 VOP                  INT_ID55_VOP,
    ((0x01 << 6) | (0x00 << 3)),        //40 dma2                 INT_ID56_DMA2,
};

void IrqPriorityInit2(void)              //中断优先级初始化, 在IntPriorityGroupingSet执行后调用
{
    uint32 i;

    IntPriorityGroupingSet2(2);
    for (i = 4; i < NUM_INTERRUPTS; i++)
    {
        IntPrioritySet2(i, IrqPriorityTab2[i]);
    }
}

/*
********************************************************************************
*
*                         End of interrupt.c
*
********************************************************************************
*/


