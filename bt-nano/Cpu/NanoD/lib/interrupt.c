/*
********************************************************************************
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
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
#define _IN_INTTERUPT_
#include "bspconfig.h"
#include <typedef.h>
#include <DriverInclude.h>
#include "rkos.h"
#include "Bsp.h"


typedef UINT32       (*p__MRS_MSP)(void);

extern int Main(void);
extern int PowerOn_Reset(void);
extern UINT32 Image$$AP_MAIN_STACK$$ZI$$Limit;


#if defined(__arm__) && defined(__ARMCC_VERSION)
__attribute__((used,section("APMainStack"))) __align(4) uint32 MainStack[512];
#if 0//def _ENABLE_WIFI_BLUETOOTH
__attribute__((used,section("IdleStack"))) __align(4) uint32 IdleStack[230];//JJJHHH 20161019 //IdleStack[250];
#else
__attribute__((used,section("IdleStack"))) __align(4) uint32 IdleStack[250];
#endif//_ENABLE_WIFI_BLUETOOTH
#elif defined(__arm__) && defined(__GNUC__)
__attribute__((section("APMainStack"))) uint32 MainStack[512] __attribute__((align(4)));
__attribute__((section("IdleStack"))) uint32 Stack[512] __attribute__((align(4)));
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
_ATTR_VECTTAB_AP_
ExecFunPtr exceptions_table[NUM_INTERRUPTS] =
{
    (ExecFunPtr)(&Image$$AP_MAIN_STACK$$ZI$$Limit),               //FAULT_ID0_REV = (UINT32)0,
    (ExecFunPtr)PowerOn_Reset,                                    //FAULT_ID1_REV,
    IntDefaultHandler,                                            //FAULT_ID2_NMI,
    IntDefaultHandler,                                            //FAULT_ID3_HARD,
    IntDefaultHandler,                                            //FAULT_ID4_MPU,
    IntDefaultHandler,                                            //FAULT_ID5_BUS,
    IntDefaultHandler,                                            //FAULT_ID6_USAGE,
    IntDefaultHandler,                                            //FAULT_ID7_REV,
    IntDefaultHandler,                                            //FAULT_ID8_REV,
    IntDefaultHandler,                                            //FAULT_ID9_REV,
    IntDefaultHandler,                                            //FAULT_ID10_REV,
    IntDefaultHandler,                                            //FAULT_ID11_SVCALL,
    IntDefaultHandler,                                            //FAULT_ID12_DEBUG,
    IntDefaultHandler,                                            //FAULT_ID13_REV,
    IntDefaultHandler,                                            //FAULT_ID14_PENDSV,
    IntDefaultHandler,                                            //FAULT_ID15_SYSTICK,

    //RKNano Process Intterupt                                    /*nanoD bsp interrupt number*/
    IntDefaultHandler,                                            //INT_ID_SFC,
    IntDefaultHandler,                                            //INT_ID_SYNTH,
    IntDefaultHandler,                                            //INT_ID_EBC,
    IntDefaultHandler,                                            //INT_ID_EMMC,
    IntDefaultHandler,                                            //INT_ID_SDMMC,
    IntDefaultHandler,                                            //INT_ID_USBC,
    IntDefaultHandler,                                            //INT_ID_DMA,
    IntDefaultHandler,                                            //INT_ID_IMDCT,
    IntDefaultHandler,                                            //INT_ID_WDT,
    IntDefaultHandler,                                            //INT_ID_MAILBOX0,
    IntDefaultHandler,                                            //INT_ID_MAILBOX1,
    IntDefaultHandler,                                            //INT_ID_MAILBOX2,
    IntDefaultHandler,                                            //INT_ID_MAILBOX3,
    IntDefaultHandler,                                            //INT_ID_PWM4,
    IntDefaultHandler,                                            //INT_ID_PWM3,
    IntDefaultHandler,                                            //INT_ID_PWM2,
    IntDefaultHandler,                                            //INT_ID_PWM1,
    IntDefaultHandler,                                            //INT_ID_PWM0,
    IntDefaultHandler,                                            //INT_ID_TIMER1,
    IntDefaultHandler,                                            //INT_ID_TIMER0,
    IntDefaultHandler,                                            //INT_ID_SRADC,
    IntDefaultHandler,                                            //INT_ID_UART5,
    IntDefaultHandler,                                            //INT_ID_UART4,
    IntDefaultHandler,                                            //INT_ID_UART3,
    IntDefaultHandler,                                            //INT_ID_UART2,
    IntDefaultHandler,                                            //INT_ID_UART1,
    IntDefaultHandler,                                            //INT_ID_UART0,
    IntDefaultHandler,                                            //INT_ID_SPI1,
    IntDefaultHandler,                                            //INT_ID_SPI0,
    IntDefaultHandler,                                            //INT_ID_I2C2,
    IntDefaultHandler,                                            //INT_ID_I2C1,
    IntDefaultHandler,                                            //INT_ID_I2C0,
    IntDefaultHandler,                                            //INT_ID_I2S1,
    IntDefaultHandler,                                            //INT_ID_I2S0,
    IntDefaultHandler,                                            //INT_ID_HIFI,
    IntDefaultHandler,                                            //INT_ID_PMU,
    IntDefaultHandler,                                            //INT_ID_GPIO2,
    IntDefaultHandler,                                            //INT_ID_GPIO1,
    IntDefaultHandler,                                            //INT_ID_GPIO0,
    IntDefaultHandler,                                            //INT_ID_VOP,
    IntDefaultHandler,                                            //INT_ID_DMA2,
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
__asm uint32 __MSR_XPSR(void)
{
    MRS     R0, XPSR
    BX      R14
}
#elif defined(__arm__) && defined(__GNUC__)
uint32 __MSR_XPSR(void)
{
__asm__(
     "MRS     R0, XPSR\n\t"
     "BX      R14\n\t"
);
}
#else
#error Unknow compiling tools.
#endif


static uint8* TaskState[] =
{
    "Ru",
    "Re",
    "B",
    "S",
    "D",
    "\b"
};

void IntDefaultHandler_displaytasklist (void)
{
    RK_TASK_CLASS * TempTaskHandler;

    //printf("\nMspSave =  0x%08x", MspSave);

    {
        uint32 i;
        printf("\nnvic->INTcontrolState = 0x%08x", nvic->INTcontrolState);
        for(i = 0; i < 32; i++)
        {
            printf("\nnvic->Irq.SetPend[i] = 0x%08x", nvic->Irq.SetPend[i]);
        }
    }

    TempTaskHandler = RKTaskGetFirstHandle(0xffffffff);

    printf("\n");

    while (TempTaskHandler != NULL)
    {
        printf("%s\t",TaskState[RKTaskState(TempTaskHandler)]);
        printf("%s\t\t\t",RKTaskGetTaskName((HTC)TempTaskHandler));
        printf("%x\t\t\t", RKTaskStackRemain((HTC)TempTaskHandler));
        printf("%d\n", TempTaskHandler->TotalMemory);

        TempTaskHandler = RKTaskGetNextHandle(TempTaskHandler, 0xffffffff);
    }


    #ifdef _MEMORY_LEAK_CHECH_
    MEMORY_BLOCK * pCur;
    uint32 totalsize, i;

    i = 0;
    totalsize = 0;

    pCur = pFirstMemoryBlock;

    printf("\n\n");

    while(pCur != NULL)
    {

         totalsize += pCur->size;

         if(pCur->flag != 0xaa55aa55)
         {
            break;
         }

         if(*(uint32*)((uint8 *)pCur + pCur->size + sizeof(MEMORY_BLOCK)) != 0x55aa55aa)
         {
            break;
         }


         printf("%d\t\t\t", pCur->size);
         printf("0x%08x\t\t", (uint8 *)pCur + sizeof(MEMORY_BLOCK));

         if(pCur->hTask == NULL)
         {
            printf("%s\n", "deleted");
         }
         else
         {
            printf("%s\n", pcTaskGetTaskName((void *)pCur->hTask));
         }

         i++;
         pCur = pCur->pNext;
    }

    printf("    total used memory block cnt = %d, totalsize = %d,  Remaining = %d\n", UsedMemoryCnt, totalsize, RKTaskHeapFree());

    if(i != UsedMemoryCnt)
    {
        printf("heap destoried in 0x%08x by %s used\n", (uint8 *)pCur + sizeof(MEMORY_BLOCK), "unknown");
    }
    #endif

    Grf_NOC_Remap_Sel(NOC_REMAP_BOOT_ROM);
    while(1);
}

#if defined(__arm__) && defined(__ARMCC_VERSION)

__asm __irq void IntDefaultHandler(void)
{
    IMPORT IntDefaultHandler_displaytasklist
    PRESERVE8

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'B'
    BL UART_SEND_BYTE
    MOV R0, #'A'
    BL UART_SEND_BYTE
    MOV R0, #'S'
    BL UART_SEND_BYTE
    MOV R0, #'E'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'R'
    BL UART_SEND_BYTE
    MOV R0, #'I'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE

    MRS R4, BASEPRI
    BL PRINT_REG

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'I'
    BL UART_SEND_BYTE
    MOV R0, #'N'
    BL UART_SEND_BYTE
    MOV R0, #'T'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE

    MRS R4, IPSR
    BL PRINT_REG

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'H'
    BL UART_SEND_BYTE
    MOV R0, #'F'
    BL UART_SEND_BYTE
    MOV R0, #'S'
    BL UART_SEND_BYTE
    MOV R0, #'R'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE

    MOV R0, #0XED2C ;HFSR
    MOV R1, #0XE000
    LSL R1, #16
    ORR R0, R1
    LDR R4, [R0]
    BL PRINT_REG

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'B'
    BL UART_SEND_BYTE
    MOV R0, #'F'
    BL UART_SEND_BYTE
    MOV R0, #'S'
    BL UART_SEND_BYTE
    MOV R0, #'R'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE

    MOV R0, #0XED29 ;HFSR
    MOV R1, #0XE000
    LSL R1, #16
    ORR R0, R1
    LDR R4, [R0]
    BL PRINT_REG

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'M'
    BL UART_SEND_BYTE
    MOV R0, #'S'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'-'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'C'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE

    MRS R6, MSP
    LDR R4, [R6, #0x18]
    BL PRINT_REG

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'M'
    BL UART_SEND_BYTE
    MOV R0, #'S'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'-'
    BL UART_SEND_BYTE
    MOV R0, #'L'
    BL UART_SEND_BYTE
    MOV R0, #'R'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE

    MRS R6, MSP
    LDR R4, [R6, #0x14]
    BL PRINT_REG

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'M'
    BL UART_SEND_BYTE
    MOV R0, #'S'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE

    MRS R4, MSP
    BL PRINT_REG

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'S'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'-'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'C'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE

    MRS R6, PSP
    LDR R4, [R6, #0x18]
    BL PRINT_REG

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'S'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'-'
    BL UART_SEND_BYTE
    MOV R0, #'L'
    BL UART_SEND_BYTE
    MOV R0, #'R'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE

    MRS R6, PSP
    LDR R4, [R6, #0x14]
    BL PRINT_REG

    MOV R0, #'\r'
    BL UART_SEND_BYTE
    MOV R0, #'\n'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #'S'
    BL UART_SEND_BYTE
    MOV R0, #'P'
    BL UART_SEND_BYTE
    MOV R0, #':'
    BL UART_SEND_BYTE
    MRS R4, PSP
    BL PRINT_REG

    BL IntDefaultHandler_displaytasklist
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
#elif defined(__arm__) && defined(__GNUC__)
void IntDefaultHandler(void)
{
__asm__(
    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'B'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'A'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'S'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'E'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'R'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'I'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R4, BASEPRI\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'I'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'N'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'T'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R4, IPSR\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'H'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'F'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'S'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'R'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "LDR R0,=#0XED2C\n\t"//;HFSR   //LYJ
    "MOV R1, #0XE000\n\t"
    "LSL R1, #16\n\t"
    "ORR R0, R1\n\t"
    "LDR R4, [R0]\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'B'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'F'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'S'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'R'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "LDR R0,=#0XED29\n\t"//;HFSR//LYJ
    "MOV R1, #0XE000\n\t"
    "LSL R1, #16\n\t"
    "ORR R0, R1\n\t"
    "LDR R4, [R0]\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'M'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'S'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'-'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'C'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R6, MSP\n\t"
    "LDR R4, [R6, #0x18]\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'M'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'S'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'-'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'L'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'R'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R6, MSP\n\t"
    "LDR R4, [R6, #0x14]\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'M'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'S'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R4, MSP\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'S'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'-'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'C'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R6, PSP\n\t"
    "LDR R4, [R6, #0x18]\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'S'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'-'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'L'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'R'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R6, PSP\n\t"
    "LDR R4, [R6, #0x14]\n\t"
    "BL PRINT_REG\n\t"

    "MOV R0, #'\r'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'\n'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'S'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #'P'\n\t"
    "BL UART_SEND_BYTE\n\t"
    "MOV R0, #':'\n\t"
    "BL UART_SEND_BYTE\n\t"

    "MRS R4, PSP\n\t"
    "BL PRINT_REG\n\t"

    "HERE:B  HERE\n\t"
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

    "UART_SEND_BYTE:LDR   R1, =0x400b0000 \n\t"//;UART
    "LDR   R2, [R1, #0x7C] \n\t"//;USR
    "MOV   R3, #0X02\n\t"
    "AND   R2, R3\n\t"
    "CBNZ   R2, UART_SEND_BYTE_END\n\t"
    "B UART_SEND_BYTE\n\t"
    "UART_SEND_BYTE_END:STR  R0, [R1]  \n\t"//;THR
    "BX LR\n\t"
);
}
#else
#error Unknow compiling tools.
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

void IntMasterEnable(void)
{
    __SETPRIMASK();
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

void IntMasterDisable(void)
{
    __RESETPRIMASK();
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

void FaultMasterEnable(void)
{
    __RESETFAULTMASK();
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

void FaultMasterDisable(void)
{
    __SETFAULTMASK();
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

void IntRegister(UINT32 ulInterrupt, ExecFunPtr(*pfnHandler)(void))
{
    UINT32 ulIdx;

    // Check the arguments.
//    ASSERT(ulInterrupt < NUM_INTERRUPTS);
//
//    // Make sure that the RAM vector table is correctly aligned.
//    ASSERT(((UINT32)exceptions_table & 0x000003ff) == 0);

    // See if the RAM vector table has been initialized.
    if (nvic->VectorTableOffset != (UINT32)exceptions_table)
    {
        // Initiale the RAM vector table.
        exceptions_table[0] = (ExecFunPtr)&Image$$AP_MAIN_STACK$$ZI$$Limit;
        exceptions_table[1] = (ExecFunPtr)Main;
        for (ulIdx = 2; ulIdx < NUM_INTERRUPTS; ulIdx++)
        {
            exceptions_table[ulIdx] = (ExecFunPtr)IntDefaultHandler;
        }

        // Point NVIC at the RAM vector table.
        nvic->VectorTableOffset = (UINT32)exceptions_table;
    }


    // Save the interrupt handler.
    exceptions_table[ulInterrupt] = (ExecFunPtr)pfnHandler;
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

void IntUnregister(UINT32 ulInterrupt)
{
    //
    // Check the arguments.
    //
//    ASSERT(ulInterrupt < NUM_INTERRUPTS);

    //
    // Reset the interrupt handler.
    //
    exceptions_table[ulInterrupt] = (ExecFunPtr)IntDefaultHandler;
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

void IntPriorityGroupingSet(UINT32 ulBits)
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
_ATTR_SYS_INIT_CODE_
UINT32 IntPriorityGroupingGet(void)
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

void IntPrioritySet(UINT32 ulInterrupt, UINT32 ucPriority)
{
    UINT32 ulBitOffset;
    UINT32 *pRegister;

    //
    // Set the interrupt priority.
    //
    ulBitOffset   = (ulInterrupt & 0x03) * 8;

    if (ulInterrupt < 16 )
    {
        pRegister = (UINT32*)((uint32)nvic->SystemPriority - 4);
    }
    else
    {
        pRegister = (UINT32 *)nvic->Irq.Priority;
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


UINT32 IntPriorityGet(UINT32 ulInterrupt)
{
    UINT32 ulBitOffset;
    UINT32 *pRegister;

    //
    // Return the interrupt priority.
    //
    ulBitOffset   = (ulInterrupt & 0x03) * 8;

    if (ulInterrupt < 16 )
    {
        pRegister = (UINT32 *)(nvic->SystemPriority - 4);
    }
    else
    {
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

void IntEnable(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt to enable.
    //
    if (ulInterrupt == FAULT_ID4_MPU)
    {
        //
        // Enable the MemManage interrupt.
        //
        nvic->SystemHandlerCtrlAndState |= NVIC_SYSHANDCTRL_MEMFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID5_BUS)
    {
        //
        // Enable the bus fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState |= NVIC_SYSHANDCTRL_BUSFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID6_USAGE)
    {
        //
        // Enable the usage fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState |= NVIC_SYSHANDCTRL_USGFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
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

void IntDisable(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt to Disable.
    //
    if (ulInterrupt == FAULT_ID4_MPU)
    {
        //
        // Disable the MemManage interrupt.
        //
        nvic->SystemHandlerCtrlAndState &= ~NVIC_SYSHANDCTRL_MEMFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID5_BUS)
    {
        //
        // Disable the bus fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState &= ~NVIC_SYSHANDCTRL_BUSFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID6_USAGE)
    {
        //
        // Disable the usage fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState &= ~NVIC_SYSHANDCTRL_USGFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
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

void IntPendingSet(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt pend to Set.
    //
    if (ulInterrupt == FAULT_ID2_NMI)
    {
        //
        // Set the NMI interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_NMIPENDSET;
    }
    else if (ulInterrupt == FAULT_ID14_PENDSV)
    {
        //
        // Set the PendSV interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSVSET;
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
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


void IntPendingClear(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt pend to Set.
    //
    if (ulInterrupt == FAULT_ID14_PENDSV)
    {
        //
        // Set the PendSV interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSVCLR;
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
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


BOOL IntISRPendingCheck(void)
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

UINT32 IntVectPendingGet(void)
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


UINT32 IntVectActiveGet(void)
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


uint32 IsrDisable(UINT32 ulInterrupt)
{
    uint32 IsEnable;

    //
    // Determine the interrupt to enable.
    //
    if (ulInterrupt == FAULT_ID4_MPU)
    {
        //
        // Is the MemManage interrupt Enabled.
        //
        IsEnable = (nvic->SystemHandlerCtrlAndState & NVIC_SYSHANDCTRL_MEMFAULTENA);
    }
    else if (ulInterrupt == FAULT_ID5_BUS)
    {
        //
        // Is the bus fault interrupt Enabled.
        //
        IsEnable = (nvic->SystemHandlerCtrlAndState & NVIC_SYSHANDCTRL_BUSFAULTENA);
    }
    else if (ulInterrupt == FAULT_ID6_USAGE)
    {
        //
        // Is the usage fault interrupt Enabled.
        //
        IsEnable = (nvic->SystemHandlerCtrlAndState & NVIC_SYSHANDCTRL_USGFAULTENA);
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
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

    IntDisable(ulInterrupt);

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


void IsrEnable(UINT32 ulInterrupt, uint32 IsEnable)
{
    if (IsEnable)
    {
        IntEnable(ulInterrupt);
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
_ATTR_SYS_INIT_CODE_
uint32 IrqPriorityTab[NUM_INTERRUPTS] =
{
   //  group        SubPriority
   -3,                                      //FAULT_ID0_REV = (UINT
   -3,                                      //FAULT_ID1_REV,
   -2,                                      //FAULT_ID2_NMI,
   -1,                                      //FAULT_ID3_HARD,
   ((0x00 << 6) | (0x00 << 3)),             //FAULT_ID4_MPU,
   ((0x00 << 6) | (0x00 << 3)),             //FAULT_ID5_BUS,
   ((0x00 << 6) | (0x00 << 3)),             //FAULT_ID6_USAGE,
   ((0x00 << 6) | (0x01 << 3)),             //FAULT_ID7_REV,
   ((0xFF << 6) | (0xFF << 3)),             //FAULT_ID8_REV,
   ((0xFF << 6) | (0xFF << 3)),             //FAULT_ID9_REV,
   ((0xFF << 6) | (0xFF << 3)),             //FAULT_ID10_REV,
   ((0xFF << 6) | (0xFF << 3)),             //FAULT_ID11_SVCALL,
   ((0xFF << 6) | (0xFF << 3)),             //FAULT_ID12_DEBUG,
   ((0xFF << 6) | (0xFF << 3)),             //FAULT_ID13_REV,
   ((0x03 << 6) | (0x00 << 3)),             //FAULT_ID14_PENDSV,
   ((0x00 << 6) | (0x00 << 3)),             //FAULT_ID15_SYSTICK,

   //RKNano Process Int                     /*nanoD bsp interrupt n
   ((0x02 << 6) | (0x01 << 3)),             //INT_ID_SFC,
   ((0x02 << 6) | (0x00 << 3)),             //INT_ID_SYNTH,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_EBC,
   ((0x02 << 6) | (0x00 << 3)),             //INT_ID_EMMC,
   ((0x02 << 6) | (0x00 << 3)),             //INT_ID_SDMMC,
   ((0x02 << 6) | (0x00 << 3)),             //INT_ID_USBC,
   ((0x00 << 6) | (0x01 << 3)),             //INT_ID_DMA,
   ((0x01 << 6) | (0x01 << 3)),             //INT_ID_IMDCT,
   ((0x02 << 6) | (0x02 << 3)),             //INT_ID_WDT,
   ((0x02 << 6) | (0x03 << 3)),             //INT_ID_MAILBOX0,
   ((0x02 << 6) | (0x03 << 3)),             //INT_ID_MAILBOX1,
   ((0x02 << 6) | (0x03 << 3)),             //INT_ID_MAILBOX2,
   ((0x02 << 6) | (0x03 << 3)),             //INT_ID_MAILBOX3,
   ((0x00 << 6) | (0x02 << 3)),             //INT_ID_PWM4,
   ((0x02 << 6) | (0x05 << 3)),             //INT_ID_PWM3,
   ((0x02 << 6) | (0x06 << 3)),             //INT_ID_PWM2,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_PWM1,
   ((0x01 << 6) | (0x02 << 3)),             //INT_ID_PWM0,
   ((0xFF << 6) | (0xFF << 3)),             //INT_ID_TIMER1,
   ((0xFF << 6) | (0xFF << 3)),             //INT_ID_TIMER0,
   ((0xFF << 6) | (0xFF << 3)),             //INT_ID_SRADC,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_UART5,
   ((0xFF << 6) | (0xFF << 3)),             //INT_ID_UART4,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_UART3,
   ((0xFF << 6) | (0xFF << 3)),             //INT_ID_UART2,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_UART1,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_UART0,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_SPI1,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_SPI0,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_I2C2,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_I2C1,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_I2C0,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_I2S1,
   ((0x02 << 6) | (0x07 << 3)),             //INT_ID_I2S0,
   ((0xFF << 6) | (0xFF << 3)),             //INT_ID_HIFI,
   ((0xFF << 6) | (0xFF << 3)),             //INT_ID_PMU,
   ((0x02 << 6) | (0x00 << 3)),             //INT_ID_GPIO2,
   ((0x02 << 6) | (0x00 << 3)),             //INT_ID_GPIO1,
   ((0x02 << 6) | (0x00 << 3)),             //INT_ID_GPIO0,
   ((0xFF << 6) | (0xFF << 3)),             //INT_ID_VOP,
   ((0xFF << 6) | (0xFF << 3)),             //INT_ID_DMA2,
};

_ATTR_SYS_INIT_CODE_
void IrqPriorityInit(void)              //中断优先级初始化, 在IntPriorityGroupingSet执行后调用
{
    uint32 i;

    IntPriorityGroupingSet(2);
    for (i = 4; i < NUM_INTERRUPTS; i++)
    {
        IntPrioritySet(i, IrqPriorityTab[i]);
    }
}

/*
********************************************************************************
*
*                         End of interrupt.c
*
********************************************************************************
*/


