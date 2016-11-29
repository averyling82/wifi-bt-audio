/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\Bcore\BcoreDevice.c
* Owner: aaron.sun
* Date: 2015.6.26
* Time: 10:18:52
* Version: 1.0
* Desc: bb system driver
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.26     10:18:52   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_BCORE_BCOREDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define BCORE_DEV_NUM 2

typedef  struct _BCORE_DEVICE_CLASS
{
    DEVICE_CLASS stBcoreDevice;
    pSemaphore osBcoreDebugSem;
    pSemaphore osBcoreOkSem;
    uint32 runing;
    uint32 Channel;

}BCORE_DEVICE_CLASS;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static BCORE_DEVICE_CLASS * gpstBcoreDevISR[BCORE_DEV_NUM];



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
rk_err_t BcoreDevCheckHandler(HDC dev);
rk_err_t BcoreTaskResume(HTC hTask);
rk_err_t BcoreTaskSuspend(HTC hTask, uint32 Level);
rk_err_t BcoreDevCheckIdle(HTC hTask);
rk_err_t BcoreDevShellTest(HDC dev, uint8 * pstr);
rk_err_t BcoreDevShellDel(HDC dev, uint8 * pstr);
rk_err_t BcoreDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t BcoreDevShellPcb(HDC dev, uint8 * pstr);
void BcoreDevIntIsr1(void);
void BcoreDevIntIsr0(void);
void BcoreDevIntIsr(uint32 DevID);
rk_err_t BcoreDevDeInit(BCORE_DEVICE_CLASS * pstBcoreDev);
rk_err_t BcoreDevInit(BCORE_DEVICE_CLASS * pstBcoreDev);
rk_err_t BcoreDevResume(HDC dev);
rk_err_t BcoreDevSuspend(HDC dev, uint32 Level);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: BcoreDev_IsRuning
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.12.9
** Time: 18:36:14
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON API rk_err_t BcoreDev_IsRuning(HDC dev)
{
    BCORE_DEVICE_CLASS * pstBcoreDev =  (BCORE_DEVICE_CLASS *)dev;
    if( pstBcoreDev->runing == 1)
    {
        return RK_SUCCESS;
    }
    return RK_ERROR;
}
/*******************************************************************************
** Name: BcoreDev_ShutOff
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:54:36
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON API rk_err_t BcoreDev_ShutOff(HDC dev)
{
   BCORE_DEVICE_CLASS * pstBcoreDev =  (BCORE_DEVICE_CLASS *)dev;
   ScuSoftResetCtr(CAL_CORE_SRST, TRUE);
   ScuClockGateCtr(HCLK_CAL_CORE_GATE,0);

   pstBcoreDev->runing = 0;

   DEBUG("shut off bb system OK");
}

/*******************************************************************************
** Name: BcoreDev_Task_Enter
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON API void BcoreDev_Task_Enter(void)
{
    //rk_printf("b core enter....");
    HTC hSelf;
    hSelf = RKTaskGetRunHandle();
    while(1)
    {
        rkos_semaphore_take(gpstBcoreDevISR[0]->osBcoreDebugSem,MAX_DELAY);

        if(BcoreDevCheckIdle(hSelf) != RK_SUCCESS)
        {
            continue;
        }

        #ifdef _LOG_DEBUG_
        BBDebug();
        #endif
    }
}

/*******************************************************************************
** Name: BcoreDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON API rk_err_t BcoreDev_Write(HDC dev, uint32 Cmd, uint32 Data, uint32 port)
{
    //BcoreDev Write...
    BCORE_DEVICE_CLASS * pstBcoreDev =  (BCORE_DEVICE_CLASS *)dev;
    if(pstBcoreDev == NULL)
    {
        return RK_ERROR;
    }

}
/*******************************************************************************
** Name: BcoreDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON API rk_err_t BcoreDev_Start(HDC dev, uint32 ModuleId)
{
    BCORE_DEVICE_CLASS * pstBcoreDev =  (BCORE_DEVICE_CLASS *)dev;


    ScuClockGateCtr(HCLK_CAL_CORE_GATE,1);

    ScuSoftResetCtr(CAL_CORE_SRST, TRUE);

    FW_LoadSegment(ModuleId, SEGMENT_OVERLAY_ALL);

    memcpy((void *)0x01020000, &chip_freq, sizeof(chip_freq_t));

    ScuSoftResetCtr(CAL_CORE_SRST, FALSE);

    DEBUG("start bb system...");

    rkos_semaphore_take(pstBcoreDev->osBcoreOkSem, MAX_DELAY);

    #ifdef BB_SYS_JTAG
    memcpy((void *)0x01020000, &chip_freq, sizeof(chip_freq_t));

    rkos_semaphore_take(pstBcoreDev->osBcoreOkSem, MAX_DELAY);

    #endif

    #ifdef _LOG_DEBUG_
    DEBUG("start bb system OK");
    #endif

    pstBcoreDev->runing = 1;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: BcoreDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.15
** Time: 14:20:49
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON FUN rk_err_t BcoreDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < BCORE_DEV_NUM; i++)
    {
        if(gpstBcoreDevISR[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: BcoreTaskResume
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 16:06:16
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON FUN rk_err_t BcoreTaskResume(HTC hTask)
{
   RK_TASK_CLASS*   pBcoreTask = (RK_TASK_CLASS*)hTask;
   pBcoreTask->State = TASK_STATE_WORKING;
   return RK_SUCCESS;
}

/*******************************************************************************
** Name: BcoreTaskSuspend
** Input:HTC hTask, uint32 Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 16:05:14
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON FUN rk_err_t BcoreTaskSuspend(HTC hTask, uint32 Level)
{
    RK_TASK_CLASS*   pBcoreTask = (RK_TASK_CLASS*)hTask;

    if(Level == TASK_STATE_IDLE1)
    {
        pBcoreTask->State = TASK_STATE_IDLE1;
    }
    else if(Level == TASK_STATE_IDLE2)
    {
        pBcoreTask->State = TASK_STATE_IDLE2;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: BcoreDevCheckIdle
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 19:11:37
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON FUN rk_err_t BcoreDevCheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        if(BcoreTaskResume(hTask) != RK_SUCCESS)
        {
            return RK_ERROR;
        }
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: BcoreDevIntIsr1
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON FUN void BcoreDevIntIsr1(void)
{
    //Call total int service...
    BcoreDevIntIsr(1);

}
/*******************************************************************************
** Name: BcoreDevIntIsr0
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON FUN void BcoreDevIntIsr0(void)
{
    //Call total int service...
    BcoreDevIntIsr(0);

}
/*******************************************************************************
** Name: BcoreDevIntIsr
** Input:uint32 DevID
** Return: void
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_COMMON_
COMMON FUN void BcoreDevIntIsr(uint32 DevID)
{
    uint32 BcoreDevIntType;


    //Get BcoreDev Int type...
    //BcoreDevIntType = GetIntType();
    if (gpstBcoreDevISR[DevID] != NULL)
    {
        uint32 Cmd;
        uint32 Data;

        MailBoxClearB2AInt(MAILBOX_ID_0,  MAILBOX_INT_0);
        Cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_0);

        switch(Cmd)
        {
            case MSGBOX_CMD_SYSTEM_PRINT_LOG:
                #ifdef _LOG_DEBUG_
                BBReqDebug((uint8 *)MailBoxReadB2AData(MAILBOX_ID_0,MAILBOX_CHANNEL_0));
                rkos_semaphore_give_fromisr(gpstBcoreDevISR[DevID]->osBcoreDebugSem);
                #endif

                break;

            case MSGBOX_CMD_SYSTEM_START_OK:
                rkos_semaphore_give_fromisr(gpstBcoreDevISR[DevID]->osBcoreOkSem);
                break;
        }
    }

}

/*******************************************************************************
** Name: BcoreDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
COMMON FUN rk_err_t BcoreDevResume(HDC dev)
{
    BCORE_DEVICE_CLASS * pstBcoreDev = (BCORE_DEVICE_CLASS *)dev;
    if(BcoreDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    BcoreHwInit(pstBcoreDev->stBcoreDevice.DevID, pstBcoreDev->Channel);
    BcoreDevInit(pstBcoreDev);
    pstBcoreDev->stBcoreDevice.State = DEV_STATE_WORKING;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: BcoreDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
COMMON FUN rk_err_t BcoreDevSuspend(HDC dev, uint32 Level)
{
    BCORE_DEVICE_CLASS * pstBcoreDev = (BCORE_DEVICE_CLASS *)dev;
    if(BcoreDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstBcoreDev->stBcoreDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstBcoreDev->stBcoreDevice.State = DEV_SATE_IDLE2;
    }

    BcoreHwDeInit(pstBcoreDev->stBcoreDevice.DevID, pstBcoreDev->Channel);
    BcoreDevDeInit(pstBcoreDev);
    return RK_SUCCESS;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: BcoreDev_Task_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
INIT API rk_err_t BcoreDev_Task_DeInit(void *pvParameters)
{
    RK_TASK_CLASS* pBcoreDevTask = (RK_TASK_CLASS*)pvParameters;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: BcoreDev_Task_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
INIT API rk_err_t BcoreDev_Task_Init(void *pvParameters)
{
    RK_TASK_CLASS* pBcoreDevTask = (RK_TASK_CLASS*)pvParameters;
    pBcoreDevTask->Idle1EventTime = 10 * PM_TIME;
    pBcoreDevTask->TaskSuspendFun = BcoreTaskSuspend;
    pBcoreDevTask->TaskResumeFun = BcoreTaskResume;
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: BcoreDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
INIT API rk_err_t BcoreDev_Delete(uint32 DevID, void * arg)
{
    //Check BcoreDev is not exist...
    if(gpstBcoreDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    RKTaskDelete(TASK_ID_BCORE, 0, SYNC_MODE);

    //BcoreDev deinit...
    BcoreHwDeInit(DevID, gpstBcoreDevISR[DevID]->Channel);
    BcoreDevDeInit(gpstBcoreDevISR[DevID]);

    if(gpstBcoreDevISR[DevID]->osBcoreDebugSem)
        rkos_semaphore_delete(gpstBcoreDevISR[DevID]->osBcoreDebugSem);
    if(gpstBcoreDevISR[DevID]->osBcoreOkSem)
        rkos_semaphore_delete(gpstBcoreDevISR[DevID]->osBcoreOkSem);

    //Free BcoreDev memory...

    rkos_memory_free(gpstBcoreDevISR[DevID]);

    //Delete BcoreDev...
    gpstBcoreDevISR[DevID] = NULL;

    //Delete BcoreDev Read and Write Module...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_BCORE_DEV);
#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: BcoreDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
INIT API HDC BcoreDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS* pstDev;
    BCORE_DEVICE_CLASS * pstBcoreDev;

    // Create handler...
    pstBcoreDev =  rkos_memory_malloc(sizeof(BCORE_DEVICE_CLASS));
    memset(pstBcoreDev, 0, sizeof(BCORE_DEVICE_CLASS));
    if (pstBcoreDev == NULL)
    {
        return NULL;
    }

    //init handler...
    pstBcoreDev->osBcoreDebugSem  = rkos_semaphore_create(1,0);
    pstBcoreDev->osBcoreOkSem  = rkos_semaphore_create(1,0);

    if(((pstBcoreDev->osBcoreDebugSem) == 0)
        || ((pstBcoreDev->osBcoreOkSem) == 0))
    {
        rkos_semaphore_delete(pstBcoreDev->osBcoreDebugSem);
        rkos_semaphore_delete(pstBcoreDev->osBcoreOkSem);
        rkos_memory_free(pstBcoreDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstBcoreDev;
    pstDev->suspend = BcoreDevSuspend;
    pstDev->resume  = BcoreDevResume;
    pstDev->SuspendMode = ENABLE_MODE;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_BCORE_DEV, SEGMENT_OVERLAY_CODE);
#endif

    //device init...
    gpstBcoreDevISR[DevID] = NULL;
    BcoreHwInit(DevID, pstBcoreDev->Channel);
    if(BcoreDevInit(pstBcoreDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstBcoreDev->osBcoreDebugSem);
        rkos_semaphore_delete(pstBcoreDev->osBcoreOkSem);
        rkos_memory_free(pstBcoreDev);
        return (HDC) RK_ERROR;
    }
    gpstBcoreDevISR[DevID] = pstBcoreDev;

    RKTaskCreate(TASK_ID_BCORE, 0, NULL, SYNC_MODE);

    return (HDC)pstDev;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: BcoreDevDeInit
** Input:BCORE_DEVICE_CLASS * pstBcoreDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
INIT FUN rk_err_t BcoreDevDeInit(BCORE_DEVICE_CLASS * pstBcoreDev)
{

    rk_printf("bcore device deinit");

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: BcoreDevInit
** Input:BCORE_DEVICE_CLASS * pstBcoreDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
INIT FUN rk_err_t BcoreDevInit(BCORE_DEVICE_CLASS * pstBcoreDev)
{
    return RK_SUCCESS;
}


#ifdef _BCORE_DEV_SHELL_
_DRIVER_BCORE_BCOREDEVICE_DATA_
static SHELL_CMD ShellBcoreName[] =
{
    "pcb",BcoreDevShellPcb,"list bcored device pcb information","bcore.pcb [bcore device object id]",
    "create",BcoreDevShellCreate,"create a bcore device","bcore.create [bcore device object id]",
    "delete",BcoreDevShellDel,"delete a bcore device","bcore.delete [bcore device object id]",
    "test",BcoreDevShellTest,"test bcore device","bcore.test [bcore device object id]",
    "\b",NULL,"NULL","NULL",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: BcoreDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_SHELL_
SHELL API rk_err_t BcoreDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellBcoreName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellBcoreName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellBcoreName[i].CmdDes, pItem);

    if(ShellBcoreName[i].ShellCmdParaseFun != NULL)
    {
        ShellBcoreName[i].ShellCmdParaseFun(dev, pItem);
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
/*******************************************************************************
** Name: BcoreDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_SHELL_
SHELL FUN rk_err_t BcoreDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hBcoreDev;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    //Get BcoreDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Open BcoreDev...
    hBcoreDev = RKDev_Open(DEV_CLASS_BCORE, DevID, NOT_CARE);
    if((hBcoreDev == NULL) || (hBcoreDev == (HDC)RK_ERROR) || (hBcoreDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("BcoreDev open failure");
         return RK_SUCCESS;
    }

    //do test....
    BcoreDev_Start(hBcoreDev, SEGMENT_ID_BB_CODE);

    //close BcoreDev...
    RKDev_Close(hBcoreDev);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: BcoreDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_SHELL_
SHELL FUN rk_err_t BcoreDevShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    //Get BcoreDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    rk_print_string("TASK_ID_BCORE success\n");

    if(RKDev_Delete(DEV_CLASS_BCORE, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("BCOREDev delete failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: BcoreDevShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_SHELL_
SHELL FUN rk_err_t BcoreDevShellCreate(HDC dev, uint8 * pstr)
{
    BCORE_DEV_ARG stBcoreDevArg;
    rk_err_t ret;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Init BcoreDev arg...
    //stBcoreDevArg.usbmode = USBOTG_MODE_DEVICE;

    //Create BcoreDev...
    ret = RKDev_Create(DEV_CLASS_BCORE, DevID, &stBcoreDevArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("BcoreDev create failure");
    }

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: BcoreDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 10:19:35
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_SHELL_
SHELL FUN rk_err_t BcoreDevShellPcb(HDC dev, uint8 * pstr)
{
    HDC hBcoreDev;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    //Get BcoreDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Display pcb...
    if(gpstBcoreDevISR[DevID] != NULL)
    {
        rk_print_string("rn");
        rk_printf_no_time("BcoreDev ID = %d Pcb list as follow:", DevID);
        rk_printf_no_time("    osBcoreDebugSem---0x%08x", gpstBcoreDevISR[DevID]->osBcoreDebugSem);
        rk_printf_no_time("    osBcoreOkSem---0x%08x", gpstBcoreDevISR[DevID]->osBcoreOkSem);
        //rk_printf_no_time("    usbmode---0x%08x", gpstUsbOtgDevISR[DevID]->usbmode);
        //rk_printf_no_time("    usbspeed---0x%08x", gpstUsbOtgDevISR[DevID]->usbspeed);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("BcoreDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;

}


#endif

#endif
