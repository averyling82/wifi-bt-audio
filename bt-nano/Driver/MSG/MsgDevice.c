/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\MSG\MsgDevice.c
* Owner: aaron.sun
* Date: 2015.6.23
* Time: 15:58:26
* Version: 1.0
* Desc: msg device class
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.23     15:58:26   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_MSG_MSGDEVICE_C__

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
#define MSG_DEV_NUM 1

typedef  struct _MSG_DEVICE_CLASS
{
    DEVICE_CLASS stMsgDevice;
    pSemaphore osMsgOperReqSem;
    uint32  Msg[(MSGDEV_MAX / 32) + 1];

}MSG_DEVICE_CLASS;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static MSG_DEVICE_CLASS * gpstMsgDevISR[MSG_DEV_NUM] = {(MSG_DEVICE_CLASS *)NULL};



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
rk_err_t MsgDevCheckHandler(HDC dev);
rk_err_t MsgDevShellTest(HDC dev, uint8 * pstr);
rk_err_t MsgDevShellDel(HDC dev, uint8 * pstr);
rk_err_t MsgDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t MsgDevShellPcb(HDC dev, uint8 * pstr);
void MsgDevIntIsr1(void);
void MsgDevIntIsr0(void);
void MsgDevIntIsr(uint32 DevID);
rk_err_t MsgDevDeInit(MSG_DEVICE_CLASS * pstMsgDev);
rk_err_t MsgDevInit(MSG_DEVICE_CLASS * pstMsgDev);
rk_err_t MsgDevResume(HDC dev);
rk_err_t MsgDevSuspend(HDC dev, uint32 Level);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MsgDev_ClearMsg
** Input:HDC dev, uint32 MsgID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.1.26
** Time: 16:37:29
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_COMMON_
COMMON API rk_err_t MsgDev_ClearMsg(HDC dev, uint32 MsgID)
{
     MSG_DEVICE_CLASS * pstMsgDev =  (MSG_DEVICE_CLASS *)dev;

    if(pstMsgDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstMsgDev->osMsgOperReqSem, MAX_DELAY);

    pstMsgDev->Msg[MsgID / 32] &= ~(1 << (MsgID % 32));

    rkos_semaphore_give(pstMsgDev->osMsgOperReqSem);
    return(RK_SUCCESS);
}

/*******************************************************************************
** Name: MsgDev_CheckMsg
** Input:HDC dev,  uint32 MsgID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 16:03:03
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_COMMON_
COMMON API rk_err_t MsgDev_CheckMsg(HDC dev,  uint32 MsgID)
{
    MSG_DEVICE_CLASS * pstMsgDev =  (MSG_DEVICE_CLASS *)dev;

    if(pstMsgDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstMsgDev->osMsgOperReqSem, MAX_DELAY);

    if (pstMsgDev->Msg[MsgID / 32] & (1 << (MsgID % 32)))
    {
        rkos_semaphore_give(pstMsgDev->osMsgOperReqSem);
        return(RK_SUCCESS);
    }

    rkos_semaphore_give(pstMsgDev->osMsgOperReqSem);
    return(RK_ERROR);
}

/*******************************************************************************
** Name: MsgDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_COMMON_
COMMON API rk_err_t MsgDev_SendMsg(HDC dev, uint32 MsgID)
{
    //MsgDev Write...
    MSG_DEVICE_CLASS * pstMsgDev =  (MSG_DEVICE_CLASS *)dev;

    if(pstMsgDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstMsgDev->osMsgOperReqSem, MAX_DELAY);

    pstMsgDev->Msg[MsgID / 32] |= 1 << (MsgID % 32);

    rkos_semaphore_give(pstMsgDev->osMsgOperReqSem);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MsgDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_COMMON_
COMMON API rk_err_t MsgDev_GetMsg(HDC dev, uint32 MsgID)
{
    //MsgDev Read...
    MSG_DEVICE_CLASS * pstMsgDev =  (MSG_DEVICE_CLASS *)dev;
    if(pstMsgDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstMsgDev->osMsgOperReqSem, MAX_DELAY);

    if (pstMsgDev->Msg[MsgID / 32] & (1 << (MsgID % 32)))
    {
        pstMsgDev->Msg[MsgID / 32] &= ~(1 << (MsgID % 32));

        rkos_semaphore_give(pstMsgDev->osMsgOperReqSem);
        return(RK_SUCCESS);
    }
    rkos_semaphore_give(pstMsgDev->osMsgOperReqSem);
    return(RK_ERROR);
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MsgDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 9:17:30
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_COMMON_
COMMON FUN rk_err_t MsgDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < MSG_DEV_NUM; i++)
    {
        if(gpstMsgDevISR[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: MsgDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_COMMON_
COMMON FUN rk_err_t MsgDevResume(HDC dev)
{
     MSG_DEVICE_CLASS * pstMsgDev = (MSG_DEVICE_CLASS *)dev;
     if(MsgDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }
     pstMsgDev->stMsgDevice.State = DEV_STATE_WORKING;

     return RK_SUCCESS;
}

/*******************************************************************************
** Name: MsgDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_COMMON_
COMMON FUN rk_err_t MsgDevSuspend(HDC dev, uint32 Level)
{
    MSG_DEVICE_CLASS * pstMsgDev = (MSG_DEVICE_CLASS *)dev;
    if(MsgDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstMsgDev->stMsgDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstMsgDev->stMsgDevice.State = DEV_SATE_IDLE2;
    }

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
** Name: MsgDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_INIT_
INIT API rk_err_t MsgDev_Delete(uint32 DevID, void * arg)
{
    //Check MsgDev is not exist...
    if(gpstMsgDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    //MsgDev deinit...
    MsgDevDeInit(gpstMsgDevISR[DevID]);

    //Free MsgDev memory...
    rkos_semaphore_delete(gpstMsgDevISR[DevID]->osMsgOperReqSem);

    rkos_memory_free(gpstMsgDevISR[DevID]);

    //Delete MsgDev...
    gpstMsgDevISR[DevID] = NULL;

    //Delete MsgDev Read and Write Module...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_MSG_DEV);
#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: MsgDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_INIT_
INIT API HDC MsgDev_Create(uint32 DevID, void * arg)
{
    MSG_DEV_ARG * pstMsgDevArg;
    DEVICE_CLASS* pstDev;
    MSG_DEVICE_CLASS * pstMsgDev;


    // Create handler...
    pstMsgDev =  rkos_memory_malloc(sizeof(MSG_DEVICE_CLASS));
    memset(pstMsgDev, 0, sizeof(MSG_DEVICE_CLASS));
    if (pstMsgDev == NULL)
    {
        return NULL;
    }

    //init handler...
    pstMsgDev->osMsgOperReqSem  = rkos_semaphore_create(1,1);

    if(pstMsgDev->osMsgOperReqSem == 0)
    {
        rkos_semaphore_delete(pstMsgDev->osMsgOperReqSem);
        rkos_memory_free(pstMsgDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstMsgDev;
    pstDev->suspend = MsgDevSuspend;
    pstDev->resume  = MsgDevResume;
    pstDev->SuspendMode = ENABLE_MODE;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_MSG_DEV, SEGMENT_OVERLAY_CODE);
#endif
    //device init...
    gpstMsgDevISR[DevID] = NULL;
    //MsgHwInit(DevID, pstMsgDev->Channel);
    if(MsgDevInit(pstMsgDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstMsgDev->osMsgOperReqSem);
        rkos_memory_free(pstMsgDev);
        return (HDC) RK_ERROR;
    }
    gpstMsgDevISR[DevID] = pstMsgDev;
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
** Name: MsgDevDeInit
** Input:MSG_DEVICE_CLASS * pstMsgDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_INIT_
INIT FUN rk_err_t MsgDevDeInit(MSG_DEVICE_CLASS * pstMsgDev)
{
    //If ip device first step unregister int
    //IntDisable(INT_ID17_UART);
    //IntPendingClear(INT_ID17_UART);
    //IntUnregister(INT_ID17_UART);

    //If ip device second step close clk
    //ScuClockGateCtr(CLOCK_GATE_UART,0);
    return RK_ERROR;

}
/*******************************************************************************
** Name: MsgDevInit
** Input:MSG_DEVICE_CLASS * pstMsgDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_INIT_
INIT FUN rk_err_t MsgDevInit(MSG_DEVICE_CLASS * pstMsgDev)
{
    //If ip device first step open ip clk...
    //ScuClockGateCtr(CLOCK_GATE_UART, 1);
    //SetUartFreq(24);

    //If ip device second step rst ip...
    //ScuSoftResetCtr(RST_UART, 1);
    //DelayMs(1);
    //ScuSoftResetCtr(RST_UART, 0);

    //If ip device three step call ip bsp function init ip...
    //UARTInit(pstUartDev->dwBitRate,pstUartDev->dwBitWidth,UART_ONE_STOPBIT,UART_PARITY_DISABLE);

    //If ip device four step register intreput service...
    //IntRegister(INT_ID17_UART ,UartIntIsr0);
    //IntPendingClear(INT_ID17_UART);
    //IntEnable(INT_ID17_UART);
    //some ip device,such as uart ip device, only a read fifo and a write fifo,
    //we can use fifo as buffer for save some data when shut off Ip int
    return RK_SUCCESS;

}


#ifdef _MSG_DEV_SHELL_
_DRIVER_MSG_MSGDEVICE_SHELL_DATA_
static SHELL_CMD ShellMsgName[] =
{
    "pcb",MsgDevShellPcb,"list msg device pcb inf","msg.pcb [msg device object id]",
    "create",MsgDevShellCreate,"create a msg device","msg.create [msg device object id]",
    "del",MsgDevShellDel,"delete a msg device","msg.delete [msg device object id]",
    "test",MsgDevShellTest,"test msg device","msg.test [msg device object id]",
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
** Name: MsgDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_SHELL_
SHELL API rk_err_t MsgDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellMsgName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellMsgName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellMsgName[i].CmdDes, pItem);
    if(ShellMsgName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellMsgName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: MsgDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_SHELL_
SHELL FUN rk_err_t MsgDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hMsgDev;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    //Get MsgDev ID...
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

    //Open MsgDev...
    hMsgDev = RKDev_Open(DEV_CLASS_MSG, 0, NOT_CARE);
    if((hMsgDev == NULL) || (hMsgDev == (HDC)RK_ERROR) || (hMsgDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("MsgDev open failure");
         return RK_SUCCESS;
    }

    //do test....

    //close MsgDev...
    RKDev_Close(hMsgDev);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: MsgDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_SHELL_
SHELL FUN rk_err_t MsgDevShellDel(HDC dev, uint8 * pstr)
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

    //Get MsgDev ID...
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
    if(RKDev_Delete(DEV_CLASS_MSG, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("MSGDev delete failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: MsgDevShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_SHELL_
SHELL FUN rk_err_t MsgDevShellCreate(HDC dev, uint8 * pstr)
{
    MSG_DEV_ARG stMsgDevArg;
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

    //Init MsgDev arg...
    //stMsgDevArg.usbmode = USBOTG_MODE_DEVICE;

    //Create MsgDev...
    ret = RKDev_Create(DEV_CLASS_MSG, DevID, &stMsgDevArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("MsgDev create failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: MsgDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:59:32
*******************************************************************************/
_DRIVER_MSG_MSGDEVICE_SHELL_
SHELL FUN rk_err_t MsgDevShellPcb(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    MSG_DEVICE_CLASS * pstMsgDev;
    uint32 i;

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

    DevID = String2Num(pstr);

    if(DevID >= LUN_MAX_NUM)
    {
        return RK_ERROR;
    }

    pstMsgDev = gpstMsgDevISR[DevID];

    if(pstMsgDev == NULL)
    {
        rk_printf("msg%d in not exist", DevID);
        return RK_SUCCESS;
    }

    rk_printf_no_time(".gpstMsgDevISR[%d]", DevID);
    rk_printf_no_time("    .stMsgDevice");
    rk_printf_no_time("        .next = %08x",pstMsgDev->stMsgDevice.next);
    rk_printf_no_time("        .UseCnt = %d",pstMsgDev->stMsgDevice.UseCnt);
    rk_printf_no_time("        .SuspendCnt = %d",pstMsgDev->stMsgDevice.SuspendCnt);
    rk_printf_no_time("        .DevClassID = %d",pstMsgDev->stMsgDevice.DevClassID);
    rk_printf_no_time("        .DevID = %d",pstMsgDev->stMsgDevice.DevID);
    rk_printf_no_time("        .suspend = %08x",pstMsgDev->stMsgDevice.suspend);
    rk_printf_no_time("        .resume = %08x",pstMsgDev->stMsgDevice.resume);
    rk_printf_no_time("    .osMsgOperReqSem = %08x",pstMsgDev->osMsgOperReqSem);
    rk_printf_no_time("    .Msg = %08x",pstMsgDev->Msg);

    for(i = 0; i < (MSGDEV_MAX / 32) + 1; i++)
    {
        rk_printf_no_time("        .Msg[%d] = %08x",i, pstMsgDev->Msg[i]);
    }

    return RK_SUCCESS;
}


#endif

#endif
