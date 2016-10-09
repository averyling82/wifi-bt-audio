/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\Timer\TimerDevice.c
* Owner: zhuzhe
* Date: 2014.5.19
* Time: 11:00:06
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* zhuzhe     2014.5.19     11:00:06   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_TIMER_TIMERDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "BspConfig.h"
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"
#include "GUITask.h"


rk_err_t TimerShellTest (HDC dev, uint8* pstr, uint32 usTick);
rk_err_t TimerShellDel(HDC dev, uint8 * pstr);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _TIMER_DEVICE_CLASS
{
    DEVICE_CLASS stTimerDevice;
    pSemaphore osTimerReqSem;
    pFunc TimerCallBack;
    uint32 Clk;

    HDC    ADCHandle;

}TIMER_DEVICE_CLASS;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static volatile TIMER_DEVICE_CLASS * gpstTimerDevISRHandler[TIMER_MAX];



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
void Timer1_Test_ISR(void);
void Timer0_Test_ISR(void);
rk_err_t TimeDevShellBspDeinit(HDC dev, uint8 * pstr);
rk_err_t TimeDevShellBspTime_Test(HDC dev, uint8 * pstr);
rk_err_t TimeDevShellBsp(HDC dev, uint8 * pstr);
rk_err_t TimerDevDeInit(HDC dev);
rk_err_t TimerDevInit(HDC dev);
void TimerIntIsr(uint32 timerNum);
void TimerDev_CallBack(void);
rk_err_t TimerShellCreate(HDC dev, uint8 * pstr);
//void TimerIntIsr0(void);
//void TimerIntIsr1(void);




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(Common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: TimerDev_Task_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.17
** Time: 20:15:34
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
COMMON API rk_err_t TimerDev_Task_Init(void *pvParameters)
{
    printf("enter TimerDev_Task_Init\n");
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: TimerDev_Task_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.17
** Time: 20:15:34
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
COMMON API rk_err_t TimerDev_Task_DeInit(void *pvParameters)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: TimerDev_Connect
** Input:HDC dev, uint32 Period
** Return: rk_err_t
** Owner:cjh
** Date: 2014.10.10
** Time: 14:28:54
*******************************************************************************/
/*
_DRIVER_TIMER_TIMERDEVICE_COMMON_
COMMON API rk_err_t TimerDev_Connect(HDC dev, uint32 tmerNum, uint32 TimerClk, uint64 usTick)
{
    TIMER_DEVICE_CLASS * pstTimergDev = (TIMER_DEVICE_CLASS *)dev;

    //timer interupt Period
    //printf("Timer task Connect\n");
    TimerDev_PeriodSet(pstTimergDev, usTick, TimerClk);
    //printf("Timer task TimerDev_PeriodSet over\n");
    RKTaskCreate(TASK_ID_TIMER, tmerNum, NULL, SYNC_MODE);

    //printf("Timer task Connect success\n");
    return RK_SUCCESS;
}
*/

/*******************************************************************************
** Name: TimerDev_Task_Enter
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.6.17
** Time: 20:15:34
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
COMMON API void TimerDev_Task_Enter(void * arg)
{
    pFunc CallBack;

    while (1)
    {
        rkos_semaphore_take(gpstTimerDevISRHandler[(uint32)arg]->osTimerReqSem,MAX_DELAY);
        CallBack = gpstTimerDevISRHandler[(uint32)arg]->TimerCallBack;
        if (CallBack)
        {
            CallBack();
        }
        //printf("timer task serice...\n");
    }
}

/*******************************************************************************
** Name: TimerDev_PeriodSet
** Input:HDC dev,pFunc TimerCallBack
** Return: rk_err_t
** Owner:cjh
** Date: 2014.5.21
** Time: 11:25:54
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
rk_err_t TimerDev_PeriodSet(HDC dev,UINT64 usTick, uint32 timerClk)
{
    UINT64 count,Timeclk,count_check;
    BOOL Retval = TRUE;
    TIMER_DEVICE_CLASS * pstTimerDevHandler = (TIMER_DEVICE_CLASS *)dev;

    //SetTimerFreq(timerNum,XIN24M,24*1000*1000);
    rk_printf("SetTimerFreq timerClk=%d\n", timerClk);
    SetTimerFreq(pstTimerDevHandler->stTimerDevice.DevID ,XIN24M,timerClk);
    Timeclk  = GetTimerFreq(pstTimerDevHandler->stTimerDevice.DevID );
    //1s
    count =  Timeclk * usTick/1000/1000;

    TimerStop (pstTimerDevHandler->stTimerDevice.DevID );

    TimerInit(pstTimerDevHandler->stTimerDevice.DevID ,TIMER_FREE_RUNNING);

   /* free mode and enable the timer  £¬not mask*/
    TimerSetCount(pstTimerDevHandler->stTimerDevice.DevID ,count);

    TimerIntUnmask(pstTimerDevHandler->stTimerDevice.DevID );

    //TimerDev_Start(pstTimerDevHandler);

    return Retval;
}

/*******************************************************************************
** Name: TimerDev_UnRegister
** Input:HDC dev,pFunc TimerCallBack
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.21
** Time: 11:25:54
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
rk_err_t TimerDev_UnRegister(HDC dev, uint16 channel)
{
    TIMER_DEVICE_CLASS * pstTimerDevHandler;

    pstTimerDevHandler = (TIMER_DEVICE_CLASS*)dev;

    TimerDev_Stop(pstTimerDevHandler);
    if(pstTimerDevHandler->TimerCallBack)
    {
        rk_printf("TimerDev_UnRegister\n");
        pstTimerDevHandler->TimerCallBack = NULL;
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;

    }
}

/*******************************************************************************
** Name: TimerDev_Register
** Input:HDC dev,uint32 clk,pFuncTimerCallBack
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.21
** Time: 11:24:35
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
rk_err_t TimerDev_Register(HDC dev, UINT64 usTick, uint32 clk,pFunc TimerCallBack)
{
    TIMER_DEVICE_CLASS * pstTimerDevHandler;

    pstTimerDevHandler = (TIMER_DEVICE_CLASS*)dev;
    //rkos_semaphore_take(pstTimerDevHandler->osTimerReqSem,MAX_DELAY);

    pstTimerDevHandler->TimerCallBack = TimerCallBack;
    pstTimerDevHandler->Clk = clk;
    TimerDev_PeriodSet(pstTimerDevHandler, usTick, clk);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: TimerDev_Clear
** Input:HDC dev
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.21
** Time: 11:24:00
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
rk_err_t TimerDev_Clear(HDC dev)
{
    TIMER_DEVICE_CLASS * pstTimerDevHandler;
    DEVICE_CLASS* pstTimerPublicHandler;

    pstTimerDevHandler = (TIMER_DEVICE_CLASS*)dev;
    pstTimerPublicHandler = (DEVICE_CLASS*)dev;

    rkos_enter_critical();
    IntPendingClear(pstTimerPublicHandler->DevID);
    rkos_exit_critical();
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: TimerDev_Stop
** Input:HDC dev
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.21
** Time: 11:23:17
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
rk_err_t  TimerDev_Stop(HDC dev)
{
    TIMER_DEVICE_CLASS * pstTimerDevHandler;
    DEVICE_CLASS* pstTimerPublicHandler;

    pstTimerDevHandler = (TIMER_DEVICE_CLASS*)dev;
    pstTimerPublicHandler = &(pstTimerDevHandler->stTimerDevice);
    printf("!!Stop timerdev=%d\n",pstTimerPublicHandler->DevID);
    //TimerStop(pstTimerPublicHandler->DevID);
    rkos_enter_critical();
    if(pstTimerPublicHandler->DevID == TIMER0)
        IntDisable(INT_ID_TIMER0);
    else
        IntDisable(INT_ID_TIMER1);
    rkos_exit_critical();

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: TimerDev_Start
** Input:HDC dev
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.21
** Time: 11:22:48
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
rk_err_t  TimerDev_Start(HDC dev)
{
    TIMER_DEVICE_CLASS * pstTimerDevHandler;
    DEVICE_CLASS* pstTimerPublicHandler;

    pstTimerDevHandler = (TIMER_DEVICE_CLASS *)dev;
    pstTimerPublicHandler = &(pstTimerDevHandler->stTimerDevice);
    TimerStart(pstTimerPublicHandler->DevID);

    return RK_SUCCESS;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TimerIntIsr0
** Input:void
** Return: void
** Owner:zhuzhe
** Date: 2014.5.19
** Time: 11:08:25
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
void TimerIntIsr0(void)
{
    TimerIntIsr(TIMER0);
}

/*******************************************************************************
** Name: TimerIntIsr1
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.5.12
** Time: 14:57:25
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
void TimerIntIsr1(void)
{
    TimerIntIsr(TIMER1);
}

/*******************************************************************************
** Name: TimerIntIsr
** Input:uint32 timerNum
** Return: void
** Owner:cjh
** Date: 2015.6.19
** Time: 9:57:25
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_COMMON_
void TimerIntIsr(uint32 timerNum)
{
    TimerClrInt(timerNum);
    rkos_semaphore_give_fromisr(gpstTimerDevISRHandler[timerNum]->osTimerReqSem);
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
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
/*******************************************************************************
** Name: TimerDevCreate
** Input:void
** Return: HDC
** Owner:zhuzhe
** Date: 2014.5.19
** Time: 11:08:25
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_INIT_
HDC TimerDev_Create(uint32 DevID, void * arg)
{
    //DEVICE_CLASS* pstTimerPublicHandler;
    DEVICE_CLASS* pstDev;
    TIMER_DEVICE_CLASS * pstTimerDevHandler;

    pstTimerDevHandler =  rkos_memory_malloc(sizeof(TIMER_DEVICE_CLASS));
    memset(pstTimerDevHandler, 0, sizeof(TIMER_DEVICE_CLASS));
    if (pstTimerDevHandler == NULL)
    {
        return NULL;
    }
    //init handler...
    pstTimerDevHandler->osTimerReqSem = rkos_semaphore_create(1,1);
    if ((pstTimerDevHandler->osTimerReqSem) == 0)//|| ((pstTimerDevHandler->osTimerReqSem) == 0)
    {
        rkos_semaphore_delete(pstTimerDevHandler->osTimerReqSem);

        rkos_memory_free(pstTimerDevHandler);
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstTimerDevHandler;
    pstDev->suspend = TimerDev_Suspend;
    pstDev->resume  = TimerDev_Resume;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_TIMER_DEV, SEGMENT_OVERLAY_CODE);
#endif

    gpstTimerDevISRHandler[DevID] = NULL;

    TimerDevHwInit(DevID, 0);
    pstTimerDevHandler->stTimerDevice.DevID = DevID;
    if (TimerDevInit(pstTimerDevHandler) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstTimerDevHandler->osTimerReqSem);
        //rkos_semaphore_delete(pstTimerDevHandler->osTimerReqSem);
        rkos_memory_free(pstTimerDevHandler);

        printf("init TimmerDev fail");
        return (HDC) RK_ERROR;
    }
    if (DevID < TIMER_MAX)
    {
        gpstTimerDevISRHandler[DevID] = pstTimerDevHandler;
        RKTaskCreate(TASK_ID_TIMER, DevID, (void *)DevID, SYNC_MODE);
    }
    else
    {
        rk_printf("timer%d no exist\n",DevID);
        rkos_semaphore_delete(pstTimerDevHandler->osTimerReqSem);
        rkos_memory_free(pstTimerDevHandler);
        return (HDC) RK_ERROR;
    }


    return (HDC)pstTimerDevHandler;
}

/*******************************************************************************
** Name: TimerDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.17
** Time: 20:15:34
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_INIT_
INIT API rk_err_t TimerDev_Delete(uint32 DevID, void * arg)
{
    //Check TimerDev is not exist...
    if(gpstTimerDevISRHandler[DevID] == NULL)
    {
        return RK_ERROR;
    }
    TimerDev_UnRegister(gpstTimerDevISRHandler[DevID],NULL);
    //TimerDev deinit...
    TimerDevDeInit(gpstTimerDevISRHandler[DevID]);
    rkos_semaphore_delete(gpstTimerDevISRHandler[DevID]->osTimerReqSem);
    rkos_memory_free(gpstTimerDevISRHandler[DevID]);
    gpstTimerDevISRHandler[DevID] = NULL;

    //Delete TimerDev Read and Write Module...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_TIMER_DEV);
#endif

    return RK_SUCCESS;

}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TimerDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.19
** Time: 11:08:25
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_INIT_
rk_err_t TimerDev_Resume(HDC dev)
{
    //TimerDev Resume...
    TIMER_DEVICE_CLASS * TimerDev =  (TIMER_DEVICE_CLASS *)dev;
    if (TimerDev == NULL)
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: TimerDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.19
** Time: 11:08:25
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_INIT_
rk_err_t TimerDev_Suspend(HDC dev)
{
    //TimerDev Suspend...
    TIMER_DEVICE_CLASS * TimerDev =  (TIMER_DEVICE_CLASS *)dev;
    if (TimerDev == NULL)
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: TimerDevDeInit
** Input:HDC dev
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.19
** Time: 11:08:25
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_INIT_
rk_err_t TimerDevDeInit(HDC dev)
{
    TIMER_DEVICE_CLASS * pstTimerDevHandler;

    pstTimerDevHandler = (TIMER_DEVICE_CLASS*)dev;

    //rk_printf("RKTaskDelete timer%d\n",pstTimerDevHandler->stTimerDevice.DevID);
    RKTaskDelete(TASK_ID_TIMER, pstTimerDevHandler->stTimerDevice.DevID, SYNC_MODE);
    TimerDev_PeriodSet(pstTimerDevHandler, 0, 0);

    IntPendingClear(FAULT_ID15_SYSTICK);
    IntUnregister(FAULT_ID15_SYSTICK);
    //TimerPeriodSet(0);
    IntDisable(FAULT_ID15_SYSTICK);
    ScuClockGateCtr( PCLK_TIMER_GATE, 0);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: TimerDevInit
** Input:HDC dev
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.19
** Time: 11:08:25
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_INIT_
rk_err_t TimerDevInit(HDC dev)
{
    DEVICE_CLASS* pstTimerPublicHandler;
    TIMER_DEVICE_CLASS * pstTimerDevHandler;

    pstTimerDevHandler = (TIMER_DEVICE_CLASS *)dev;
    pstTimerPublicHandler = &(pstTimerDevHandler->stTimerDevice);
    //TimerDev_PeriodSet(pstTimerDevHandler, 1000*1000, 24*1000*1000);//start
    return RK_SUCCESS;
}

#ifdef _TIMER_DEV_SHELL_

_DRIVER_TIMER_TIMERDEVICE_SHELL_DATA_
static SHELL_CMD ShellTimeName[] =
{
    //"pcb",
#ifdef SHELL_BSP
    "bsp",TimeDevShellBsp,"NULL","NULL",
#endif
    "create",TimerShellCreate,"NULL","NULL",
    "test",TimerShellTest,"NULL","NULL",
    "delete",TimerShellDel,"NULL","NULL",
    "\b",NULL,"NULL","NULL",                       // the end
};

#ifdef SHELL_BSP
static SHELL_CMD ShellTimeBspName[] =
{
    "test",TimeDevShellBspTime_Test,"NULL","NULL",
    "deinit",TimeDevShellBspDeinit,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};
#endif
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TimerDev_Shell
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 16:40:07
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_SHELL_
SHELL API rk_err_t TimerDev_Shell(HDC dev,  uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t  ret;

    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellTimeName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                  //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellTimeName[i].CmdDes, pItem);
    if(ShellTimeName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellTimeName[i].ShellCmdParaseFun(dev, pItem);
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
_DRIVER_TIMER_TIMERDEVICE_SHELL_ HDC hAudio;
_DRIVER_TIMER_TIMERDEVICE_SHELL_ __align(4) uint32 DmaBuf[2048];

/*******************************************************************************
** Name: TimerDev_CallBack
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.6.19
** Time: 10:13:39
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_SHELL_
COMMON FUN void TimerDev_CallBack(void)
{
    rk_count_clk_start();
    #ifdef __DRIVER_AUDIO_AUDIODEVICE_C__
    AudioDev_Write(hAudio, 0, (uint8 *)DmaBuf);
    #endif
    rk_count_clk_end();
}



/*******************************************************************************
** Name: TimerShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.11
** Time: 20:08:42
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_SHELL_
SHELL FUN rk_err_t TimerShellCreate(HDC dev, uint8 * pstr)
{
    rk_err_t ret;
    HDC hTimerDev;
    //KEY_DEV_ARG stKeyArg;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    {
        //audio test
        #ifdef __DRIVER_AUDIO_AUDIODEVICE_C__
        hAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);
        AudioDev_GetMainTrack(hAudio);
        AudioDev_SetChannel(hAudio, 0, 2);
        AudioDev_SetSampleRate(hAudio, 0, CodecFS_192KHz);
        AudioDev_SetTrackLen(hAudio, 768);
        AudioDev_SetBit(hAudio, 0, 24);
        AudioDev_SetVol(hAudio, 25);
        #endif
    }

    {        //rk_printf("timer pstr = %s\n",pstr);
        if (StrCmpA((uint8 *) pstr, "/0", 2) == 0)
        {
            ret = RKDev_Create(DEV_CLASS_TIMER,TIMER0,NULL);
            if (ret != RK_SUCCESS)
            {
                printf("TIMER0 CREATE FRIL\n");
            }
            else
            {
                printf("TIMER0 CREATE SUCCESS\n");
            }

            hTimerDev =  RKDev_Open(DEV_CLASS_TIMER,TIMER0,NOT_CARE);
            if(hTimerDev != NULL)
            {
                printf("TimerDev0 open success\n");
            }
            TimerDev_Register(hTimerDev, 100*1000, 24*1000*1000, TimerDev_CallBack);
            TimerDev_Start(hTimerDev);

        }
        else if (StrCmpA((uint8 *) pstr, "/1", 2) == 0)
        {
            ret = RKDev_Create(DEV_CLASS_TIMER,TIMER1,NULL);
            if (ret != RK_SUCCESS)
            {
                printf("TIMER1 CREATE FRIL\n");
            }
            else
            {
                printf("TIMER1 CREATE SUCCESS\n");
            }

            hTimerDev =  RKDev_Open(DEV_CLASS_TIMER,TIMER1,NOT_CARE);
            if(hTimerDev != NULL)
            {
                printf("TimerDev1 open success\n");
            }
            TimerDev_Register(hTimerDev, 2*1000*1000, 24*1000*1000, TimerDev_CallBack);
            TimerDev_Start(hTimerDev);
        }

    }
    RKDev_Close(hTimerDev);
    //printf("TIMER CREATE SUCCESS\n");
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdioDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_SHELL_
SHELL FUN rk_err_t TimerShellDel(HDC dev, uint8 * pstr)
{

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    {
        rk_printf("timer pstr = %s\n",pstr);
        if (StrCmpA((uint8 *) pstr, "/0", 2) == 0)
        {
            if (RKDev_Delete(DEV_CLASS_TIMER, 0, NULL) != RK_SUCCESS)
            {
                printf("TimerDev0 delete failure\n");
                return RK_SUCCESS;
            }
        }
        else if (StrCmpA((uint8 *) pstr, "/1", 2) == 0)
        {
            if (RKDev_Delete(DEV_CLASS_TIMER, 1, NULL) != RK_SUCCESS)
            {
                printf("TimerDev1 delete failure\n");
                return RK_SUCCESS;
            }

        }

    }
    printf("TimerDev Delete over\n");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: TimerShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.12
** Time: 14:30:36
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_SHELL_
SHELL FUN rk_err_t TimerShellTest (HDC dev, uint8* pstr, uint32 usTick)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    return RK_SUCCESS;
}

#ifdef SHELL_BSP

/*******************************************************************************
** Name: TimeDevShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 17:53:20
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_SHELL_
SHELL FUN rk_err_t TimeDevShellBsp(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

    uint8 Space;

    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellTimeBspName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                                 //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellTimeBspName[i].CmdDes, pItem);
    if(ShellTimeBspName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellTimeBspName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;
}

uint32 TIME_intstatus;

/*******************************************************************************
** Name: Timer1_Test_ISR
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.3
** Time: 15:55:11
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_SHELL_
SHELL FUN void Timer1_Test_ISR(void)
{
    pFunc callback;
    pTIMER_REG timerReg;
    TIME_intstatus = 0;
    printf("\r\n ------------Timer11111_Test_ISR------------\n");
    TimerClrInt(TIMER1);
    //TimerStop(TIMER1);
    //callback = g_timerIRQ[0];
    //if (callback)
    {
        //callback();
    }
}
/*******************************************************************************
** Name: Timer0_Test_ISR
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.3
** Time: 15:52:18
*******************************************************************************/

_DRIVER_TIMER_TIMERDEVICE_SHELL_
SHELL FUN void Timer0_Test_ISR(void)
{
    pFunc callback;
    pTIMER_REG timerReg;
    TIME_intstatus = 0;
    printf("\r\n ------------Timer00000_Test_ISR------------\n");
    TimerClrInt(TIMER0);
    //TimerStop(TIMER0);
    //callback = g_timerIRQ[0];
    //if (callback)
    {
        //callback();
    }
}

/*******************************************************************************
** Name: TimeDevShellBspDeinit
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 17:56:41
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_SHELL_
SHELL FUN rk_err_t TimeDevShellBspDeinit(HDC dev, uint8 * pstr)
{

}

/*******************************************************************************
** Name: TimeDevShellBspTime_Test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 17:55:58
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_SHELL_
SHELL FUN rk_err_t TimeDevShellBspTime_Test(HDC dev, uint8 * pstr)
{
    uint64 loadcount = 0;
    uint32 loadCounthigh = 0,loadCountlow = 0;

    uint32 time_reg_num;
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 5*1000000;      //5s
        printf("\r\n Time0 5s start 0\n");
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 10*1000000;      //10s
        printf("\r\n Time0 10s start 0\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 15*1000000;      //15s
        printf("\r\n Time0 15s start 0\n");
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 20*1000000;      //20s
        printf("\r\n Time0 20s start 0\n");
    }
    else if(StrCmpA(pstr, "4", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 5400000000;      //5400s
        printf("\r\n Time0 5400s start 0\n");
    }
    //---------------------------------------------------------------//
    else if(StrCmpA(pstr, "5", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 5*1000000;      //5s
        printf("\r\n Time1 5s start 0\n");
    }
    else if(StrCmpA(pstr, "6", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 10*1000000;      //10s
        printf("\r\n Time1 10s start 0\n");
    }
    else if(StrCmpA(pstr, "7", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 15*1000000;      //15s
        printf("\r\n Time1 15s start 0\n");
    }
    else if(StrCmpA(pstr, "8", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 20*1000000;      //20s
        printf("\r\n Time1 20s start 0\n");
    }
    else if(StrCmpA(pstr, "9", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 5400000000;      //5400s
        printf("\r\n Time1 5400s start 0\n");
    }

    TIME_intstatus = 1;

    if(0 == time_reg_num)
    {
        IntUnregister(INT_ID_TIMER0);
        IntPendingClear(INT_ID_TIMER0);
        IntDisable(INT_ID_TIMER0);

        TimerStop(TIMER0);
        TimerInit(TIMER0,TIMER_FREE_RUNNING);

        IntRegister(INT_ID_TIMER0 ,Timer0_Test_ISR);
        IntPendingClear(INT_ID_TIMER0);
        IntEnable(INT_ID_TIMER0);

        TimerSetCount(TIMER0,loadcount);

        TimerIntUnmask(TIMER0);

        TimerStart(TIMER0);
    }
    else if(1 == time_reg_num)
    {
        IntUnregister(INT_ID_TIMER1);
        IntPendingClear(INT_ID_TIMER1);
        IntDisable(INT_ID_TIMER1);

        TimerStop(TIMER1);
        TimerInit(TIMER1,TIMER_FREE_RUNNING);

        IntRegister(INT_ID_TIMER1 ,Timer1_Test_ISR);
        IntPendingClear(INT_ID_TIMER1);
        IntEnable(INT_ID_TIMER1);

        TimerSetCount(TIMER1,loadcount);

        TimerIntUnmask(TIMER1);

        TimerStart(TIMER1);
    }
    while(TIME_intstatus);

    printf("\r\ntime init over");

}
#endif



#endif

#endif



