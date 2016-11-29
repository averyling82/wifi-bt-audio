/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\pmc.c
* Owner: aaron.sun
* Date: 2015.10.9
* Time: 16:58:26
* Version: 1.0
* Desc: pmc control
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.9     16:58:26   1.0
********************************************************************************************
*/

#define __CPU_NANOD_LIB_PMC_C__
#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_PMC_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "typedef.h"
#include "driverinclude.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: VDD_VoltageAdjust_Set
** Input:uint32 level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.15
** Time: 10:04:18
*******************************************************************************/
_CPU_NANOD_LIB_PMC_COMMON_
COMMON API rk_err_t VDD_VoltageAdjust_Set(uint32 level)
{
    //TODO...
    if (level == SCU_DCOUT_120)
    {
        //PWM1 (VDD_CORE FB)
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3,GPIO_OUT);
        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_HIGH);
    }
    else if (level == SCU_DCOUT_100)
    {
        //PWM1 (VDD_CORE FB)
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3,GPIO_OUT);
        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_LOW);
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: System_Reset
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.16
** Time: 17:20:57
*******************************************************************************/
_CPU_NANOD_LIB_PMC_COMMON_
COMMON API void System_Reset(uint32 direct)
{
    uint32 temp, i;
    // N   a  n   o   C  R   e  b    o  o  t   F    l    a   g
    // 4e 61 6e 6f, 43 52 65 62, 6f 6f 74 46, 6c 61 67
    //RebootTag[0] = 0x6f6e614e;
    //RebootTag[1] = 0x62655243;
    //RebootTag[2] = 0x46746f6f;
    //RebootTag[3] = 0x0067616c;

    SetPllDefault();

    //disable all interrupt
    nvic->SysTick.Ctrl     &= ~NVIC_SYSTICKCTRL_TICKINT;
    for (i = 0; i < 32; i++)
    {
        nvic->Irq.Disable[i]    = 0xffffffff;
        nvic->Irq.ClearPend[i]  = 0xffffffff;
    }

    for (i = 0; i < 10; i++)
    {
        CRU->CRU_CLKGATE_CON[i] = 0xffff0000;
    }

    for (i = 0; i < 4; i++)
    {
        CRU->CRU_SOFTRST[i] = 0xffff0000;
    }

    #if 0
    for (i = 0; i < 4; i++)
    {
        Grf->GPIO_IO0MUX[i]  = 0xffff0000;
        Grf->GPIO_IO1MUX[i]  = 0xffff0000;
        Grf->GPIO_IO2MUX[i]  = 0xffff0000;

        Grf->GPIO_IO0PULL[i] = 0xffff0000;
        Grf->GPIO_IO1PULL[i] = 0xffff0000;
        Grf->GPIO_IO2PULL[i] = 0xffff0000;
    }

    {
        Gpio_SetPortDirec(GPIO_CH0, 0x00000000);
        Gpio_SetPortDirec(GPIO_CH1, 0x00000000);
        Gpio_SetPortDirec(GPIO_CH2, 0x00000000);
    }
    #endif

    nvic->VectorTableOffset = (UINT32)0x00000000;

    if (direct == SYS_RESET_MASKROM) //firmware update.
    {
        Grf_otgphy_suspend(0);
        Pmu_Reg->PMU_SYS_REG3 = (0x18BF|0xFFFF0000);
    }
    else if(direct == SYS_RESET_LOADER)
    {
        Pmu_Reg->PMU_SYS_REG3 = (0x18AF|0xFFFF0000);
    }

    Grf->GRF_INTER_CON0 = NOC_REMAP_MASK << 16 | NOC_REMAP_BOOT_ROM;
    nvic->APIntRst = 0x05fa0007;       //system reseet.

    while (1);
}

/*******************************************************************************
** Name: System_Power_Init
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.9
** Time: 17:24:27
*******************************************************************************/
_CPU_NANOD_LIB_PMC_COMMON_
COMMON API void System_Power_Init(void)
{
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin6,IOMUX_GPIO2A6_IO);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin6,GPIO_OUT);
}

/*******************************************************************************
** Name: System_Power_On
** Input:BOOL Enable
** Return: void
** Owner:aaron.sun
** Date: 2015.10.9
** Time: 17:23:42
*******************************************************************************/
_CPU_NANOD_LIB_PMC_COMMON_
COMMON API void System_Power_On(BOOL Enable)
{
    if(Enable)
    {
        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin6,GPIO_HIGH);
    }
    else
    {
        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin6,GPIO_LOW);
    }
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
