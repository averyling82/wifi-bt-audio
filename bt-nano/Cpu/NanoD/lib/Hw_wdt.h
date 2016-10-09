/*
********************************************************************************************
*
*        Copyright (c): 2016 - 2016 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\Hw_wdt.h
* Owner: aaron.sun
* Date: 2016.9.28
* Time: 10:03:05
* Version: 1.0
* Desc: wdt register define
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.9.28     10:03:05   1.0
********************************************************************************************
*/


#ifndef __CPU_NANOD_LIB_HW_WDT_H__
#define __CPU_NANOD_LIB_HW_WDT_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _CPU_NANOD_LIB_HW_WDT_COMMON_  __attribute__((section("cpu_nanod_lib_hw_wdt_common")))
#define _CPU_NANOD_LIB_HW_WDT_INIT_  __attribute__((section("cpu_nanod_lib_hw_wdt_init")))
#define _CPU_NANOD_LIB_HW_WDT_SHELL_  __attribute__((section("cpu_nanod_lib_hw_wdt_shell")))

typedef __packed struct _WATCHDOG_REG
{
    UINT32 WDT_CR;
    UINT32 WDT_TORR; /*This field is used to select the timeout period from
                       which the watchdog counter restarts. A change of the
                        timeout period takes effect only after the next
                        counter restart (kick).
                        The range of values available for a 32-bit watchdog*/
    UINT32 WDT_CCVR; /*Current counter value
                        This register, when read, is the current value of the
                        internal counter. This value is read coherently when
                        ever it is read*/
    UINT32 WDT_CRR;  /*This register is used to restart the WDT counter. As a
                        safety feature to prevent accidental restarts, the
                        value 0x76 must be written. A restart also clears the
                        WDT interrupt. Reading this register returns zero.*/

    UINT32 WDT_STAT; /*This register shows the interrupt status of the WDT.
                        1¡¯b1: Interrupt is active regardless of polarity.
                        1¡¯b0: Interrupt is inactive.*/
    UINT32 WDT_EOI;

}WDTReg_t, *pWDTReg_t;

#define Wdt  ((WDTReg_t *) WDT_BASE)

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
