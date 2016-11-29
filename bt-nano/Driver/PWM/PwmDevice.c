/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\PWM\PwmDevice.c
* Owner: chad.Ma
* Date: 2014.11.10
* Time: 17:45:27
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    chad.Ma     2014.11.10     17:45:27   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_PWM_PWMDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _PWM_DEVICE_CLASS
{
    DEVICE_CLASS stPwmDevice;
    uint32 channel;
    pSemaphore osPwmControlReqSem;
    uint32 Rate;

}PWM_DEVICE_CLASS;


#define PWM_FREQ        10000     //can not exceed 10KHZ
#define BL_PWM_CH       1

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static PWM_DEVICE_CLASS * gpstPwmDevISR;



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
rk_err_t PwmDevCheckHandler(HDC dev);
void PwmDevShellIsrServer4(void);
void PwmDevShellIsrServer0(void);
void PwmDevShellIsrHandle(ePWM_CHN pwmChannel);
rk_err_t PwmDevShellBspStart(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellBspStop(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellBspSetRate(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellBspSetFreq(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellBspIrqTest(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellBspHelp(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellHelp(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellBsp(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellTest(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellDel(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t PwmDevShellPcb(HDC dev, uint8 * pstr);
void PwmDevIntIsr(void);
rk_err_t PwmDevDeInit(PWM_DEVICE_CLASS * pstPwm);
rk_err_t PwmDevInit(PWM_DEVICE_CLASS * pstPwm);
rk_err_t PwmDevResume(HDC dev);
rk_err_t PwmDevSuspend(HDC dev, uint32 Level);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: PwmDev_SetRate
** Input:HDC dev, UINT32 rate
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.17
** Time: 17:39:14
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_COMMON_
COMMON API rk_err_t     PwmDev_SetRate(HDC dev, UINT32 rate)
{
    PWM_DEVICE_CLASS * pstPwm = (PWM_DEVICE_CLASS *)dev;

    rkos_semaphore_take(gpstPwmDevISR->osPwmControlReqSem,MAX_DELAY);

    PwmRateSet(pstPwm->channel, rate, 10000);
    pstPwm->Rate = rate;

    rkos_semaphore_give(gpstPwmDevISR->osPwmControlReqSem);
}

/*******************************************************************************
** Name: PwmDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_COMMON_
COMMON API rk_err_t PwmDev_Write(HDC dev)
{

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_COMMON_
COMMON API rk_err_t PwmDev_Read(HDC dev)
{
    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: PwmDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.12
** Time: 18:50:17
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_COMMON_
COMMON FUN rk_err_t PwmDevCheckHandler(HDC dev)
{
    if(dev == gpstPwmDevISR)
    {
        return RK_SUCCESS;
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: PwmDevIntIsr
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_COMMON_
COMMON FUN void PwmDevIntIsr(void)
{
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: PwmDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_INIT_
INIT FUN rk_err_t PwmDevResume(HDC dev)
{
     PWM_DEVICE_CLASS * pstPwmDev = (PWM_DEVICE_CLASS *)dev;
     if(PwmDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }
     PwmDevHwInit (pstPwmDev->stPwmDevice.DevID,pstPwmDev->channel);
     PwmDevInit(pstPwmDev);
     pstPwmDev->stPwmDevice.State = DEV_STATE_WORKING;
     return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_INIT_
INIT FUN rk_err_t PwmDevSuspend(HDC dev, uint32 Level)
{
    PWM_DEVICE_CLASS * pstPwmDev = (PWM_DEVICE_CLASS *)dev;
    if(PwmDevCheckHandler(dev) == RK_ERROR)
    {
       return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstPwmDev->stPwmDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstPwmDev->stPwmDevice.State = DEV_SATE_IDLE2;
    }

    PwmDevHwDeInit(pstPwmDev->stPwmDevice.DevID, pstPwmDev->channel);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_INIT_
INIT API rk_err_t PwmDev_Delete(uint32 DevID, void * arg)
{
    PwmDevHwDeInit(DevID, gpstPwmDevISR->channel);
    rkos_semaphore_delete(gpstPwmDevISR->osPwmControlReqSem);
    rkos_memory_free(gpstPwmDevISR);
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_PWM_DEV);
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_INIT_
INIT API HDC PwmDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS*     pDev;
    PWM_DEVICE_CLASS* PwmDev;
    PWM_DEV_ARG * pstPwmDevArg;

    pstPwmDevArg = (PWM_DEV_ARG *)arg;
    if(pstPwmDevArg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    PwmDev  = (PWM_DEVICE_CLASS*)rkos_memory_malloc(sizeof(PWM_DEVICE_CLASS));
    if (PwmDev == NULL)
    {
        return PwmDev;
    }

    PwmDev->osPwmControlReqSem = rkos_semaphore_create(1,1);

    if(PwmDev->osPwmControlReqSem == 0)
    {
        rkos_semaphore_delete(PwmDev->osPwmControlReqSem);
        rkos_memory_free(PwmDev);
        rk_print_string("\ncreate PwmDev fail");
        return (HDC) RK_ERROR;
    }

    PwmDev->channel = pstPwmDevArg->channel;
    pDev = (DEVICE_CLASS* )PwmDev;
    pDev->Idle1EventTime = 10 * PM_TIME;
    pDev->SuspendMode = ENABLE_MODE;
    pDev->suspend = PwmDevSuspend;
    pDev->resume = PwmDevResume;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_PWM_DEV, SEGMENT_OVERLAY_CODE);
#endif

    PwmDevHwInit (DevID,PwmDev->channel);

    if(PwmDevInit(PwmDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(PwmDev->osPwmControlReqSem);
        rkos_memory_free(PwmDev);
        rk_print_string("\ncreate PwmDev fail");
        return (HDC) RK_ERROR;
    }

    gpstPwmDevISR = PwmDev;

    return &(PwmDev->stPwmDevice);
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: PwmDevDeInit
** Input:PWM_DEVICE_CLASS * pstPwm
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_INIT_
INIT FUN rk_err_t PwmDevDeInit(PWM_DEVICE_CLASS * pstPwm)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDevInit
** Input:PWM_DEVICE_CLASS * pstPwm
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_INIT_
INIT FUN rk_err_t PwmDevInit(PWM_DEVICE_CLASS * pstPwm)
{
    PwmRateSet(0,100,10000);
    pstPwm->Rate = 100;
    return RK_SUCCESS;
}


#ifdef _PWM_DEV_SHELL_
_DRIVER_PWM_PWMDEVICE_SHELL_DATA_
static SHELL_CMD ShellPwmName[] =
{
    "pcb",PwmDevShellPcb,"NULL","NULL",
    "create",PwmDevShellCreate,"NULL","NULL",
    "del",PwmDevShellDel,"NULL","NULL",
    "test",PwmDevShellTest,"NULL","NULL",
#ifdef SHELL_BSP
    "bsp",PwmDevShellBsp,"NULL","NULL",
#endif
    "\b",NULL,"NULL","NULL",
};

#ifdef SHELL_BSP
static SHELL_CMD ShellPwmBspName[] =
{
    "start",PwmDevShellBspStart,"NULL","NULL",
    "stop",PwmDevShellBspStop,"NULL","NULL",
    "setrate",PwmDevShellBspSetRate,"NULL","NULL",
    "setfreq",PwmDevShellBspSetFreq,"NULL","NULL",
    "irqtest",PwmDevShellBspIrqTest,"NULL","NULL",
    "\b",NULL,"NULL","NULL",

};


uint8       gPWMChannel;
uint8       gPWMInt_flag;
uint32      gPWMRate;       //reference/capture
uint32      gPWMFreq;       //freq
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: PwmDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL API rk_err_t PwmDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellPwmName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellPwmName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                      //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellPwmName[i].CmdDes, pItem);
    if(ShellPwmName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellPwmName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef SHELL_BSP
/*******************************************************************************
** Name: PwmDevShellIsrServer1
** Input:void
** Return: vois
** Owner:chad.Ma
** Date: 2014.12.1
** Time: 17:31:50
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN void PwmDevShellIsrServer4(void)
{
    PwmDevShellIsrHandle(PWM_CHN4);
}

/*******************************************************************************
** Name: PwmDevShellIsrServer
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.12.1
** Time: 17:30:00
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN void PwmDevShellIsrServer0(void)
{
    PwmDevShellIsrHandle(PWM_CHN0);
}

/*******************************************************************************
** Name: PwmDevShellIsrHandle
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.24
** Time: 16:44:21
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN void PwmDevShellIsrHandle(ePWM_CHN pwmChannel )
{
    gPWMInt_flag = 1;
    PWM_SetIntDisable( pwmChannel );
    PWM_ClrInt( pwmChannel );
//    printf( "pwm channel[%d] interrupt handle out~ \r\n",pwmChannel);
}

/*******************************************************************************
** Name: PwmDevShellBspHelp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 18:06:11
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellBspHelp(HDC dev, uint8 * pstr)
{
     pstr--;

    if(StrLenA(pstr) != 0)
        return RK_ERROR;

    printf("pwm.bsp?????????????pwm??????????\r\n");
    printf("??????:\r\n");
    printf("help :        pwm.bsp ??????? \r\n");
    printf("start :       ??pwm\r\n");
    printf("stop  :       ??pwm\r\n");
    printf("setrate :     ??pwm???\r\n");
    printf("setfreq :     ???? \r\n");
    printf("irqtest :     ??pwm??????r\n");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDevShellBspStart
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 18:08:26
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellBspStart(HDC dev, uint8 * pstr)
{
    // TODO:
    //add other code below:
    gPWMChannel = PWM_CHN0;
    PWM_Start(gPWMChannel);

    //open rst uart ip
    ScuSoftResetCtr(RST_PWM, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_PWM, 0);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmShellBspStop
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 18:08:01
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellBspStop(HDC dev, uint8 * pstr)
{

    // TODO:
    //add other code below:

    gPWMChannel = PWM_CHN0;
    PWM_Start(gPWMChannel);

    //open rst uart ip
    ScuSoftResetCtr(RST_PWM, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_PWM, 0);

    PWM_Stop( gPWMChannel );

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmShellBspSetRate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 18:07:37
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellBspSetRate(HDC dev, uint8 * pstr)
{
    UINT32 rate;    //reference / capture

    // TODO:
    //add other code below:
    while(pstr[0] == ' ')
        pstr++;

    rate = StringtoNum(pstr);
    if( rate > 100 )
    {
        printf("???0-100??????,????100. \r\n");
        return RK_PARA_ERR;
    }

    gPWMChannel = PWM_CHN0;
    PWM_Start(gPWMChannel);

    //open rst uart ip
    ScuSoftResetCtr(RST_PWM, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_PWM, 0);

    PwmRegReset(gPWMChannel);
    gPWMFreq = 10000;
    PwmRateSet(gPWMChannel , rate, gPWMFreq);
//    PWM_Stop( gPWMChannel );

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmShellBspSetFreq
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 18:07:18
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellBspSetFreq(HDC dev, uint8 * pstr)
{
    uint32 freq;

    while(pstr[0] == ' ')
        pstr++;

    freq = StringtoNum(pstr);
    if(freq > 10000)
    {
        printf("warinning :pwm??????10khz,please try again.\r\n");
        return RK_PARA_ERR;
    }

    gPWMChannel = PWM_CHN0;
    PWM_Start(gPWMChannel);

    //open rst uart ip
    ScuSoftResetCtr(RST_PWM, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_PWM, 0);

    PwmRegReset(gPWMChannel);
    gPWMRate = 50;
    PwmRateSet(gPWMChannel,gPWMRate,freq);
//    PWM_Stop( gPWMChannel );
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmShellBspIrqTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 18:06:59
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellBspIrqTest(HDC dev, uint8 * pstr)
{

    gPWMChannel = PWM_CHN0;
    gPWMInt_flag = 0;

    PWM_Start(gPWMChannel);

    //open rst uart ip
    ScuSoftResetCtr(RST_PWM, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_PWM, 0);

    switch(gPWMChannel)
    {
        case PWM_CHN0:
        case PWM_CHN1:
        case PWM_CHN2:
        case PWM_CHN3:
            IntRegister(INT_ID_PWM0 ,PwmDevShellIsrServer0);
            IntPendingClear(INT_ID_PWM0);
            IntEnable(INT_ID_PWM0);
            break;

        case PWM_CHN4:
            IntRegister(INT_ID_PWM1 ,PwmDevShellIsrServer4);
            IntPendingClear(INT_ID_PWM1);
            IntEnable(INT_ID_PWM1);
            break;

         default:
            break;
    }

    gPWMRate = 50;
    gPWMFreq = 10000;

    PwmRegReset(gPWMChannel);
    PWM_SetIntEnable(gPWMChannel);
    PwmRateSet(gPWMChannel , gPWMRate, gPWMFreq);

    while(!gPWMInt_flag);

    if(gPWMInt_flag)
    {
        gPWMInt_flag = 0;
    }

    rk_print_string("pwm IRQ test over.\r\n");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDevShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:54:08
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellBsp(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellPwmBspName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellPwmBspName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                        //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellPwmBspName[i].CmdDes, pItem);
    if(ShellPwmBspName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellPwmBspName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;
}
#endif

/*******************************************************************************
** Name: PwmDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellTest(HDC dev, uint8 * pstr)
{

    // TODO:
    //add other code below:
    //...

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    rk_printf ("Enrty Pwm test\n");
    PwmRateSet(0,20,PWM_FREQ);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellDel(HDC dev, uint8 * pstr)
{

    // TODO:
    //add other code below:
    //...
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDevShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellCreate(HDC dev, uint8 * pstr)
{

    // TODO:
    //add other code below:
    //...
    PWM_DEVICE_CLASS  PwmDev;
    DEVICE_CLASS      DevClass;
    uint32 DevID;
    uint32 channel;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    DevID = *pstr;
    channel = 0;

    PwmDev.stPwmDevice.DevID = DevID;
    PwmDev.channel = channel;

    //Create PWMDev...
    ret = RKDev_Create(DEV_CLASS_PWM, DevID, &PwmDev);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("PWMDev create failure\n");
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PwmDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 17:45:44
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_SHELL_
SHELL FUN rk_err_t PwmDevShellPcb(HDC dev, uint8 * pstr)
{

    // TODO:
    //add other code below:
    //...

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}

#endif
#endif

