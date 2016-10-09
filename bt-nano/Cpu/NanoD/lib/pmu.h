/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\pmu.h
* Owner: aaron.sun
* Date: 2015.11.16
* Time: 15:51:31
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.11.16     15:51:31   1.0
********************************************************************************************
*/
#ifndef __COMMON_DRIVER_PMU_PMU_H__
#define __COMMON_DRIVER_PMU_PMU_H__

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
#define _CPU_NANOD_LIB_PMU_COMMON_  __attribute__((section("cpu_nanod_lib_pmu_common")))
#define _CPU_NANOD_LIB_PMU_INIT_  __attribute__((section("cpu_nanod_lib_pmu_init")))
#define _CPU_NANOD_LIB_PMU_SHELL_  __attribute__((section("cpu_nanod_lib_pmu_shell")))


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
extern void PmuSetSysRegister(uint32 RegId, uint32 value);
extern uint32 PmuGetSysRegister(uint32 RegId);
extern void PmuSoftReset(uint32 module, uint32 reset);
extern void PmuGpio2NegedegeIntDisable(uint32 gpio);
extern void PmuGpio2NegedegeIntEnable(uint32 gpio);
extern void PmuGpio2PosedegeIntDisable(uint32 gpio);
extern void PmuGpio2PosedegeIntEnable(uint32 gpio);
extern void PmuIntDisable(uint32 intmode);
extern void PmuIntEnable(uint32 intmode);
extern void PmuOscPowerDown(void);
extern void PmuSelLowFreq(uint32 selpath);
extern void PmuUseLowFreqEn(void);
extern void PmuSetPowerMode(uint32 PowerMode);
extern BOOL PmuCheckPdLogicPowerDown(uint32  pdhigh);
extern void PmuPdLogicPowerDown(uint32 pdhigh);
extern void PmuGpioNegedegePlusWakeUpDisable(uint32 GpioNum);
extern void PmuGpioNegedegePlusWakeUpEnable(uint32 GpioNum);
extern void PmuGpioPosedegePlusWakeUpDisable(uint32 GpioNum);
extern void PmuGpioPosedegePlusWakeUpEnable(uint32 GpioNum);
extern void PmuGpioIntWakeUpDisable(void);
extern void PmuGpioIntWakeUpEnable(void);



#endif

