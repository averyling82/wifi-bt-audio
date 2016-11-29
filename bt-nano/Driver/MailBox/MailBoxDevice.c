/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\MailBox\MailBoxDevice.c
* Owner: cjh
* Date: 2015.5.21
* Time: 9:06:51
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2015.5.21     9:06:51   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_MAILBOX_MAILBOXDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "Bsp.h"
#include "DeviceInclude.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define MAILBOX_DEV_NUM 4

typedef  struct _MAILBOX_DEVICE_CLASS
{
    DEVICE_CLASS stMailBoxDevice;
    xSemaphoreHandle osMailBoxReadReqSem;
    xSemaphoreHandle osMailBoxReadSem;
    xSemaphoreHandle osMailBoxWriteReqSem;
    xSemaphoreHandle osMailBoxWriteSem;
    xSemaphoreHandle osMailBoxOperReqSem;
    xSemaphoreHandle osMailBoxOperSem;
    uint32 MailBoxID;
    uint32 TypeA2BB2A;
    uint32 Cmd;
    uint32 Data;

}MAILBOX_DEVICE_CLASS;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static MAILBOX_DEVICE_CLASS * gpstMailBoxDevISR[MAILBOX_DEV_NUM] = {(MAILBOX_DEVICE_CLASS *)NULL,(MAILBOX_DEVICE_CLASS *)NULL
        ,(MAILBOX_DEVICE_CLASS *)NULL,(MAILBOX_DEVICE_CLASS *)NULL
                                                                   };
uint8 memDmabuf[1024];
uint8 memDmach = 0;
uint8 memDmafinish = 0;
uint8 BcoreDmabuf[1024];
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
rk_err_t MailBoxDevShellHelp(HDC dev, uint8 * pstr);
rk_err_t MailBoxDevShellWrite(HDC dev, uint8 * pstr);
rk_err_t MailBoxDevShellRead(HDC dev, uint8 * pstr);
rk_err_t MailBoxDevShellDMA_Write(HDC dev, uint8 * pstr);
rk_err_t MailBoxDevShellDMA_Read(HDC dev, uint8 * pstr);
rk_err_t MailBoxDevShellTest(HDC dev, uint8 * pstr);
rk_err_t MailBoxDevShellDel(HDC dev, uint8 * pstr);
rk_err_t MailBoxDevShellMc(HDC dev, uint8 * pstr);
rk_err_t MailBoxDevShellPcb(HDC dev, uint8 * pstr);
void MailBoxDevA2BIsr(uint32 DevID);
void MailBoxDevB2AIsr(uint32 DevID);
rk_err_t MailBoxDevDeInit(MAILBOX_DEVICE_CLASS * pstMailBoxDev);
rk_err_t MailBoxDevInit(MAILBOX_DEVICE_CLASS * pstMailBoxDev);
rk_err_t MailBoxDevResume(HDC dev);
rk_err_t MailBoxDevSuspend(HDC dev);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MailBoxDevA2BIsr3
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API void MailBoxDevA2BIsr3(void)
{
    //Call total int service...
    MailBoxDevA2BIsr(3);

}

/*******************************************************************************
** Name: MailBoxDevA2BIsr2
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API void MailBoxDevA2BIsr2(void)
{
    //Call total int service...
    MailBoxDevA2BIsr(2);

}

/*******************************************************************************
** Name: MailBoxDevA2BIsr1
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API void MailBoxDevA2BIsr1(void)
{
    //Call total int service...
    MailBoxDevA2BIsr(1);

}

/*******************************************************************************
** Name: MailBoxDevA2BIsr0
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API void MailBoxDevA2BIsr0(void)
{
    //Call total int service...
    MailBoxDevA2BIsr(0);

}

/*******************************************************************************
** Name: MailBoxDevB2AIsr3
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API void MailBoxDevB2AIsr3(void)
{
    //Call total int service...
    MailBoxDevB2AIsr(3);

}

/*******************************************************************************
** Name: MailBoxDevB2AIsr2
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API void MailBoxDevB2AIsr2(void)
{
    //Call total int service...
    MailBoxDevB2AIsr(2);

}

/*******************************************************************************
** Name: MailBoxDevB2AIsr1
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API void MailBoxDevB2AIsr1(void)
{
    //Call total int service...
    MailBoxDevB2AIsr(1);

}

/*******************************************************************************
** Name: MailBoxDevB2AIsr0
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API void MailBoxDevB2AIsr0(void)
{
    //Call total int service...
    MailBoxDevB2AIsr(0);

}

/*******************************************************************************
** Name: MailBoxDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:10
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API rk_err_t MailBoxDev_Write(HDC dev)
{
    uint32 cmd;
    uint32 data;
    uint32 typeA2BB2A;
    //MailBoxDev Write...
    MAILBOX_DEVICE_CLASS * pstMailBoxDev =  (MAILBOX_DEVICE_CLASS *)dev;
    cmd = pstMailBoxDev->Cmd;
    data = pstMailBoxDev->Data;
    typeA2BB2A = pstMailBoxDev->TypeA2BB2A;

    if (pstMailBoxDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstMailBoxDev->osMailBoxWriteReqSem, MAX_DELAY);


    switch (typeA2BB2A)
    {
        case MAILBOX_A2B_TYPE:
            rkos_semaphore_take(pstMailBoxDev->osMailBoxWriteSem, MAX_DELAY);

            MailBoxWriteA2BCmd(cmd, MAILBOX_ID_0, pstMailBoxDev->MailBoxID);
            MailBoxWriteA2BData(data, MAILBOX_ID_0, pstMailBoxDev->MailBoxID);
            break;

        case MAILBOX_B2A_TYPE:
            rkos_semaphore_take(pstMailBoxDev->osMailBoxWriteSem, MAX_DELAY);

            MailBoxWriteB2ACmd(cmd, MAILBOX_ID_0, pstMailBoxDev->MailBoxID);
            MailBoxWriteB2AData(data, MAILBOX_ID_0, pstMailBoxDev->MailBoxID);
            break;

        default:
            break;
    }

    rkos_semaphore_give(pstMailBoxDev->osMailBoxWriteReqSem);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: MailBoxDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:10
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON API rk_err_t MailBoxDev_Read(HDC dev)
{
    uint32 cmd;
    uint32 data;
    uint32 typeA2BB2A;
    //MailBoxDev Write...
    MAILBOX_DEVICE_CLASS * pstMailBoxDev =  (MAILBOX_DEVICE_CLASS *)dev;
    typeA2BB2A = pstMailBoxDev->TypeA2BB2A;
    //MailBoxDev Read...
    if (pstMailBoxDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstMailBoxDev->osMailBoxReadReqSem, MAX_DELAY);

    switch (typeA2BB2A)
    {
        case MAILBOX_A2B_TYPE:
            cmd = MailBoxReadA2BCmd(MAILBOX_ID_0, pstMailBoxDev->MailBoxID);
            data = MailBoxReadA2BData(MAILBOX_ID_0, pstMailBoxDev->MailBoxID);
            break;

        case MAILBOX_B2A_TYPE:
            cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, pstMailBoxDev->MailBoxID);
            data = MailBoxReadB2AData(MAILBOX_ID_0, pstMailBoxDev->MailBoxID);
            break;
        default:
            break;
    }
    rkos_enter_critical();
    pstMailBoxDev->Cmd = cmd;
    pstMailBoxDev->Data = data;
    rkos_exit_critical();

    //rkos_semaphore_take(pstMailBoxDev->osMailBoxReadSem, MAX_DELAY);
    rkos_semaphore_give(pstMailBoxDev->osMailBoxReadReqSem);

    rk_printf("MailBoxDev_Read cmd=%d, data=%d\n",cmd,data);
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
** Name: M2M_DmaIsr
** Input:uint32 DevID
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON FUN void MailBoxDev_M2M_DmaIsr(uint32 ch)
{
    rk_printf("dma complete\n");
    memDmafinish = 1;
    DmaIntIsr();
}

/*******************************************************************************
** Name: MailBoxDevIntIsr
** Input:uint32 DevID
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON FUN void MailBoxDevA2BIsr(uint32 DevID)
{
    uint32 cmd;
    uint32 data;
    uint32 TypeA2BB2A;
    int intstatus;
    uint32 IntEvent;
    MAILBOX_DEVICE_CLASS * pstMailBoxDev =  gpstMailBoxDevISR[DevID];
    IntEvent = MailBoxGetIntType(pstMailBoxDev->MailBoxID, MAILBOX_B2A_TYPE);

    switch (pstMailBoxDev->MailBoxID)
    {
        case MAILBOX_CHANNEL_0:
            if (IntEvent & MAILBOX_INT_0)
            {
                //cmd = MailBoxReadA2BCmd(MAILBOX_ID_0, MAILBOX_CHANNEL_0);
                //data = MailBoxReadA2BData(MAILBOX_ID_0, MAILBOX_CHANNEL_0);
                MailBoxClearA2BInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_0));

                rkos_semaphore_give_fromisr(pstMailBoxDev->osMailBoxWriteSem);
                //rk_printf("A to B channle 0: cmd = %d, data = %d\r\n",cmd, data);
            }
            break;
        case MAILBOX_CHANNEL_1:
            if (IntEvent & MAILBOX_INT_1)
            {
                //cmd = MailBoxReadA2BCmd(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                //data = MailBoxReadA2BData(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxClearA2BInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_1));

                rkos_semaphore_give_fromisr(pstMailBoxDev->osMailBoxWriteSem);
                //rk_printf("B to A channle 0: cmd = %d, data = %d\r\n",cmd, data);
            }
            break;

        case MAILBOX_CHANNEL_2:
            if (IntEvent & MAILBOX_INT_2)
            {
                //cmd = MailBoxReadA2BCmd(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                //data = MailBoxReadA2BData(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxClearA2BInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_2));

                rkos_semaphore_give_fromisr(pstMailBoxDev->osMailBoxWriteSem);
                //rk_printf("A to B channle 0: cmd = %d, data = %d\r\n",cmd, data);
            }
            break;

        case MAILBOX_CHANNEL_3:
            if (IntEvent & MAILBOX_INT_3)
            {
                //cmd = MailBoxReadA2BCmd(MAILBOX_ID_0, MAILBOX_CHANNEL_3);
                //data = MailBoxReadA2BData(MAILBOX_ID_0, MAILBOX_CHANNEL_3);
                MailBoxClearA2BInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_3));

                rkos_semaphore_give_fromisr(pstMailBoxDev->osMailBoxWriteSem);
                //rk_printf("A to B channle 0: cmd = %d, data = %d\r\n",cmd, data);
            }
            break;

        default:
            break;
    }
    ScuSoftResetCtr(MAILBOX_SRST, TRUE);
    DelayMs(10);
    ScuSoftResetCtr(MAILBOX_SRST, FALSE);
}

/*******************************************************************************
** Name: MailBoxDevIntIsr
** Input:uint32 DevID
** Return: void
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON FUN void MailBoxDevB2AIsr(uint32 DevID)
{
    uint32 cmd;
    uint32 data;
    uint32 TypeA2BB2A;
    int intstatus;
    uint32 IntEvent;
    MAILBOX_DEVICE_CLASS * pstMailBoxDev =  gpstMailBoxDevISR[DevID];
    IntEvent = MailBoxGetIntType(pstMailBoxDev->MailBoxID, MAILBOX_A2B_TYPE);

    switch (pstMailBoxDev->MailBoxID)
    {
        case MAILBOX_CHANNEL_0:
            if (IntEvent & MAILBOX_INT_0)
            {
                //cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_0);
                //data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_0);
                MailBoxClearB2AInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_0));

                rkos_semaphore_give_fromisr(pstMailBoxDev->osMailBoxWriteSem);
                //rk_printf("B to A channle 0: cmd = %d, data = %d\r\n",cmd, data);
            }
            break;
        case MAILBOX_CHANNEL_1:
            if (IntEvent & MAILBOX_INT_1)
            {
                //cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                //data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxClearB2AInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_1));

                 rkos_semaphore_give_fromisr(pstMailBoxDev->osMailBoxWriteSem);
                //rk_printf("B to A channle 0: cmd = %d, data = %d\r\n",cmd, data);
            }
            break;

        case MAILBOX_CHANNEL_2:
            if (IntEvent & MAILBOX_INT_2)
            {
                //cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                //data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxClearB2AInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_2));

                rkos_semaphore_give_fromisr(pstMailBoxDev->osMailBoxWriteSem);
                //rk_printf("B to A channle 0: cmd = %d, data = %d\r\n",cmd, data);
            }
            break;

        case MAILBOX_CHANNEL_3:
            if (IntEvent & MAILBOX_INT_3)
            {
                //cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_3);
                //data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_3);
                MailBoxClearB2AInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_3));

                rkos_semaphore_give_fromisr(pstMailBoxDev->osMailBoxWriteSem);
                //rk_printf("B to A channle 0: cmd = %d, data = %d\r\n",cmd, data);
            }
            break;

        default:
            break;
    }
    ScuSoftResetCtr(MAILBOX_SRST, TRUE);
    DelayMs(10);
    ScuSoftResetCtr(MAILBOX_SRST, FALSE);
}

/*******************************************************************************
** Name: MailBoxDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON FUN rk_err_t MailBoxDevResume(HDC dev)
{
    //MailBoxDev Resume...
    MAILBOX_DEVICE_CLASS * pstMailBoxDev =  (MAILBOX_DEVICE_CLASS *)dev;
    if (pstMailBoxDev == NULL)
    {
        return RK_ERROR;
    }

}
/*******************************************************************************
** Name: MailBoxDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_
COMMON FUN rk_err_t MailBoxDevSuspend(HDC dev)
{
    //MailBoxDev Suspend...
    MAILBOX_DEVICE_CLASS * pstMailBoxDev =  (MAILBOX_DEVICE_CLASS *)dev;
    if (pstMailBoxDev == NULL)
    {
        return RK_ERROR;
    }

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MailBoxDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_INIT_
INIT API rk_err_t MailBoxDev_Delete(uint32 DevID, void * arg)
{
    //Check MailBoxDev is not exist...
    if (gpstMailBoxDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    //MailBoxDev deinit...
    MailBoxDevDeInit(gpstMailBoxDevISR[DevID]);

    //Free MailBoxDev memory...
    rkos_semaphore_delete(gpstMailBoxDevISR[DevID]->osMailBoxOperReqSem);
    rkos_semaphore_delete(gpstMailBoxDevISR[DevID]->osMailBoxOperSem);
    rkos_semaphore_delete(gpstMailBoxDevISR[DevID]->osMailBoxReadReqSem);
    rkos_semaphore_delete(gpstMailBoxDevISR[DevID]->osMailBoxReadSem);
    rkos_semaphore_delete(gpstMailBoxDevISR[DevID]->osMailBoxWriteReqSem);
    rkos_semaphore_delete(gpstMailBoxDevISR[DevID]->osMailBoxWriteSem);
    rkos_memory_free(gpstMailBoxDevISR[DevID]);

    //Delete MailBoxDev...
    gpstMailBoxDevISR[DevID] = NULL;

//    //Delete MailBoxDev Read and Write Module...
//#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
//    FW_RemoveSegment(SEGMENT_ID_MAILBOX_DEV);
//#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: MailBoxDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_INIT_
INIT API HDC MailBoxDev_Create(uint32 DevID, void * arg)
{
    MAILBOX_DEV_ARG * pstMailBoxDevArg;
    DEVICE_CLASS* pstDev;
    MAILBOX_DEVICE_CLASS * pstMailBoxDev;
    printf("MailBoxDev_Create create \n");
    if (arg == NULL)
    {
        printf("arg == NULL \n");
        return (HDC)RK_PARA_ERR;
    }

    // Create handler...
    pstMailBoxDevArg = (MAILBOX_DEV_ARG *)arg;
    pstMailBoxDev =  rkos_memory_malloc(sizeof(MAILBOX_DEVICE_CLASS));
    memset(pstMailBoxDev, 0, sizeof(MAILBOX_DEVICE_CLASS));
    if (pstMailBoxDev == NULL)
    {
        return NULL;
    }

    //init handler...
    pstMailBoxDev->osMailBoxOperReqSem  = rkos_semaphore_create(1,1);
    pstMailBoxDev->osMailBoxOperSem  = rkos_semaphore_create(1,0);

    pstMailBoxDev->osMailBoxReadReqSem  = rkos_semaphore_create(1,1);
    pstMailBoxDev->osMailBoxReadSem  = rkos_semaphore_create(1,1);

    pstMailBoxDev->osMailBoxWriteReqSem  = rkos_semaphore_create(1,1);
    pstMailBoxDev->osMailBoxWriteSem  = rkos_semaphore_create(1,1);
    if (((pstMailBoxDev->osMailBoxOperReqSem) == 0)
            || ((pstMailBoxDev->osMailBoxOperSem) == 0)
            || ((pstMailBoxDev->osMailBoxReadReqSem) == 0)
            || ((pstMailBoxDev->osMailBoxReadSem) == 0)
            || ((pstMailBoxDev->osMailBoxWriteReqSem) == 0)
            || ((pstMailBoxDev->osMailBoxWriteSem) == 0))
    {
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxOperReqSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxOperSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxReadReqSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxReadSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxWriteReqSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxWriteSem);
        rkos_memory_free(pstMailBoxDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstMailBoxDev;
    pstDev->suspend = MailBoxDevSuspend;
    pstDev->resume  = MailBoxDevResume;

    //init arg...
    pstMailBoxDev->TypeA2BB2A = pstMailBoxDevArg->TypeA2BB2A;
    MailBoxDevHwInit(DevID,  pstMailBoxDev->TypeA2BB2A);

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_MAILBOX_DEV, SEGMENT_OVERLAY_CODE);
#endif
    //device init...
    gpstMailBoxDevISR[DevID] = NULL;
    if (MailBoxDevInit(pstMailBoxDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxOperReqSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxOperSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxReadReqSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxReadSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxWriteReqSem);
        rkos_semaphore_delete(pstMailBoxDev->osMailBoxWriteSem);
        rkos_memory_free(pstMailBoxDev);
        return (HDC) RK_ERROR;
    }
    gpstMailBoxDevISR[DevID] = pstMailBoxDev;
    //MailBoxDevInit(pstMailBoxDev);

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
** Name: MailBoxDevDeInit
** Input:MAILBOX_DEVICE_CLASS * pstMailBoxDev
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_INIT_
INIT FUN rk_err_t MailBoxDevDeInit(MAILBOX_DEVICE_CLASS * pstMailBoxDev)
{
    //If ip device first step unregister int
    //IntDisable(INT_ID17_UART);
    //IntPendingClear(INT_ID17_UART);
    //IntUnregister(INT_ID17_UART);

    //If ip device second step close clk
    //ScuClockGateCtr(CLOCK_GATE_UART,0);



    IntDisable(INT_ID_MAILBOX0 + pstMailBoxDev->MailBoxID);
    IntPendingClear(INT_ID_MAILBOX0 + pstMailBoxDev->MailBoxID);
    IntUnregister(INT_ID_MAILBOX0 + pstMailBoxDev->MailBoxID);
    ScuClockGateCtr(PCLK_MAILBOX_GATE,0);
    if (MAILBOX_A2B_TYPE == pstMailBoxDev->TypeA2BB2A)
        MailBoxDisableA2BInt(MAILBOX_ID_0, (int32)(1 << pstMailBoxDev->MailBoxID));
    else if (MAILBOX_B2A_TYPE == pstMailBoxDev->TypeA2BB2A)
        MailBoxDisableB2AInt(MAILBOX_ID_0, (int32)(1 << pstMailBoxDev->MailBoxID));
    else
    {
        return RK_ERROR;
        rk_printf("IntType Error\n");
    }


}
/*******************************************************************************
** Name: MailBoxDevInit
** Input:MAILBOX_DEVICE_CLASS * pstMailBoxDev
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:09
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_INIT_
INIT FUN rk_err_t MailBoxDevInit(MAILBOX_DEVICE_CLASS * pstMailBoxDev)
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
    if (pstMailBoxDev->MailBoxID > MAILBOX_DEV_NUM)
    {

        return RK_ERROR;
    }
    else
    {
        return RK_SUCCESS;
    }
    return RK_ERROR;

}


#ifdef _MAILBOX_DEV_SHELL_
_DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_
static SHELL_CMD ShellMailBoxName[] =
{
    "pcb",MailBoxDevShellPcb,"NULL","NULL",
    "create",MailBoxDevShellMc,"NULL","NULL",
    "del",MailBoxDevShellDel,"NULL","NULL",
    "test",MailBoxDevShellTest,"NULL","NULL",
    "wb2a",MailBoxDevShellWrite,"NULL","NULL",
    "ra2b",MailBoxDevShellRead,"NULL","NULL",
    "dmawa2b",MailBoxDevShellDMA_Write,"NULL","NULL",
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
** Name: MailBoxDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:10
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_
SHELL API rk_err_t MailBoxDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellMailBoxName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }



    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellMailBoxName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellMailBoxName[i].CmdDes, pItem);
    if(ShellMailBoxName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellMailBoxName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: MailBoxDevShellWrite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 10:11:19
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_
SHELL FUN rk_err_t MailBoxDevShellWrite(HDC dev, uint8 * pstr)
{
    uint32 DevID,ret;
    MAILBOX_DEV_ARG stMailBoxArg;
    MAILBOX_DEVICE_CLASS *devMailBox;
    HDC hMailBoxDev;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    //Get MailBoxDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        DevID = 2;
    }
    else if (StrCmpA(pstr, "3", 1) == 0)
    {
        DevID = 3;
    }
    else
    {
        DevID = 0;
    }
    rk_printf("channle %d test start\n",DevID);

    //Set MailBoxArg...
    stMailBoxArg.TypeA2BB2A = MAILBOX_B2A_TYPE;

    //Creat MailBoxDev...
    ret = RKDev_Create(DEV_CLASS_MAILBOX, DevID,&stMailBoxArg);
    if (ret != RK_SUCCESS)
    {
        rk_printf("MAILBOX create failure ret=%d\n",ret);
    }

    //Open MailBoxDev...
    hMailBoxDev = RKDev_Open(DEV_CLASS_MAILBOX, DevID, NOT_CARE);//DevID 0

    if ((hMailBoxDev == NULL) || (hMailBoxDev == (HDC)RK_ERROR) || (hMailBoxDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("MailBoxDev open failure");
        return RK_SUCCESS;
    }
    devMailBox = (MAILBOX_DEVICE_CLASS *)hMailBoxDev;
    //Set MailBoxDevArg...
    devMailBox->Cmd = 11;
    devMailBox->Data = 22;
    devMailBox->TypeA2BB2A = stMailBoxArg.TypeA2BB2A;
    devMailBox->MailBoxID = DevID;

    MailBoxDev_Write(devMailBox);
    DelayMs(1000);

    //close MailBoxDev...
    RKDev_Close(hMailBoxDev);
}

/*******************************************************************************
** Name: MailBoxDevShellRead
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 10:11:19
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_
SHELL FUN rk_err_t MailBoxDevShellRead(HDC dev, uint8 * pstr)
{
    uint32 DevID,ret;
    MAILBOX_DEV_ARG stMailBoxArg;
    MAILBOX_DEVICE_CLASS *devMailBox;
    HDC hMailBoxDev;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }


    //Get MailBoxDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        DevID = 2;
    }
    else if (StrCmpA(pstr, "3", 1) == 0)
    {
        DevID = 3;
    }
    else
    {
        DevID = 0;
    }
    rk_printf("channle %d test start\n",DevID);

    //Set MailBoxArg...
    stMailBoxArg.TypeA2BB2A = MAILBOX_A2B_TYPE;

    //Creat MailBoxDev...
    ret = RKDev_Create(DEV_CLASS_MAILBOX, DevID,&stMailBoxArg);
    if (ret != RK_SUCCESS)
    {
        rk_printf("I2cDev create failure ret=%d\n",ret);
    }

    //Open MailBoxDev...
    hMailBoxDev = RKDev_Open(DEV_CLASS_MAILBOX, DevID, NOT_CARE);//DevID 0
    if ((hMailBoxDev == NULL) || (hMailBoxDev == (HDC)RK_ERROR) || (hMailBoxDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("MailBoxDev open failure");
        return RK_SUCCESS;
    }
    devMailBox = (MAILBOX_DEVICE_CLASS *)hMailBoxDev;
    //Set MailBoxDevArg...
    devMailBox->TypeA2BB2A = stMailBoxArg.TypeA2BB2A;
    devMailBox->MailBoxID = DevID;

    MailBoxDev_Read(devMailBox);
    DelayMs(10);

    //close MailBoxDev...
    RKDev_Close(hMailBoxDev);
}

/*******************************************************************************
** Name: MailBoxDevShellDMA_Write
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 10:11:19
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_
SHELL FUN rk_err_t MailBoxDevShellDMA_Write(HDC dev, uint8 * pstr)
{
    uint32 ret ,i, j;
    MAILBOX_DEV_ARG stMailBoxArg;
    MAILBOX_DEVICE_CLASS *devMailBox;
    HDC hMailBoxDev;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }


    //Set MailBoxArg...
    stMailBoxArg.TypeA2BB2A = MAILBOX_A2B_TYPE;

    //Creat MailBoxDev...
    ret = RKDev_Create(DEV_CLASS_MAILBOX, MAILBOX_ID_3,&stMailBoxArg);
    if (ret != RK_SUCCESS)
    {
        rk_printf("MailBox create failure ret=%d\n",ret);
    }

    //Open MailBoxDev...
    hMailBoxDev = RKDev_Open(DEV_CLASS_MAILBOX, MAILBOX_ID_3, NOT_CARE);//DevID 0
    if ((hMailBoxDev == NULL) || (hMailBoxDev == (HDC)RK_ERROR) || (hMailBoxDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("MailBoxDev open failure");
        return RK_SUCCESS;
    }

    devMailBox = (MAILBOX_DEVICE_CLASS *)hMailBoxDev;
    //Set MailBoxDevArg...
    devMailBox->Cmd = 7;
    devMailBox->Data = 9;
    devMailBox->TypeA2BB2A = stMailBoxArg.TypeA2BB2A;
    devMailBox->MailBoxID = MAILBOX_ID_3;

            //creat mda
            ret = RKDev_Create(DEV_CLASS_DMA, 0, NULL);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("dma device create failure");
    }


    //DmaDevHwInit(dev, );
    DMA_CFGX M2MDmaCfg  = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD,0};
    DelayMs(50);
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA, (void*)MailBoxDev_M2M_DmaIsr);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    for (j=0; j< 6; j++)
    {
        rk_printf("A core Dma write test ch = %d\n",j);
        DmaEnableInt(j);
        memDmafinish = 0;
        memset(memDmabuf, 0x55, 1024);
        memset(BcoreDmabuf, 0, 1024);
        //DmaConfig(i, (uint32)BcoreDmabuf, (uint32)memDmabuf,256, &M2MDmaCfg, MailBoxDev_M2M_DmaIsr);
        DmaDev_DmaStart(hMailBoxDev, j, (uint32)memDmabuf, (uint32)BcoreDmabuf, 56, &M2MDmaCfg, MailBoxDev_M2M_DmaIsr);
        while (!memDmafinish)
        {

        }
        for (i=0; i< 1024; i++)
        {
            if (BcoreDmabuf[i] != 0x55)
            {
                rk_printf("A core Dma write test ch =%d fail\n" , j);
                return ret;
            }
        }
        if (i == 1024)
        {
            rk_printf("A core Dma write test ch =%d OK\n" , j);
        }
    }
    MailBoxDev_Write(devMailBox);
    DelayMs(10);

    //close MailBoxDev...
    RKDev_Close(devMailBox);
}


/*******************************************************************************
** Name: MailBoxDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:10
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_
SHELL FUN rk_err_t MailBoxDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hMailBoxDev;
    uint32 DevID;
    uint32 ret;
    MAILBOX_DEV_ARG stMailBoxArg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    //Get MailBoxDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        DevID = 2;
    }
    else if (StrCmpA(pstr, "3", 1) == 0)
    {
        DevID = 3;
    }
    else
    {
        DevID = 0;
    }
    //Set MailBoxArg...
    stMailBoxArg.TypeA2BB2A = MAILBOX_A2B_TYPE;

    //Creat MailBoxDev...
    ret = RKDev_Create(DEV_CLASS_MAILBOX, DevID,&stMailBoxArg);
    if (ret != RK_SUCCESS)
    {
        rk_printf("MAILBOX create failure ret=%d\n",ret);
    }

    //Open MailBoxDev...
    hMailBoxDev = RKDev_Open(DEV_CLASS_MAILBOX, DevID, NOT_CARE);//DevID 0
    if ((hMailBoxDev == NULL) || (hMailBoxDev == (HDC)RK_ERROR) || (hMailBoxDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("MailBoxDev open failure");
        return RK_SUCCESS;
    }
    rk_print_string("MailBoxDev open");
    //do test....

    //close MailBoxDev...
    RKDev_Close(hMailBoxDev);
    rk_print_string("MailBoxDev close");
    rk_print_string("MailBoxDev test over");
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: MailBoxDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:10
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_
SHELL FUN rk_err_t MailBoxDevShellDel(HDC dev, uint8 * pstr)
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

    //Get MailBoxDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        DevID = 2;
    }
    else if (StrCmpA(pstr, "3", 1) == 0)
    {
        DevID = 3;
    }
    else
    {
        DevID = 0;
    }

    if (RKDev_Delete(DEV_CLASS_MAILBOX, DevID) != RK_SUCCESS)
    {
        rk_print_string("MAILBOXDev delete failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: MailBoxDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:10
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_
SHELL FUN rk_err_t MailBoxDevShellMc(HDC dev, uint8 * pstr)
{
    MAILBOX_DEV_ARG stMailBoxDevArg;
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


    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        DevID = 2;
    }
    else if (StrCmpA(pstr, "3", 1) == 0)
    {
        DevID = 3;
    }
    else
    {
        DevID = 0;
    }

    //Init MailBoxDev arg...
    //stMailBoxDevArg.usbmode = USBOTG_MODE_DEVICE;

    //Create MailBoxDev...
    rk_print_string("MailBoxDev create \n");
    ret = RKDev_Create(DEV_CLASS_MAILBOX, DevID, &stMailBoxDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("MailBoxDev create failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: MailBoxDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.5.21
** Time: 9:15:10
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_
SHELL FUN rk_err_t MailBoxDevShellPcb(HDC dev, uint8 * pstr)
{
    HDC hMailBoxDev;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    //Get MailBoxDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        DevID = 2;
    }
    else if (StrCmpA(pstr, "3", 1) == 0)
    {
        DevID = 3;
    }
    else
    {
        DevID = 0;
    }

    //Display pcb...
    if (gpstMailBoxDevISR[DevID] != NULL)
    {
        rk_print_string("rn");
        rk_printf_no_time("MailBoxDev ID = %d Pcb list as follow:", DevID);
        rk_printf_no_time("      &stMailBoxDevice---0x%08x", &gpstMailBoxDevISR[DevID]->stMailBoxDevice);
        rk_printf_no_time("      osMailBoxOperReqSem---0x%08x", gpstMailBoxDevISR[DevID]->osMailBoxOperReqSem);
        rk_printf_no_time("      osMailBoxOperSem---0x%08x", gpstMailBoxDevISR[DevID]->osMailBoxOperSem);
        rk_printf_no_time("      osMailBoxReadReqSem---0x%08x", gpstMailBoxDevISR[DevID]->osMailBoxReadReqSem);
        rk_printf_no_time("      osMailBoxReadSem---0x%08x", gpstMailBoxDevISR[DevID]->osMailBoxReadSem);
        rk_printf_no_time("      osMailBoxWriteReqSem---0x%08x", gpstMailBoxDevISR[DevID]->osMailBoxWriteReqSem);
        rk_printf_no_time("      osMailBoxWriteSem---0x%08x", gpstMailBoxDevISR[DevID]->osMailBoxWriteSem);
        //rk_printf_no_time("      usbmode---0x%08x", gpstUsbOtgDevISR[DevID]->usbmode);
        //rk_printf_no_time("      usbspeed---0x%08x", gpstUsbOtgDevISR[DevID]->usbspeed);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("MailBoxDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;

}

#endif

#endif
