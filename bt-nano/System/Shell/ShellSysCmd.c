/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\Shell\ShellSysCmd.c
* Owner: aaron.sun
* Date: 2015.10.20
* Time: 9:38:01
* Version: 1.0
* Desc: rkos system cmd
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.20     9:38:01   1.0
********************************************************************************************
*/
#include "BspConfig.h"
#ifdef __SYSTEM_SHELL_SHELLSYSCMD_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "rkos.h"
#include "bsp.h"
#include "ShellSysCmd.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define EFFECT_TEST_NUM 0X0000ffff
#define EFFECT_TEST_BASE_NUM 2000


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t ShellSystemIdle1(HDC dev, uint8 * pstr);
rk_err_t ShellSystemFault(HDC dev, uint8 * pstr);
rk_err_t ShellSystemReset(HDC dev, uint8 * pstr);
rk_err_t ShellPowerOff(HDC dev, uint8 * pstr);
rk_err_t ShellSystemFreq(HDC dev, uint8 * pstr);
rk_err_t ShellMemoryLeakStop(HDC dev, char * pstr);
rk_err_t ShellMemoryLeakWatch(HDC dev, char * pstr);
rk_err_t ShellMemoryLeakstart(HDC dev, char * pstr);
rk_err_t ShellMemory(HDC dev,  char * pstr);
rk_err_t ShellRkosIdleTest(HDC dev,  char * pstr);
rk_err_t ShellCpuTest(HDC dev,  char * pstr);
void RkosEffectTestReport(void);
void RkosEffectTest1(void);
void MathCapacity(void);
void RkosEffectTest2(void);
void MemoryReadWriteTest(void);
void RkosEffectTest3(void);
void Filt(int* in, int len);
rk_err_t ShellBsp(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTest_DMA_LLP_RW(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTest_DMA_RW(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestPMUSRAM_RW(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestHIRAM_RW(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestHDRAM_RW(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestDRAM_RW(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestIRAM_RW(HDC dev, uint8 * pstr);
rk_err_t ShellBspMailBoxTestB2A(HDC dev, uint8 * pstr);
rk_err_t ShellBspMailBoxTestA2B(HDC dev, uint8 * pstr);
rk_err_t ShellBspMailBoxReadCmd(HDC dev, uint8 * pstr);
rk_err_t ShellBspMailBoxReadData(HDC dev, uint8 * pstr);
rk_err_t ShellBspMailBoxWriteCmd(HDC dev, uint8 * pstr);
rk_err_t ShellBspMailBoxWriteData(HDC dev, uint8 * pstr);
rk_err_t ShellBspGpioTestInt(HDC dev, uint8 * pstr);
rk_err_t ShellBspGpioTestPull(HDC dev, uint8 * pstr);
rk_err_t ShellBspGpioTestInOut(HDC dev, uint8 * pstr);
rk_err_t ShellBspGpioSetPull(HDC dev, uint8 * pstr);
rk_err_t ShellBspGpioSetLow(HDC dev, uint8 * pstr);
rk_err_t ShellBspGpioSetOut(HDC dev, uint8 * pstr);
rk_err_t ShellBspGpioSetHigh(HDC dev, uint8 * pstr);
rk_err_t ShellBspGpioSetIn(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestEffect(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestConsumption(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestReadWrite(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestClk(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryTestPower(HDC dev, uint8 * pstr);
rk_err_t ShellBspNvicResumeNanoD(HDC dev, uint8 * pstr);
rk_err_t ShellBspNvicResumeM3(HDC dev, uint8 * pstr);
rk_err_t ShellBspNvicTestPriority(HDC dev, uint8 * pstr);
rk_err_t ShellBspNvicTestInt(HDC dev, uint8 * pstr);
rk_err_t ShellBspNvicSetMask(HDC dev, uint8 * pstr);
rk_err_t ShellBspNvicSetPriority(HDC dev, uint8 * pstr);
rk_err_t ShellBspNvicSetInt(HDC dev, uint8 * pstr);
rk_err_t ShellBspMailBoxHelp(HDC dev, uint8 * pstr);
rk_err_t ShellBspGpioHelp(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemoryHelp(HDC dev, uint8 * pstr);
rk_err_t ShellBspNvicHelp(HDC dev, uint8 * pstr);
rk_err_t ShellBspHelp(HDC dev, uint8 * pstr);
rk_err_t ShellBspMailBox(HDC dev, uint8 * pstr);
rk_err_t ShellBspNvic(HDC dev, uint8 * pstr);
rk_err_t ShellBspGPIO(HDC dev, uint8 * pstr);
rk_err_t ShellBspMemory(HDC dev, uint8 * pstr);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static int sCoefs[32] =
{
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,0 , 0 , 0,
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,0 , 0 , 0

};

static int tempbuf[EFFECT_TEST_BASE_NUM];

static uint32 IdleTime;
static uint32 IdleCnt;
static uint32 SysTick;
static uint32 temp;

_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
static SHELL_CMD ShellSystemName[] =
{
    "memory",ShellMemory,"count system heap memory","system.memory",
    "cpu",ShellCpuTest,"test cpu excute speed","system.cpu [/1 | /2]",
    "rkos",ShellRkosIdleTest,"test rkos speed","system.rkos",
    "poweroff",ShellPowerOff,"system power off","system.poweroff",
    "freq",ShellSystemFreq,"display system freque","system.freq",
    "reset",ShellSystemReset,"reset system","system.reset",
    "fault",ShellSystemFault,"fault system","system.fault",
    "idle1",ShellSystemIdle1,"set system idle1 event time","system.idle1 <time>",
    "\b",NULL,"NULL","NULL",
};


#ifdef SHELL_BSP
_SYSTEM_SHELL_SHELLBSP_READ_
static SHELL_CMD ShellBspName[] =
{
    "memory",ShellBspMemory,"NULL","NULL",
    "gpio",ShellBspGPIO,"NULL","NULL",
    "nvic",ShellBspNvic,"NULL","NULL",
    "mailbox",ShellBspMailBox,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

_SYSTEM_SHELL_SHELLBSP_READ_
static SHELL_CMD ShellBspNvicName[] =
{
    "setint",ShellBspNvicSetInt,"NULL","NULL",
    "setpriority",ShellBspNvicSetPriority,"NULL","NULL",
    "setmask",ShellBspNvicSetMask,"NULL","NULL",
    "testint",ShellBspNvicTestInt,"NULL","NULL",
    "testpriority",ShellBspNvicTestPriority,"NULL","NULL",
    "resumem3",ShellBspNvicResumeM3,"NULL","NULL",
    "resumenanod",ShellBspNvicResumeNanoD,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

_SYSTEM_SHELL_SHELLBSP_READ_
static SHELL_CMD ShellBspMemoryName[] =
{
    "testpower",ShellBspMemoryTestPower,"NULL","NULL",
    "testclk",ShellBspMemoryTestClk,"NULL","NULL",
    "testrw",ShellBspMemoryTestReadWrite,"NULL","NULL",
    "testconsumption",ShellBspMemoryTestConsumption,"NULL","NULL",
    "testeffect",ShellBspMemoryTestEffect,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

_SYSTEM_SHELL_SHELLBSP_READ_
static SHELL_CMD ShellBspMemoryRAMName[] =
{
    "iram",ShellBspMemoryTestIRAM_RW,"NULL","NULL",
    "dram",ShellBspMemoryTestDRAM_RW,"NULL","NULL",
    "hiram",ShellBspMemoryTestHIRAM_RW,"NULL","NULL",
    "hdram",ShellBspMemoryTestHDRAM_RW,"NULL","NULL",
    "pmusarm",ShellBspMemoryTestPMUSRAM_RW,"NULL","NULL",
    "dma",ShellBspMemoryTest_DMA_RW,"NULL","NULL",
    "dmallp",ShellBspMemoryTest_DMA_LLP_RW,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

_SYSTEM_SHELL_SHELLBSP_READ_
static SHELL_CMD ShellBspGpioName[] =
{
    "help",NULL,"NULL","NULL",
    "setin",NULL,"NULL","NULL",
    "setout",NULL,"NULL","NULL",
    "sethigh",NULL,"NULL","NULL",
    "setlow",NULL,"NULL","NULL",
    "setpull",NULL,"NULL","NULL",
    "testinout",NULL,"NULL","NULL",
    "testpull",NULL,"NULL","NULL",
    "testint",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

_SYSTEM_SHELL_SHELLBSP_READ_
static SHELL_CMD ShellBspMailBoxName[] =
{
    "writedata",ShellBspMailBoxWriteData,"NULL","NULL",
    "writecmd",ShellBspMailBoxWriteCmd,"NULL","NULL",
    "readdata",ShellBspMailBoxReadData,"NULL","NULL",
    "readcmd",ShellBspMailBoxReadCmd,"NULL","NULL",
    "testa2b",ShellBspMailBoxTestA2B,"NULL","NULL",
    "testb2a",ShellBspMailBoxTestB2A,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

#endif

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
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ShellSystemParsing
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 10:29:14
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON API rk_err_t ShellSystemParsing(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellSystemName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellSystemName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellSystemName[i].CmdDes, pItem);
    if(ShellSystemName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellSystemName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ShellSystemIdle1
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.11.8
** Time: 19:33:57
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN rk_err_t ShellSystemIdle1(HDC dev, uint8 * pstr)
{
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    uint32 Idle1EventTime;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != ' ') || (Space != '\0'))
    {
        return RK_ERROR;
    }

    Idle1EventTime = String2Num(pstr);

    rk_printf("system idle1 event time = %d s", Idle1EventTime);

    gSysConfig.SysIdle1EventTime = Idle1EventTime * 1000;

    return RK_SUCCESS;

}


/*******************************************************************************
** Name: ShellSystemFault
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.10.10
** Time: 16:48:44
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN rk_err_t ShellSystemFault(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    IntPendingSet(2);
    while(1);
}
/*******************************************************************************
** Name: ShellSystemReset
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.9.8
** Time: 15:13:16
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN rk_err_t ShellSystemReset(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    PmuSetSysRegister(0, 0);
    DeviceTask_SystemReset(0);
}
/*******************************************************************************
** Name: ShellSystemFreq
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.12.11
** Time: 14:12:22
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN rk_err_t ShellSystemFreq(HDC dev, uint8 * pstr)
{
    chip_freq_t  * pchip_freq;
    uint32 i;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    rk_printf_no_time("freq module list");
    printf("\n");
    for(i = 0; i < 64; i++)
    {
        if(g_APPList & (0x01 << i))
        {
            printf("%d ", i);
        }
    }

    pchip_freq = &chip_freq;

    rk_printf_no_time(".chip_freq");
    rk_printf_no_time("    .pll = %d", pchip_freq->pll);
    rk_printf_no_time("    .stclk_sys_core = %d",pchip_freq->stclk_sys_core);
    rk_printf_no_time("    .hclk_sys_core = %d",pchip_freq->hclk_sys_core);
    rk_printf_no_time("    .fclk_sys_core = %d",pchip_freq->fclk_sys_core);
    rk_printf_no_time("    .stclk_cal_core = %d",pchip_freq->stclk_cal_core);
    rk_printf_no_time("    .hclk_cal_core = %d",pchip_freq->hclk_cal_core);
    rk_printf_no_time("    .fclk_cal_core = %d",pchip_freq->fclk_cal_core);
    rk_printf_no_time("    .pclk_logic_pre = %d",pchip_freq->pclk_logic_pre);
    rk_printf_no_time("    .hclk_logic_pre = %d",pchip_freq->hclk_logic_pre);
    rk_printf_no_time("    .pclk_pmu_pre = %d",pchip_freq->pclk_pmu_pre);
    rk_printf_no_time("    .hclk_pmu_pre = %d",pchip_freq->hclk_pmu_pre);
    rk_printf_no_time("    .hclk_high_pre = %d",pchip_freq->hclk_high_pre);
    rk_printf_no_time("    .usbphy_clk = %d",pchip_freq->usbphy_clk);
    rk_printf_no_time("    .i2s_clk_out = %d",pchip_freq->i2s_clk_out);
    rk_printf_no_time("    .i2s_clk[0] = %d",pchip_freq->i2s_clk[0]);
    rk_printf_no_time("    .i2s_clk[1] = %d",pchip_freq->i2s_clk[1]);
    rk_printf_no_time("    .ebc_clk = %d",pchip_freq->ebc_clk);
    rk_printf_no_time("    .sdmmc_clk[0] = %d",pchip_freq->sdmmc_clk[0]);
    rk_printf_no_time("    .sdmmc_clk[1] = %d",pchip_freq->sdmmc_clk[1]);
    rk_printf_no_time("    .acodec_clk = %d",pchip_freq->acodec_clk);
    rk_printf_no_time("    .spi_clk[0] = %d",pchip_freq->spi_clk[0]);
    rk_printf_no_time("    .spi_clk[1] = %d",pchip_freq->spi_clk[1]);
    rk_printf_no_time("    .sfc_clk = %d",pchip_freq->sfc_clk);
    rk_printf_no_time("    .saradc_clk = %d",pchip_freq->saradc_clk);
    rk_printf_no_time("    .timer_clk[0] = %d",pchip_freq->timer_clk[0]);
    rk_printf_no_time("    .timer_clk[1] = %d",pchip_freq->timer_clk[1]);
    rk_printf_no_time("    .pmu_clk = %d",pchip_freq->pmu_clk);
    rk_printf_no_time("    .uart_clk[0] = %d",pchip_freq->uart_clk[0]);
    rk_printf_no_time("    .uart_clk[1] = %d",pchip_freq->uart_clk[1]);
    rk_printf_no_time("    .uart_clk[2] = %d",pchip_freq->uart_clk[2]);
    rk_printf_no_time("    .uart_clk[3] = %d",pchip_freq->uart_clk[3]);
    rk_printf_no_time("    .uart_clk[4] = %d",pchip_freq->uart_clk[4]);
}

/*******************************************************************************
** Name: ShellPowerOff
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.25
** Time: 16:17:50
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
SHELL FUN rk_err_t ShellPowerOff(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
    return RK_SUCCESS;
}

#ifdef SHELL_BSP
/*******************************************************************************
** Name: ShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.13
** Time: 16:35:58
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON API rk_err_t ShellBsp(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellBspName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }


    ret = ShellCheckCmd(ShellBspName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellBspName[i].CmdDes, pItem);
    if(ShellBspName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellBspName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;

}

uint32 MailBoxCmd;
uint32 MailBoxID;
uint32 Port;
uint32 MailBoxData;

void MailBoxA2BService()
{
    MailBoxCmd = MailBoxReadA2BCmd(MailBoxID, Port);
    MailBoxData = MailBoxReadA2BData(MailBoxID, Port);
    MailBoxClearA2BInt(MailBoxID, (uint32)(1 << Port));
}

void MailBoxB2AService()
{
    MailBoxCmd = MailBoxReadB2ACmd(MailBoxID, Port);
    MailBoxData = MailBoxReadB2AData(MailBoxID, Port);
    MailBoxClearB2AInt(MailBoxID, (uint32)(1 << Port));
}

/*******************************************************************************
** Name: ShellBspMailBoxTestCmd
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:43:11
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMailBoxTestB2A(HDC dev, uint8 * pstr)
{
    uint32 i;
    uint32 Cmd, Data;

    for(i = 0; i < 4; i++)
    {
        IntRegister(INT_ID_MAILBOX0 + i, MailBoxB2AService);
        IntPendingClear(INT_ID_MAILBOX0 + i);
        IntEnable(INT_ID_MAILBOX0 + i);
        MailBoxEnableB2AInt(MailBoxID, (int32)(1 << i));
        Cmd = 0x55 + i;
        Data = 0xaa + i;
        MailBoxID = 0;
        Port = i;
        MailBoxCmd = 0;
		MailBoxData = 0;

        MailBoxWriteB2ACmd(Cmd, MailBoxID, Port);
        MailBoxWriteB2AData(Data, MailBoxID, Port);

        while(1)
        {
            if((MailBoxCmd == Cmd) && (MailBoxData == Data))
            {
                rk_printf_no_time1("MailBox B2A Port = %d test ok", i);
                break;
            }
        }

        IntUnregister(INT_ID_MAILBOX0 + i);
        IntPendingClear(INT_ID_MAILBOX0 + i);
        IntDisable(INT_ID_MAILBOX0 + i);
        MailBoxDisableB2AInt(MailBoxID, (int32)(1 << i));

    }

    rk_print_string("mailbox test over\r\n");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspMailBoxTestData
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:42:30
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMailBoxTestA2B(HDC dev, uint8 * pstr)
{
    uint32 i;
    uint32 Cmd, Data;

    for(i = 0; i < 4; i++)
    {
        IntRegister(INT_ID_MAILBOX0 + i, MailBoxA2BService);
        IntPendingClear(INT_ID_MAILBOX0 + i);
        IntEnable(INT_ID_MAILBOX0 + i);
        MailBoxEnableA2BInt(MailBoxID, (int32)(1 << i));
        Cmd = 0x55 + i;
        Data = 0xaa + i;
        MailBoxID = 0;
        Port = i;
        MailBoxCmd = 0;
		MailBoxData = 0;

        MailBoxWriteA2BCmd(Cmd, MailBoxID, Port);
        MailBoxWriteA2BData(Data, MailBoxID, Port);

        while(1)
        {
            if((MailBoxCmd == Cmd) && (MailBoxData == Data))
            {
                rk_printf_no_time1("MailBox A2B Port = %d test ok", i);
                break;
            }
        }

        IntUnregister(INT_ID_MAILBOX0 + i);
        IntPendingClear(INT_ID_MAILBOX0 + i);
        IntDisable(INT_ID_MAILBOX0 + i);
        MailBoxDisableA2BInt(MailBoxID, (int32)(1 << i));

    }

    rk_print_string("mailbox test over\r\n");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspMailBoxReadCmd
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:41:41
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMailBoxReadCmd(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspMailBoxReadData
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:41:14
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMailBoxReadData(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspMailBoxWriteCmd
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:18:04
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMailBoxWriteCmd(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspMailBoxWriteData
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:17:32
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMailBoxWriteData(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspGpioTestInt
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:17:03
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspGpioTestInt(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspGpioTestPull
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:16:05
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspGpioTestPull(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspGpioTestInOut
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:14:24
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspGpioTestInOut(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspGpioSetPull
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:13:27
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspGpioSetPull(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspGpioSetLow
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:02:29
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspGpioSetLow(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspGpioSetOut
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:02:08
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspGpioSetOut(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspGpioSetHigh
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:01:27
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspGpioSetHigh(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspGpioSetIn
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:00:56
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspGpioSetIn(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}


static uint32 DmaFinish;
static void DmaIsr()
{
     DmaFinish = 1;
     DmaDisableInt(0);
}

_ATTR_AP_SYS_BSS_ __align(4) uint32 buf[512];

/*******************************************************************************
** Name: ShellBspMemoryTestEffect
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:00:04
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestEffect(HDC dev, uint8 * pstr)
{
    uint32 * pstr1, * pstr2;
    uint32 savesystick,i;

    pstr1 = rkos_memory_malloc(2048);
    //pstr1 = buf;
    pstr2 = rkos_memory_malloc(2048);

    DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

     //open uart clk
    ScuClockGateCtr(CLOCK_GATE_DMA, 1);

    //open rst uart ip
    ScuSoftResetCtr(RST_DMA, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_DMA, 0);

    IntRegister(INT_ID_DMA ,DmaIsr);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    savesystick = SysTickCounter;

    for(i = 0; i < 3000; i++)
    {
        DmaEnableInt(0);
        DmaFinish = 0;

        DmaConfig(0, (uint32)pstr1, (uint32)pstr2, 512, &DmaCfg, NULL);
    	while(!DmaFinish)
        {
            __WFI();
        }

    }

    rk_printf("dma1 acess memory speed = %d M/S", (3 * 2048)/ ((SysTickCounter - savesystick) * 10));


    //close dma clk
    ScuClockGateCtr(CLOCK_GATE_DMA, 0);

    //rst dma ip
    ScuSoftResetCtr(RST_DMA, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_DMA, 0);


    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellBspMemoryTestConsumption
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:59:28
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestConsumption(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
uint32 IDRAM_DMA_CH;
uint32 IDRAM_DmaFinish;
_SYSTEM_SHELL_SHELLBSP_READ_
void IDRAM_DmaIsr()
{
     IDRAM_DmaFinish = 1;
     rk_print_string("\r\n IDRAM_DmaIsr\n");
     DmaDisableInt(IDRAM_DMA_CH);
}
_SYSTEM_SHELL_SHELLBSP_READ_ DMA_LLP TEST_llpListn[DMA_CHN_MAX][50];

/*******************************************************************************
** Name: ShellBspMemoryTest_DMA_LLP_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:11:16
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTest_DMA_LLP_RW(HDC dev, uint8 * pstr)
{
    uint32 *pbuf_LLP_0,*pbuf_LLP_1,*pbuf_LLP_2,
           *pbuf_LLP_3,*pbuf_LLP_4,*pbuf_LLP_5,
           *pbuf_LLP_6,*pbuf_LLP_7,*pbuf_LLP_8,
           *pbuf_LLP_9;
    uint32 *pbuf1_LLP_0,*pbuf1_LLP_1,*pbuf1_LLP_2,
           *pbuf1_LLP_3,*pbuf1_LLP_4,*pbuf1_LLP_5,
           *pbuf1_LLP_6,*pbuf1_LLP_7,*pbuf1_LLP_8,
           *pbuf1_LLP_9;
    uint32 i = 0;
    uint32 f1,f2,f3,f4;

    #define H_IRAM_ADDR 0x01040000
    #define H_IRAM_SIZE 0x40000

    #define H_DRAM_ADDR 0x03040000
    #define H_DRAM_SIZE 0x30000

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        IDRAM_DMA_CH = 0;
        rk_print_string("\r\n IDRAM_DMA_CH start 0\n");
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        IDRAM_DMA_CH = 1;
        rk_print_string("\r\n IDRAM_DMA_CH start 1\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        IDRAM_DMA_CH = 2;
        rk_print_string("\r\n IDRAM_DMA_CH start 2\n");
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        IDRAM_DMA_CH = 3;
        rk_print_string("\r\n IDRAM_DMA_CH start 3\n");
    }
    else if(StrCmpA(pstr, "4", 1) == 0)
    {
        IDRAM_DMA_CH = 4;
        rk_print_string("\r\n IDRAM_DMA_CH start 4\n");
    }
    else if(StrCmpA(pstr, "5", 1) == 0)
    {
        IDRAM_DMA_CH = 5;
        rk_print_string("\r\n IDRAM_DMA_CH start 5\n");
    }

    pbuf_LLP_0 = (uint32 *)(0x01040000);
    pbuf_LLP_1 = (uint32 *)(0x01040500);
    pbuf_LLP_2 = (uint32 *)(0x01041000);
    pbuf_LLP_3 = (uint32 *)(0x01041500);
    pbuf_LLP_4 = (uint32 *)(0x01042000);
    pbuf_LLP_5 = (uint32 *)(0x01042500);
    pbuf_LLP_6 = (uint32 *)(0x01043000);
    pbuf_LLP_7 = (uint32 *)(0x01043500);
    pbuf_LLP_8 = (uint32 *)(0x01044000);
    pbuf_LLP_9 = (uint32 *)(0x01044500);

    pbuf1_LLP_0 = (uint32 *)0x03040000;
    pbuf1_LLP_1 = (uint32 *)0x03040500;
    pbuf1_LLP_2 = (uint32 *)0x03041000;
    pbuf1_LLP_3 = (uint32 *)0x03041500;
    pbuf1_LLP_4 = (uint32 *)0x03042000;
    pbuf1_LLP_5 = (uint32 *)0x03042500;
    pbuf1_LLP_6 = (uint32 *)0x03043000;
    pbuf1_LLP_7 = (uint32 *)0x03043500;
    pbuf1_LLP_8 = (uint32 *)0x03044000;
    pbuf1_LLP_9 = (uint32 *)0x03044500;


    //open uart clk
    ScuClockGateCtr(CLOCK_GATE_DMA, 1);

    //open rst uart ip
    ScuSoftResetCtr(RST_DMA, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_DMA, 0);

    IntRegister(INT_ID_DMA ,IDRAM_DmaIsr);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DmaEnableInt(IDRAM_DMA_CH);
    IDRAM_DmaFinish = 0;

    DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

    f1 = f3 = 0x55aa55aa;
    f2 = f4 = 0xaa55aa55;

    for(i = 0; i < 0x100; i++)
    {
        *pbuf_LLP_0++ = f1;
        *pbuf_LLP_1++ = f1;
        *pbuf_LLP_2++ = f1;
        *pbuf_LLP_3++ = f1;
        *pbuf_LLP_4++ = f1;
        *pbuf_LLP_5++ = f1;
        *pbuf_LLP_6++ = f1;
        *pbuf_LLP_7++ = f1;
        *pbuf_LLP_8++ = f1;
        *pbuf_LLP_9++ = f1;

        /*
        *pbuf1_LLP_0++ = 0;
        *pbuf1_LLP_1++ = 0;
        *pbuf1_LLP_2++ = 0;
        *pbuf1_LLP_3++ = 0;
        *pbuf1_LLP_4++ = 0;
        *pbuf1_LLP_5++ = 0;
        *pbuf1_LLP_6++ = 0;
        *pbuf1_LLP_7++ = 0;
        *pbuf1_LLP_8++ = 0;
        *pbuf1_LLP_9++ = 0;
        */
    }

    TEST_llpListn[IDRAM_DMA_CH][0].SAR = 0x01040000;
    TEST_llpListn[IDRAM_DMA_CH][0].DAR = 0x03040000;
    TEST_llpListn[IDRAM_DMA_CH][0].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][1].SAR = 0x01040500;
    TEST_llpListn[IDRAM_DMA_CH][1].DAR = 0x03040500;
    TEST_llpListn[IDRAM_DMA_CH][1].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][2].SAR = 0x01041000;
    TEST_llpListn[IDRAM_DMA_CH][2].DAR = 0x03041000;
    TEST_llpListn[IDRAM_DMA_CH][2].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][3].SAR = 0x01041500;
    TEST_llpListn[IDRAM_DMA_CH][3].DAR = 0x03041500;
    TEST_llpListn[IDRAM_DMA_CH][3].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][4].SAR = 0x01042000;
    TEST_llpListn[IDRAM_DMA_CH][4].DAR = 0x03042000;
    TEST_llpListn[IDRAM_DMA_CH][4].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][5].SAR = 0x01042500;
    TEST_llpListn[IDRAM_DMA_CH][5].DAR = 0x03042500;
    TEST_llpListn[IDRAM_DMA_CH][5].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][6].SAR = 0x01043000;
    TEST_llpListn[IDRAM_DMA_CH][6].DAR = 0x03043000;
    TEST_llpListn[IDRAM_DMA_CH][6].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][7].SAR = 0x01043500;
    TEST_llpListn[IDRAM_DMA_CH][7].DAR = 0x03043500;
    TEST_llpListn[IDRAM_DMA_CH][7].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][8].SAR = 0x01044000;
    TEST_llpListn[IDRAM_DMA_CH][8].DAR = 0x03044000;
    TEST_llpListn[IDRAM_DMA_CH][8].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][9].SAR = 0x01044500;
    TEST_llpListn[IDRAM_DMA_CH][9].DAR = 0x03044500;
    TEST_llpListn[IDRAM_DMA_CH][9].SIZE = 0x100;

    DmaConfig_for_LLP(IDRAM_DMA_CH, 0x100, 10,&DmaCfg, TEST_llpListn[IDRAM_DMA_CH]);
		while(!IDRAM_DmaFinish);


    for(i = 0; i < 0x100; i++)
    {
        f1 = *pbuf1_LLP_0++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_0 = 0x%x", f1);
        }

        f1 = *pbuf1_LLP_1++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_1 = 0x%x", f1);
        }


        f1 = *pbuf1_LLP_2++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_2 = 0x%x", f1);
        }

        f1 = *pbuf1_LLP_3++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_3 = 0x%x", f1);
        }

        f1 = *pbuf1_LLP_4++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_4 = 0x%x", f1);
        }

        f1 = *pbuf1_LLP_5++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_5 = 0x%x", f1);
        }

        f1 = *pbuf1_LLP_6++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_6 = 0x%x", f1);
        }
#if 1
        f1 = *pbuf1_LLP_7++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_7 = 0x%x", f1);
        }

        f1 = *pbuf1_LLP_8++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_8 = 0x%x", f1);
        }

        f1 = *pbuf1_LLP_9++;
        if((f1 & f3) != f3)
        {
            rk_printf("pbuf1_LLP_9 = 0x%x", f1);
        }
#endif

    }

    DmaDisableInt(IDRAM_DMA_CH);

    rk_print_string("\r\ndmallp test over");

    #undef H_IRAM_ADDR
    #undef H_IRAM_SIZE
    #undef H_DRAM_ADDR
    #undef H_DRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTest_DMA_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:10:43
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTest_DMA_RW(HDC dev, uint8 * pstr)
{
    uint32 * pbuf, *pbuf1;
    uint32 i = 0;
    uint32 f1,f2,f3,f4;

    #define H_IRAM_ADDR 0x01040000
    #define H_IRAM_SIZE 0x40000

    #define H_DRAM_ADDR 0x03040000
    #define H_DRAM_SIZE 0x30000

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        IDRAM_DMA_CH = 0;
        rk_print_string("\r\n IDRAM_DMA_CH start 0\n");
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        IDRAM_DMA_CH = 1;
        rk_print_string("\r\n IDRAM_DMA_CH start 1\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        IDRAM_DMA_CH = 2;
        rk_print_string("\r\n IDRAM_DMA_CH start 2\n");
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        IDRAM_DMA_CH = 3;
        rk_print_string("\r\n IDRAM_DMA_CH start 3\n");
    }
    else if(StrCmpA(pstr, "4", 1) == 0)
    {
        IDRAM_DMA_CH = 4;
        rk_print_string("\r\n IDRAM_DMA_CH start 4\n");
    }
    else if(StrCmpA(pstr, "5", 1) == 0)
    {
        IDRAM_DMA_CH = 5;
        rk_print_string("\r\n IDRAM_DMA_CH start 5\n");
    }
    //open uart clk
    ScuClockGateCtr(CLOCK_GATE_DMA, 1);

    //open rst uart ip
    ScuSoftResetCtr(RST_DMA, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_DMA, 0);

    IntRegister(INT_ID_DMA ,IDRAM_DmaIsr);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DmaEnableInt(IDRAM_DMA_CH);
    IDRAM_DmaFinish = 0;

    DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

    f1 = f3 = 0x55aa55aa;
    f2 = f4 = 0xaa55aa55;

//------------------------------------------------------------//
    pbuf = (uint32 *)H_IRAM_ADDR;

    for(i = 0; i < H_DRAM_SIZE/4; i++)
    {
        *pbuf++ = f1;
    }

    DmaConfig(IDRAM_DMA_CH, H_IRAM_ADDR, H_DRAM_ADDR, H_DRAM_SIZE/4, &DmaCfg, TEST_llpListn[IDRAM_DMA_CH]);
		while(!IDRAM_DmaFinish);

    DmaDisableInt(IDRAM_DMA_CH);

    pbuf1 = (uint32 *)H_DRAM_ADDR;

    f1 = 0;

    for(i = 0; i < H_DRAM_SIZE/4; i++)
    {
        f1 = *pbuf1++;
        if((f1 & f3) != f3)
        {
            rk_printf("err data f1 = 0x%x", f1);
            rk_printf("err address = 0x%x", pbuf1);
        }

        if((i%500) == 0)
        {
           printf("0x%x,", f1);
        }
    }
    rk_print_string("\r\n 55aa test over\n");
//-------------------------------------------------------------------//
    pbuf = (uint32 *)H_IRAM_ADDR;

    for(i = 0; i < H_DRAM_SIZE/4; i++)
    {
        *pbuf++ = f2;
    }

    DmaEnableInt(IDRAM_DMA_CH);
    IDRAM_DmaFinish = 0;

    DmaConfig(IDRAM_DMA_CH, H_IRAM_ADDR, H_DRAM_ADDR, H_DRAM_SIZE/4, &DmaCfg, TEST_llpListn[IDRAM_DMA_CH]);
		while(!IDRAM_DmaFinish);

    DmaDisableInt(IDRAM_DMA_CH);

    pbuf1 = (uint32 *)H_DRAM_ADDR;

    f2 = 0;

    for(i = 0; i < H_DRAM_SIZE/4; i++)
    {
        f2 = *pbuf1++;
        if((f2 & f4) != f4)
        {
            rk_printf("err data f2 = 0x%x", f2);
            rk_printf("err address = 0x%x", pbuf1);
        }

        if((i%500) == 0)
        {
           printf("0x%x,", f2);
        }
    }
    rk_print_string("\r\n aa55 test over\n");
    #undef H_IRAM_ADDR
    #undef H_IRAM_SIZE
    #undef H_DRAM_ADDR
    #undef H_DRAM_SIZE
    rk_print_string("\r\ndma test over");

}

/*******************************************************************************
** Name: ShellBspMemoryTestPMUSRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:53:07
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestPMUSRAM_RW(HDC dev, uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;
    uint32 i = 0;
    uint32 f1,f2,f3,f4;
    #define PMU_SRAM_ADDR0 0x00000000
    #define PMU_SRAM_ADDR1 0x00ff0000
    #define PMU_SRAM_SIZE  0x10000
    //test pmu sram

    pbuf = (uint8 *)PMU_SRAM_ADDR1;

    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        rk_print_string("\r\npmu sram read write test start 0x55aa\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        rk_print_string("\r\npmu sram read write test start 0xaa55\n");
    }



    for(i = 0; i < PMU_SRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)PMU_SRAM_ADDR1;

    for(i = 0; i < PMU_SRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            rk_printf("err data f1 = %x", f1);
            rk_printf("err address = %x", pbuf);
        }

        f2 = *pbuf++;

        if((f2 & f4) != f4)
        {
            rk_printf("err data f2 = %x", f2);
            rk_printf("err address = %x", pbuf);
        }

        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2);
        }
    }

    rk_print_string("\r\npmu sram read write test over");
    #undef PMU_SRAM_ADDR0
    #undef PMU_SRAM_ADDR1
    #undef PMU_SRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTestHIRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:51:37
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestHIRAM_RW(HDC dev, uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;
    uint32 i = 0;
    uint8 f1,f2,f3,f4;

    #define H_IRAM_ADDR 0x01040000
    #define H_IRAM_SIZE 0x40000

    //test H iram
    pbuf = (uint8 *)H_IRAM_ADDR;

    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        rk_print_string("\r\nHiram read write test start 0x55aa\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        rk_print_string("\r\nHiram read write test start 0xaa55\n");
    }

    for(i = 0; i < H_IRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)H_IRAM_ADDR;

    for(i = 0; i < H_IRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            rk_printf("err data f1 = %x", f1);
            rk_printf("err address = %x", pbuf);
        }

        f2 = *pbuf++;

        if((f2 & f4) != f4)
        {
            rk_printf("err data f2 = %x", f2);
            rk_printf("err address = %x", pbuf);
        }

        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2);
        }
    }

    rk_print_string("\r\nhiram read write test over");
    #undef H_IRAM_ADDR
    #undef H_IRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTestHDRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:49:32
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestHDRAM_RW(HDC dev, uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;
    uint32 i = 0;
    uint8 f1,f2,f3,f4;

    #define H_DRAM_ADDR 0x03040000
    #define H_DRAM_SIZE 0x30000

    //test H dram
    pbuf = (uint8 *)H_DRAM_ADDR;

    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        rk_print_string("\r\nHdram read write test start 0x55aa\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        rk_print_string("\r\nHdram read write test start 0xaa55\n");
    }

    for(i = 0; i < H_DRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)H_DRAM_ADDR;

    for(i = 0; i < H_DRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            rk_printf("err data f1 = %x", f1);
            rk_printf("err address = %x", pbuf);
        }

        f2 = *pbuf++;

        if((f2 & f4) != f4)
        {
            rk_printf("err data f2 = %x", f2);
            rk_printf("err address = %x", pbuf);
        }

        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2);
        }
    }

    rk_print_string("\r\nhdram read write test over");
    #undef H_DRAM_ADDR
    #undef H_DRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTestDRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:45:16
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestDRAM_RW(HDC dev, uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;
    uint32 i = 0;
    uint8 f1,f2,f3,f4;
    #define DRAM_ADDR 0x03000000
    #define DRAM_SIZE 0x40000


    //test H dram
    pbuf = (uint8 *)DRAM_ADDR;

    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        rk_print_string("\r\ndram read write test start 0x55aa\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        rk_print_string("\r\ndram read write test start 0xaa55\n");
    }


    for(i = 0; i < DRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)DRAM_ADDR;

    for(i = 0; i < DRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            rk_printf("err data f1 = %x", f1);
            rk_printf("err address = %x", pbuf);
        }

        f2 = *pbuf++;

        if((f2 & f4) != f4)
        {
            rk_printf("err data f2 = %x", f2);
            rk_printf("err address = %x", pbuf);
        }

        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2);
        }
    }

    rk_print_string("\r\ndram read write test over");

    #undef DRAM_ADDR
    #undef DRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTestIRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:42:02
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestIRAM_RW(HDC dev, uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;
    uint32 i = 0;
    uint8 f1,f2,f3,f4;

    #define IRAM_ADDR 0x01000000
    #define IRAM_SIZE 0x40000

    //test H dram
    pbuf = (uint8 *)IRAM_ADDR;

    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        rk_print_string("\r\niram read write test start 0x55aa\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        rk_print_string("\r\niram read write test start 0xaa55\n");
    }

    for(i = 0; i < IRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)IRAM_ADDR;

    for(i = 0; i < IRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            rk_printf("err data f1 = %x", f1);
            rk_printf("err address = %x", pbuf);
        }

        f2 = *pbuf++;

        if((f2 & f4) != f4)
        {
            rk_printf("err data f2 = %x", f2);
            rk_printf("err address = %x", pbuf);
        }

        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2);
        }
    }

    rk_print_string("\r\niram read write test over");
    #undef IRAM_ADDR
    #undef IRAM_SIZE


}

/*******************************************************************************
** Name: ShellBspMemoryTestReadWrite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:59:02
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestReadWrite(HDC dev, uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;

    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellBspMemoryRAMName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellBspMemoryRAMName, pItem, StrCnt);


    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellBspMemoryRAMName[i].CmdDes, pItem);
    if(ShellBspMemoryRAMName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellBspMemoryRAMName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;

}
/*******************************************************************************
** Name: ShellBspMemoryTestClk
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:56:10
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestClk(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspMemoryTestPower
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:55:37
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemoryTestPower(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspNvicResumeNanoD
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:54:40
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspNvicResumeNanoD(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspNvicResumeM3
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:54:12
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspNvicResumeM3(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspNvicTestPriority
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:53:11
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspNvicTestPriority(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}

uint32 IntCnt;

void TestInt(void)
{
    IntCnt++;
}

/*******************************************************************************
** Name: ShellBspNvicTestInt
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:52:30
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspNvicTestInt(HDC dev, uint8 * pstr)
{
    uint32 i;
    IntCnt = 15;

    for(i = 16; i < NUM_INTERRUPTS; i++)
    {
        if(i == INT_ID_UART0)
        {
			IntCnt++;
			continue;
        }

        IntRegister(i ,TestInt);
        IntPendingClear(i);
        IntEnable(i);

        IntPendingSet(i);

        while(1)
        {
            if(IntCnt == i)
            {
                rk_printf_no_time("Int = %d test ok", i);
                break;
            }
        }

        IntDisable(i);

        IntPendingClear(i);

        IntUnregister(i);

    }

    rk_print_string("\r\nnvic int test over");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspNvicSetMask
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:51:36
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspNvicSetMask(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspNvicSetPriority
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:50:56
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspNvicSetPriority(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellBspNvicSetInt
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:50:14
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspNvicSetInt(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellBspMailBox
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 9:16:04
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMailBox(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellBspMailBoxName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }


    ret = ShellCheckCmd(ShellBspMailBoxName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellBspMailBoxName[i].CmdDes, pItem);
    if(ShellBspMailBoxName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellBspMailBoxName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;

}


/*******************************************************************************
** Name: ShellBspNvic
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 9:14:32
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspNvic(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellBspNvicName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellBspNvicName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellBspNvicName[i].CmdDes, pItem);
    if(ShellBspNvicName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellBspNvicName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;

}

/*******************************************************************************
** Name: ShellBspGPIO
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 9:14:03
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspGPIO(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellBspMemory
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 9:13:00
*******************************************************************************/
_SYSTEM_SHELL_SHELLBSP_READ_
COMMON FUN rk_err_t ShellBspMemory(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellBspMemoryName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellBspMemoryName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellBspMemoryName[i].CmdDes, pItem);
    if(ShellBspMemoryName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellBspMemoryName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;

}
#endif


/*******************************************************************************
** Name: ShellMemory
** Input:HDC dev,  char * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 10:18:40
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN rk_err_t ShellMemory(HDC dev,  char * pstr)
{
    uint8 TxBuffer[128];
    uint32 MemorySize;
    uint16 BuffSize = 0;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    if (dev == NULL)
        return RK_ERROR;

    MemorySize = RKTaskHeapFree();
    BuffSize = sprintf(TxBuffer,"Total       Used      Remaining\r\n");
    UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

    BuffSize = sprintf(TxBuffer,"-------------------------------\r\n");
    UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);


    BuffSize = sprintf(TxBuffer,"0x%x      ",RKTaskHeapTotal());
    UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

    BuffSize = sprintf(TxBuffer,"0x%x      ",(RKTaskHeapTotal() - RKTaskHeapFree()));
    UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

    BuffSize = sprintf(TxBuffer,"0x%x    ",MemorySize);
    UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellRkosIdleTest
** Input:HDC dev,  char * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 9:59:56
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN rk_err_t ShellRkosIdleTest(HDC dev,  char * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    IdleTime = 1;

    pIdleFunSave = pIdleFun;
    pIdleFun = RkosEffectTest1;

    rk_print_string("  rkos executive capacity testing...\r\n");

    SysTick = SysTickCounter;
    IdleCnt = 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellCpuTest
** Input:HDC dev,  char * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 9:53:14
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN rk_err_t ShellCpuTest(HDC dev,  char * pstr)
{
    uint32 Time, Delay, Tick, MaxFreq;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);
    OBS_output_Source_sel(obs_clk_sys_core);

    pIdleFunSave = pIdleFun;
    if (StrCmpA(pstr, "/1", 2) == 0)
    {
        pIdleFun = RkosEffectTest2;
        pstr += 2;
    }
    else if (StrCmpA(pstr, "/2", 2) == 0)
    {
        pIdleFun = RkosEffectTest3;
         pstr += 2;
    }
    else
    {
        pIdleFun = RkosEffectTest1;
    }

    if(*pstr != NULL)
    {
        MaxFreq = String2Num(pstr);
    }
    else
    {
        MaxFreq = SYS_MAX_FREQ;
    }

    rk_print_string("  cpu chage freq test...\r\n");

    for (Time = SYS_MIN_FREQ; Time <= MaxFreq; Time++)
    {
        SetSysFreq(Time);

        rk_printf("armclk = %dHz, \tpclk = %dHz", chip_freq.hclk_sys_core, chip_freq.pclk_logic_pre);

        Delay = 0x7ffff;
        while (Delay--);
    }

    rk_print_string("\r\n  cpu executive capacity testing...\r\n");

    vTaskSuspendAll();

    IdleCnt = 0;
    SysTick = SysTickCounter;

    while (pIdleFun != NULL)
    {
        vApplicationIdleHook();
    }

    pIdleFun = pIdleFunSave;

    Tick = SysTickCounter -  SysTick;
    Time = (EFFECT_TEST_NUM * EFFECT_TEST_BASE_NUM)/ Tick;

    xTaskResumeAll();

    rk_printf_no_time("  cpu executive capacity is %d --- %d\r\n", Time, Tick);

    rk_print_string("\r\ncpu test over");

    FREQ_Resume();


    return RK_SUCCESS;

}

/*******************************************************************************
** Name: RkosEffectTestReport
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 9:51:31
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN void RkosEffectTestReport(void)
{
    uint32 time, tick;

    tick = SysTickCounter -  SysTick - 1;
    time = (EFFECT_TEST_NUM * 3000)/ tick;

    printf("\r\n  rkos executive capacity is %d --- %d", time, tick);

}

/*******************************************************************************
** Name: RkosEffectTest1
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 9:49:59
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN void RkosEffectTest1(void)
{
    if (IdleCnt < EFFECT_TEST_NUM)
    {
        MathCapacity();
        IdleCnt++;
    }
    else if (IdleCnt == EFFECT_TEST_NUM)
    {
        if (IdleTime != NULL)
        {
            RkosEffectTestReport();
            pIdleFun = pIdleFunSave;
            IdleTime = 0;
            IdleCnt = 0;
        }
        else
        {
            pIdleFun = NULL;
            IdleCnt = 0;
        }
    }

}

/*******************************************************************************
** Name: MathCapacity
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 9:48:50
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN void MathCapacity(void)
{
    uint32 i, j;
    for (i = 0; i < EFFECT_TEST_BASE_NUM; i++)
        temp++;
}

/*******************************************************************************
** Name: RkosEffectTest2
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 9:45:32
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN void RkosEffectTest2(void)
{
    if (IdleCnt < EFFECT_TEST_NUM)
    {
        MemoryReadWriteTest();
        IdleCnt++;
    }
    else if (IdleCnt == EFFECT_TEST_NUM)
    {
        if (IdleTime != NULL)
        {
            pIdleFun = pIdleFunSave;
            IdleCnt = 0;
            IdleTime = NULL;

        }
        else
        {
            pIdleFun = NULL;
            IdleCnt = 0;
        }
    }

}
/*******************************************************************************
** Name: MemoryReadWriteTest
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 9:44:26
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN void MemoryReadWriteTest(void)
{
    //SCAN H_IRAM
    uint8 * pbuf = rkos_memory_malloc(512);
    uint8 * pbuf1;
    uint8 * pbuf2;
    uint32 i, j;
    uint8 temp;

    for (i = 0; i < EFFECT_TEST_BASE_NUM; i++)
    {
        pbuf1 = pbuf;
        pbuf2 = pbuf;

        for (j = 0; j < 512; j++)
        {
            *pbuf1++ = temp;
            temp = *pbuf2++;
        }
    }

    rkos_memory_free(pbuf);

}

/*******************************************************************************
** Name: RkosEffectTest3
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 9:42:35
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN void RkosEffectTest3(void)
{
    if (IdleCnt < EFFECT_TEST_NUM)
    {
        Filt(tempbuf, EFFECT_TEST_BASE_NUM);
        IdleCnt++;
    }
    else if (IdleCnt == EFFECT_TEST_NUM)
    {
        if (IdleTime != NULL)
        {
            pIdleFun = pIdleFunSave;
            IdleCnt = 0;
            IdleTime = NULL;
        }
        else
        {
            pIdleFun = NULL;
            IdleCnt = 0;
        }
    }
}

/*******************************************************************************
** Name: Filt
** Input:int* in, int len
** Return: void
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 9:41:22
*******************************************************************************/
_SYSTEM_SHELL_SHELLSYSCMD_COMMON_
COMMON FUN void Filt(int* in, int len)
{
    int iLoop;
    int *inptr;
    long long temp;
    int *coeff;
    int * out;

    out = in;

    while (len--)
    {
        coeff = sCoefs;
        inptr = in;
        in++;
        temp = 0;

        iLoop = 32;
        while (iLoop--)
        {
            int scoeff;
            scoeff = *coeff;
            temp += scoeff * (*inptr--);
            coeff++;
        }
        *out = (int)temp;
        out++;
    }

}



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
