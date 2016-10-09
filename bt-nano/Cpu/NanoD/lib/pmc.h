/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\pmc.h
* Owner: aaron.sun
* Date: 2015.10.9
* Time: 16:58:36
* Version: 1.0
* Desc: pmc control
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.9     16:58:36   1.0
********************************************************************************************
*/


#ifndef __CPU_NANOD_LIB_PMC_H__
#define __CPU_NANOD_LIB_PMC_H__

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

#define _CPU_NANOD_LIB_PMC_COMMON_  __attribute__((section("cpu_nanod_lib_pmc_common")))
#define _CPU_NANOD_LIB_PMC_INIT_  __attribute__((section("cpu_nanod_lib_pmc_init")))
#define _CPU_NANOD_LIB_PMC_SHELL_  __attribute__((section("cpu_nanod_lib_pmc_shell")))

#define SYS_RESET_RESTART  0
#define SYS_RESET_LOADER   1
#define SYS_RESET_MASKROM  2

#define SCU_DCOUT_100                   ((uint32)0x100)
#define SCU_DCOUT_120                   ((uint32)0x120)

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
extern rk_err_t VDD_VoltageAdjust_Set(uint32 level);
extern void System_Reset(uint32 direct);
extern void System_Power_Init(void);
extern void System_Power_On(BOOL Enable);



#endif
