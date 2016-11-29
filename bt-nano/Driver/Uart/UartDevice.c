/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\Uart\UartDevice.c
* Owner: Aaron.sun
* Date: 2014.2.14
* Time: 13:51:36
* Desc: Uart Device Class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.2.14     13:51:36   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_UART_UARTDEVICE_C__

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
typedef  struct _TX_ITEM
{
    uint8* buffer;
    uint32 size;
    uint32 mode;
    pUartTx_complete UartTx_complete;

}TX_ITEM;


typedef  struct _RX_ITEM
{
    uint8* buffer;
    uint32 size;

}RX_ITEM;


typedef  struct _UART_DEVICE_CLASS
{
    DEVICE_CLASS    stDev;            //device father class
    uint32 dwBitRate;
    uint32 dwBitWidth;
    uint32 stopbit;
    uint32 parity;
    pSemaphore osUartReadReqSem;
    pSemaphore osUartReadSem;
    pSemaphore osUartWriteReqSem;
    pSemaphore osUartWriteSem;
    pSemaphore osUartControlReqSem;

    uint32           dwTxNeedTransLen;
    uint32           dwRxNeedTransLen;
    uint32           dwTxStatus;
    uint32           dwRxStatus;
    TX_ITEM          stTxItem;
    RX_ITEM          stRxItem;
    uint32 Channel;

}UART_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static UART_DEVICE_CLASS * gpstUartDevISR[UART_DEV_MAX];


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
rk_err_t UartDevCheckHandler(HDC dev);
rk_err_t UartDevShellFlowControlTest(HDC dev,uint8 * pstr);
void UartDevShellBspIsrHandle5(void);
void UartDevShellBspIsrHandle4(void);
void UartDevShellBspIsrHandle3(void);
void UartDevShellBspIsrHandle2(void);
void UartDevShellBspIsrHandle1(void);
void UartDevShellBspIsrHandle0(void);
rk_err_t UartDevShellTeamworkTest(HDC dev, uint8 * pstr);
void UartDevShellDmaIsrHandle(void);
rk_err_t UartDevShellBspMemToUart(HDC dev, uint8 * pstr);
rk_err_t UartDevShellBspUartToMem(HDC dev, uint8 * pstr);
void UartDevShellBspIsrHandle(eUART_CH uartID);
rk_err_t UartDevShellBspSR( HDC dev, uint8 * pstr);
rk_err_t UartDevShellBspRecive(HDC dev, uint8 * pstr);
rk_err_t UartDevShellBspSend(HDC dev, uint8 * pstr);
rk_err_t UartDevShellBspSetBaudRate(HDC dev, uint8 * pstr);
rk_err_t UartDevShellBspSetParity(HDC dev, uint8 * pstr);
rk_err_t UartDevShellBspSetStopB(HDC dev, uint8 * pstr);
rk_err_t UartDevShellBspSetbitW( HDC dev, uint8 * pstr);
rk_err_t UartDevShellHelp(HDC dev, uint8 * pstr);
rk_err_t UartDevShellBspHelp(HDC dev, uint8 * pstr);
rk_err_t UartDevShellBsp(HDC dev, uint8 * pstr);
rk_err_t UartDevResume(HDC dev);
rk_err_t UartDevSuspend(HDC dev, uint32 Level);
rk_err_t UartDevDeInit(UART_DEVICE_CLASS * pstUartDev);
rk_err_t UartDevInit(UART_DEVICE_CLASS * pstUartDev);
void UartIntIsr(uint32 UartDevID);
rk_err_t UartDevShellPcb(HDC dev, uint8 * pstr);
rk_err_t UartDevShellMc(HDC dev, uint8 * pstr);
rk_err_t UartDevShellDel(HDC dev, uint8 * pstr);
rk_err_t UartDevShellTest(HDC dev, uint8 * pstr);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UartDev_SetFlowControl
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.2.24
** Time: 17:58:36
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API rk_err_t UartDev_SetFlowControl(HDC dev)
{
    UART_DEVICE_CLASS * pstUartDev =  (UART_DEVICE_CLASS *)dev ;
    if (pstUartDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstUartDev->osUartReadReqSem, MAX_DELAY);
    rkos_semaphore_take(pstUartDev->osUartWriteReqSem, MAX_DELAY);
    UARTEnableAutoFlowControl(pstUartDev->stDev.DevID);
    rkos_semaphore_give(pstUartDev->osUartWriteReqSem);
    rkos_semaphore_give(pstUartDev->osUartReadReqSem);
}


/*******************************************************************************
** Name: UartDev_SetBaudRate
** Input:HDC dev, uint32 BaudRate
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.2.24
** Time: 17:57:40
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API rk_err_t UartDev_SetBaudRate(HDC dev, uint32 BaudRate)
{
    UART_DEVICE_CLASS * pstUartDev =  (UART_DEVICE_CLASS *)dev ;
    if (pstUartDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstUartDev->osUartReadReqSem, MAX_DELAY);
    rkos_semaphore_take(pstUartDev->osUartWriteReqSem, MAX_DELAY);
    UARTSetBaudRate(pstUartDev->stDev.DevID, 24*1000*1000, BaudRate);
    rkos_semaphore_give(pstUartDev->osUartWriteReqSem);
    rkos_semaphore_give(pstUartDev->osUartReadReqSem);
}

/*******************************************************************************
** Name: UartIntIsr5
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.26
** Time: 17:13:46
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API void UartIntIsr5(void)
{
    UartIntIsr(UART_DEV5);
}

/*******************************************************************************
** Name: UartIntIsr4
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.26
** Time: 17:13:13
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API void UartIntIsr4(void)
{
    UartIntIsr(UART_DEV4);
}

/*******************************************************************************
** Name: UartIntIsr3
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.26
** Time: 17:12:39
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API void UartIntIsr3(void)
{
    UartIntIsr(UART_DEV3);
}

/*******************************************************************************
** Name: UartIntIsr2
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.26
** Time: 17:12:07
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API void UartIntIsr2(void)
{
    UartIntIsr(UART_DEV2);
}

/*******************************************************************************
** Name: UartIntIsr1
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.26
** Time: 17:10:59
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API void UartIntIsr1(void)
{
    UartIntIsr(UART_DEV1);
}

/*******************************************************************************
** Name: UartIntIsr0
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.4.11
** Time: 15:55:17
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API void UartIntIsr0(void)
{
    UartIntIsr(UART_DEV0);
}


/*******************************************************************************
** Name: UartDevRead
** Input:DEVICE_CLASS* dev,uint32 pos, void* buffer, uint32 size,uint8 mode,pRx_indicate Rx_indicate
** Return: rk_size_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:48:03
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API rk_size_t UartDev_Read(HDC dev, void* buffer, uint32 size ,uint32 timeout)
{
    rk_size_t realsize;

    UART_DEVICE_CLASS * pstUartDev =  (UART_DEVICE_CLASS *)dev ;
    if (pstUartDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstUartDev->osUartReadReqSem, MAX_DELAY);

    //rkos_enter_critical();

    pstUartDev->dwRxStatus = DEVICE_STATUS_SYNC_PIO_READ;

    pstUartDev->stRxItem.buffer = (uint8*) buffer;
    pstUartDev->stRxItem.size   = size;
    pstUartDev->dwRxNeedTransLen = size;

    //rkos_exit_critical();

    realsize = UARTReadByte((eUART_CH)pstUartDev->stDev.DevID,pstUartDev->stRxItem.buffer, pstUartDev->dwRxNeedTransLen);


    if (realsize == pstUartDev->dwRxNeedTransLen)
    {
        pstUartDev->dwRxStatus = DEVICE_STATUS_IDLE;
        rkos_semaphore_give(pstUartDev->osUartReadReqSem);
        return size;
    }
    else
    {
        pstUartDev->stRxItem.buffer += realsize;
        pstUartDev->dwRxNeedTransLen -= realsize;
    }

    UARTSetIntEnabled((eUART_CH)pstUartDev->stDev.DevID,UART_IE_RX);
    if(rkos_semaphore_take(pstUartDev->osUartReadSem, timeout)== RK_ERROR)
    {
        UARTSetIntDisabled((eUART_CH)(eUART_CH)pstUartDev->stDev.DevID,UART_IE_RX);
        gpstUartDevISR[(eUART_CH)pstUartDev->stDev.DevID]->dwRxStatus = DEVICE_STATUS_IDLE;
        rkos_semaphore_give(pstUartDev->osUartReadReqSem);
        return size- pstUartDev->dwRxNeedTransLen;
    }
    //UARTSetIntDisabled(UART_IE_RX);

    rkos_semaphore_give(pstUartDev->osUartReadReqSem);

    return size;
}


/*******************************************************************************
** Name: UartIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
void UartIntIsr(uint32 UartDevID)
{
    uint32 uartIntType, realsize;

    if(UartDevID >= UART_DEV_MAX)
        return;

    uartIntType = UARTGetIntType((eUART_CH)UartDevID);

    if (gpstUartDevISR[UartDevID] != NULL)
    {
        if (uartIntType & UART_INT_READ_FIFO_NOT_EMPTY)
        {
            if (gpstUartDevISR[UartDevID]->dwRxStatus == DEVICE_STATUS_SYNC_PIO_READ)
            {
                if (gpstUartDevISR[UartDevID]->dwRxNeedTransLen)
                {
                    realsize = UARTReadByte((eUART_CH)UartDevID,gpstUartDevISR[UartDevID]->stRxItem.buffer, gpstUartDevISR[UartDevID]->dwRxNeedTransLen);

                    gpstUartDevISR[UartDevID]->stRxItem.buffer += realsize;
                    gpstUartDevISR[UartDevID]->dwRxNeedTransLen -= realsize;

                    if (gpstUartDevISR[UartDevID]->dwRxNeedTransLen == 0)
                    {
                        if (gpstUartDevISR[UartDevID]->dwRxStatus == DEVICE_STATUS_SYNC_PIO_READ)
                        {
                            rkos_semaphore_give_fromisr(gpstUartDevISR[UartDevID]->osUartReadSem);
                        }
                        gpstUartDevISR[UartDevID]->dwRxStatus = DEVICE_STATUS_IDLE;
                        UARTSetIntDisabled((eUART_CH)UartDevID,UART_IE_RX);
                    }
                }
            }
        }

        if (uartIntType & UART_INT_WRITE_FIFO_EMPTY)
        {
            if ((gpstUartDevISR[UartDevID]->dwTxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                    || (gpstUartDevISR[UartDevID]->dwTxStatus == DEVICE_STATUS_SYNC_PIO_WRITE))
            {
                if (gpstUartDevISR[UartDevID]->dwTxNeedTransLen)
                {
                    realsize = UARTWriteByte((eUART_CH)UartDevID,gpstUartDevISR[UartDevID]->stTxItem.buffer, gpstUartDevISR[UartDevID]->dwTxNeedTransLen);

                    gpstUartDevISR[UartDevID]->stTxItem.buffer += realsize;
                    gpstUartDevISR[UartDevID]->dwTxNeedTransLen -= realsize;

                    if (gpstUartDevISR[UartDevID]->dwTxNeedTransLen == 0)
                    {
                        if (gpstUartDevISR[UartDevID]->dwTxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                        {
                            if (gpstUartDevISR[UartDevID]->stTxItem.UartTx_complete != NULL)
                            {
                                gpstUartDevISR[UartDevID]->stTxItem.UartTx_complete(gpstUartDevISR[UartDevID]->stTxItem.buffer);
                            }
                            UARTSetIntDisabled((eUART_CH)UartDevID,UART_IE_TX);
                            rkos_semaphore_give_fromisr(gpstUartDevISR[UartDevID]->osUartWriteReqSem);
                        }

                        else if (gpstUartDevISR[UartDevID]->dwTxStatus == DEVICE_STATUS_SYNC_PIO_WRITE)
                        {
                            rkos_semaphore_give_fromisr(gpstUartDevISR[UartDevID]->osUartWriteSem);
                        }

                        gpstUartDevISR[UartDevID]->dwTxStatus = DEVICE_STATUS_IDLE;
                    }
                }
            }
        }
    }
}


/*******************************************************************************
** Name: UartDevWrite
** Input:DEVICE_CLASS* dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_size_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:46:51
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON API rk_size_t UartDev_Write(HDC dev,const void* buffer,
                    uint32 size,uint8 mode,pUartTx_complete Tx_complete)
{
    rk_size_t realsize;

    UART_DEVICE_CLASS * pstUartDev = (UART_DEVICE_CLASS *)dev;
    if (pstUartDev == NULL)
    {
        return RK_ERROR;
    }

    if (mode == SYNC_MODE)
    {
        rkos_semaphore_take(pstUartDev->osUartWriteReqSem, MAX_DELAY);

        //rkos_enter_critical();

        pstUartDev->dwTxStatus = DEVICE_STATUS_SYNC_PIO_WRITE;

        pstUartDev->stTxItem.buffer = (uint8*) buffer;
        pstUartDev->stTxItem.size   = size;
        pstUartDev->dwTxNeedTransLen = size;

        //rkos_exit_critical();

        if(pstUartDev->stDev.DevID == BT_HCI_UART_ID)
        {
            //printf("\nbt want uart write..... = %d", size);
            //DelayMs(1000);
        }


        realsize = UARTWriteByte((eUART_CH)pstUartDev->stDev.DevID, \
                                pstUartDev->stTxItem.buffer,
                                pstUartDev->dwTxNeedTransLen);

        if (realsize == pstUartDev->dwTxNeedTransLen)
        {
            pstUartDev->dwTxStatus = DEVICE_STATUS_IDLE;
            rkos_semaphore_give(pstUartDev->osUartWriteReqSem);
            return size;
        }
        else
        {
            pstUartDev->stTxItem.buffer += realsize;
            pstUartDev->dwTxNeedTransLen -= realsize;
        }

        UARTSetIntEnabled((eUART_CH)pstUartDev->stDev.DevID,UART_IE_TX);
        rkos_semaphore_take(pstUartDev->osUartWriteSem, MAX_DELAY);
        UARTSetIntDisabled((eUART_CH)pstUartDev->stDev.DevID,UART_IE_TX);

        rkos_semaphore_give(pstUartDev->osUartWriteReqSem);

        return size;
    }
    else if (mode == ASYNC_MODE)
    {
        rkos_semaphore_take(pstUartDev->osUartWriteReqSem, MAX_DELAY);
        //rkos_enter_critical();

        pstUartDev->dwTxStatus = DEVICE_STATUS_ASYNC_PIO_WRITE;
        pstUartDev->stTxItem.buffer = (uint8*) buffer;

        pstUartDev->stTxItem.size   = size;
        pstUartDev->stTxItem.UartTx_complete = Tx_complete;
        pstUartDev->dwTxNeedTransLen = size;

        //rkos_exit_critical();

        realsize = UARTWriteByte((eUART_CH)pstUartDev->stDev.DevID, \
                                pstUartDev->stTxItem.buffer,
                                pstUartDev->dwTxNeedTransLen);

        if (realsize == pstUartDev->dwTxNeedTransLen)
        {
            pstUartDev->dwTxStatus = DEVICE_STATUS_IDLE;
            rkos_semaphore_give(pstUartDev->osUartWriteReqSem);
            return size;
        }
        else
        {
            pstUartDev->stTxItem.buffer += realsize;
            pstUartDev->dwTxNeedTransLen -= realsize;
        }
        UARTSetIntEnabled((eUART_CH)pstUartDev->stDev.DevID,UART_IE_TX);
        return realsize;

    }
    return NULL;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UartDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.17
** Time: 15:53:14
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_COMMON_
COMMON FUN rk_err_t UartDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < UART_DEV_MAX; i++)
    {
        if(gpstUartDevISR[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UartDevCreate
** Input:void
** Return: DEVICE_CLASS *
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:40:31
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
INIT API HDC UartDev_Create(uint32 DevID, void *arg)
{
    DEVICE_CLASS * pstDev;
    UART_DEVICE_CLASS * pstUartDev;
    UART_DEV_ARG * pstUartArg = (UART_DEV_ARG *)arg;

    if (arg == NULL)
    {
        return (HDC)(RK_PARA_ERR);
    }

    pstUartDev =  rkos_memory_malloc(sizeof(UART_DEVICE_CLASS));
    if (pstUartDev == NULL)
    {
        return pstUartDev;
    }

    pstUartDev->osUartReadReqSem        = rkos_semaphore_create(1,1);
    pstUartDev->osUartWriteReqSem       = rkos_semaphore_create(1,1);
    pstUartDev->osUartControlReqSem     = rkos_semaphore_create(1,1);
    pstUartDev->osUartReadSem           = rkos_semaphore_create(1,0);
    pstUartDev->osUartWriteSem          = rkos_semaphore_create(1,0);

    if ((pstUartDev->osUartReadReqSem
            && pstUartDev->osUartWriteReqSem
            && pstUartDev->osUartControlReqSem
            && pstUartDev->osUartReadSem
            && pstUartDev->osUartWriteSem) == 0)
    {
        rkos_semaphore_delete(pstUartDev->osUartReadReqSem);
        rkos_semaphore_delete(pstUartDev->osUartWriteReqSem);
        rkos_semaphore_delete(pstUartDev->osUartControlReqSem);
        rkos_semaphore_delete(pstUartDev->osUartReadSem);
        rkos_semaphore_delete(pstUartDev->osUartWriteSem);

        rkos_memory_free(pstUartDev);
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstUartDev;

    pstDev->suspend = UartDevSuspend;
    pstDev->resume  = UartDevResume;
    pstDev->SuspendMode = ENABLE_MODE;
    pstDev->DevID   = DevID;

    pstUartDev->dwRxStatus = DEVICE_STATUS_IDLE;
    pstUartDev->dwTxStatus = DEVICE_STATUS_IDLE;
    pstUartDev->dwBitRate = pstUartArg->dwBitRate;
    pstUartDev->dwBitWidth = pstUartArg->dwBitWidth;
    pstUartDev->Channel = pstUartArg->Channel;
    pstUartDev->stopbit = pstUartArg->stopbit;
    pstUartDev->parity= pstUartArg->parity;
    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_UART_DEV, SEGMENT_OVERLAY_CODE);
    #endif


    gpstUartDevISR[DevID] = pstUartDev;
    UartDevHwDeInit(DevID,  pstUartDev->Channel);
    UartDevHwInit(DevID,  pstUartDev->Channel, 24*1000*1000);
    UartDevInit(pstUartDev);

    return pstDev;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UartDevResume
** Input:DEVICE_CLASS *
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:57:19
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDevResume(HDC dev)
{
    UART_DEVICE_CLASS * pstUartDev = (UART_DEVICE_CLASS *)dev;
    if(UartDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    UartDevHwInit(pstUartDev->stDev.DevID,  pstUartDev->Channel, 24*1000*1000);
    UartDevInit(pstUartDev);
    UARTSetIntEnabled((eUART_CH)pstUartDev->stDev.DevID,UART_IE_RX);

    pstUartDev->stDev.State = DEV_STATE_WORKING;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UartDevSuspend
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:56:05
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDevSuspend(HDC dev, uint32 Level)
{
    UART_DEVICE_CLASS * pstUartDev = (UART_DEVICE_CLASS *)dev;
    if(UartDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstUartDev->stDev.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstUartDev->stDev.State = DEV_SATE_IDLE2;
    }

    UartDevDeInit(pstUartDev);
    UartDevHwDeInit(pstUartDev->stDev.DevID, pstUartDev->Channel);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UartDev_Delete
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:41:18
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDev_Delete(uint32 DevID, void *arg)
{
    UART_DEVICE_CLASS * pstUartDev = gpstUartDevISR[DevID];

    if(pstUartDev == NULL)
    {
        return RK_SUCCESS;
    }

    UartDevDeInit(pstUartDev);
    UartDevHwDeInit(DevID, pstUartDev->Channel);

    rkos_semaphore_delete( pstUartDev->osUartReadReqSem);
    rkos_semaphore_delete( pstUartDev->osUartWriteReqSem );
    rkos_semaphore_delete( pstUartDev->osUartControlReqSem );
    rkos_semaphore_delete( pstUartDev->osUartReadSem );
    rkos_semaphore_delete( pstUartDev->osUartWriteSem );

    gpstUartDevISR[DevID] = NULL;

    rkos_memory_free(pstUartDev);


    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_UART_DEV);
    #endif

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: UartDevDeInit
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:38:30
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDevDeInit(UART_DEVICE_CLASS * pstUartDev)
{

}

/*******************************************************************************
** Name: UartDevInit
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:44:46
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDevInit(UART_DEVICE_CLASS * pstUartDev)
{

    //Default initialization state, you can change from IOCONTROL
    UARTInit(pstUartDev->stDev.DevID,
                pstUartDev->dwBitRate,
                pstUartDev->dwBitWidth,
                pstUartDev->stopbit,
                pstUartDev->parity);
    //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);
    return RK_SUCCESS;
}


#ifdef _UART_DEV_SHELL_
_DRIVER_UART_UARTDEVICE_SHELL_DATA_
static SHELL_CMD ShellUartName[] =
{
    "pcb",UartDevShellPcb,"list uart device pcb inf","uart.pcb [object id]",
    "create",UartDevShellMc,"create uart device","uart.create [object id]",
    "delete",UartDevShellDel,"delete uart device","uart.delete [object id]",
    "test",UartDevShellTest,"test uart device","uart.test [object id]",
    #ifdef SHELL_BSP
    "bsp",UartDevShellBsp,"NULL","NULL",
    #endif
    "\b",NULL,"NULL","NULL",
};

#ifdef SHELL_BSP
_DRIVER_UART_UARTDEVICE_SHELL_DATA_
static SHELL_CMD ShellUartBspName[] =
{
    "setbitw",UartDevShellBspSetbitW,"NULL","NULL",
    "setstopb",UartDevShellBspSetStopB,"NULL","NULL",
    "setparity",UartDevShellBspSetParity,"NULL","NULL",
    "setbaudrate",UartDevShellBspSetBaudRate,"NULL","NULL",
    "send",UartDevShellBspSend,"NULL","NULL",
    "recive",UartDevShellBspRecive,"NULL","NULL",
    "sr",UartDevShellBspSR,"NULL","NULL",
    "m2u",UartDevShellBspMemToUart,"NULL","NULL",
    "u2m",UartDevShellBspUartToMem,"NULL","NULL",
    "uutogether",UartDevShellTeamworkTest,"NULL","NULL",  //uart Cooperative Work
    "flowcontrol",UartDevShellFlowControlTest,"NULL","NULL",  //test uart cts / rts
    "\b",NULL,"NULL","NULL",
};

uint8        gUartChannel0;
uint8        gUartChannel;
uint32       gUartBitRate;
uint32       gUartBitWidth;
uint8        gUartint_flag;
uint8        gUartint_write_flag;
char         gUartData;

uint8       gDmaFinish;
eDMA_CHN    gdma_test_channel;

__align(4)  char dmatest_Str[30] = {'a','a','a','a','a','b','b','b','b','b', \
                               'c','c','c','c','c','d','d','d','d','d', \
                               'e','e','e','e','e','f','f','f','f','f',};


typedef struct _UART_st
{
    uint32           dwTxNeedTransLen;
    uint32           dwRxNeedTransLen;
    uint32           dwTxStatus;
    uint32           dwRxStatus;
    TX_ITEM          stTxItem;
    RX_ITEM          stRxItem;
}UART_st;

static UART_st      gUart_st[UART_MAX];

#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UsbOtgDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL API rk_err_t UartDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellUartName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellUartName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                      //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellUartName[i].CmdDes, pItem);
    if(ShellUartName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellUartName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: UartDevShellFlowControlTest
** Input:HDC dev,uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.12.30
** Time: 11:10:17
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellFlowControlTest(HDC dev,uint8 * pstr)
{
    char _data = 0;
    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;
    gUartint_flag = 0;
    gUartint_write_flag = 1;
    uint32 status;


   switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    //initialize uart 1
    UartDevGpioInit(gUartChannel);

    //Default initialization state, you can change from IOCONTROL
    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);

    UARTEnableFlowControl(gUartChannel);

    //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);
    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }


    //initialize uart 0
    gUartChannel0 = UART_CH0;
    UartDevGpioInit(gUartChannel0);

    UARTInit(gUartChannel0,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);

    UARTEnableFlowControl(gUartChannel0);

    switch( gUartChannel0 )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

   /*uart 0 as reciver ,uart 1 as transmitter.rts0 ---> cts1,*/
    while(1)
    {
        _data = 0xaa;
        UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
        UARTWriteByte(gUartChannel,(uint8*)&_data,1);

        UARTEnableAutoFlowControl(gUartChannel0);
        //get uart1 cts status,if 0 means cts_n input is de-asserted (logic 1)
        //if 1 means cts_n input is asserted (logic 0)
        status = UARTGetCTSState(gUartChannel);
        while(status == 0);
        break;
    }

    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartIntIsr0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartIntIsr1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartIntIsr2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartIntIsr3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartIntIsr4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartIntIsr5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    return RK_SUCCESS;

}


/*******************************************************************************
** Name: UartDevShellBspIsrHandle5
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.28
** Time: 17:42:54
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN void UartDevShellBspIsrHandle5(void)
{
    UartDevShellBspIsrHandle(UART_CH5);
}
/*******************************************************************************
** Name: void
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.28
** Time: 17:42:25
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN void UartDevShellBspIsrHandle4(void)
{
    UartDevShellBspIsrHandle(UART_CH4);
}
/*******************************************************************************
** Name: UartDevShellBspIsrHandle3
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.28
** Time: 17:42:11
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN void UartDevShellBspIsrHandle3(void)
{
    UartDevShellBspIsrHandle(UART_CH3);
}
/*******************************************************************************
** Name: UartDevShellBspIsrHandle2
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.28
** Time: 17:42:00
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN void UartDevShellBspIsrHandle2(void)
{
    UartDevShellBspIsrHandle(UART_CH2);
}
/*******************************************************************************
** Name: UartDevShellBspIsrHandle1
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.28
** Time: 17:41:42
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN void UartDevShellBspIsrHandle1(void)
{
    UartDevShellBspIsrHandle(UART_CH1);
}
/*******************************************************************************
** Name: UartDevShellBspIsrHandle0
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.28
** Time: 17:40:08
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN void UartDevShellBspIsrHandle0(void)
{
    UartDevShellBspIsrHandle(UART_CH0);
}


/*******************************************************************************
** Name: UartDevShellTeamworkTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.28
** Time: 17:14:52
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellTeamworkTest(HDC dev, uint8 * pstr)
{
    char _data = 0;
    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;
    gUartint_flag = 0;
    gUartint_write_flag = 1;


     switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }
    //Default initialization state, you can change from IOCONTROL
    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);

    //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);
    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    gUartChannel0 = UART_CH0;
    UARTInit(gUartChannel0,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);

    switch( gUartChannel0 )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    while(1)
    {
        UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);
        UARTReadByte(gUartChannel, (uint8*)&_data,1);

        if( _data == 'q' || _data == 'Q')
            break;

        if( gUartint_flag )
        {
            gUartint_flag = 0;

            if(!gUartint_write_flag)
            {
                gUartint_write_flag = 1;
                UARTSetIntEnabled((eUART_CH)gUartChannel0,UART_IE_TX);
                UARTWriteByte(gUartChannel0,(uint8*)&_data,1);
            }
        }
    }

        UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
        UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartIntIsr0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartIntIsr1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartIntIsr2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartIntIsr3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartIntIsr4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartIntIsr5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UartDevSHellDmaIsrHandle
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.27
** Time: 17:36:04
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN void UartDevShellDmaIsrHandle(void)
{
     gDmaFinish = 1;
     DmaDisableInt(gdma_test_channel);
}


/*******************************************************************************
** Name: UartDevShellBspMemToUart
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.27
** Time: 17:14:13
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBspMemToUart(HDC dev, uint8 * pstr)
{
    uint8 uart_ch;

//    char test_Str[30] = {'a','a','a','a','a','b','b','b','b','b', \
//                               'c','c','c','c','c','d','d','d','d','d', \
//                               'e','e','e','e','e','f','f','f','f','f',};
    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;
    gUartint_flag = 0;
    gdma_test_channel = DMA_CHN1;

    /*send bsp help info*/

     switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    while(pstr[0] == ' ')
        pstr++;
    if( pstr != NULL )      //get the input uart channel no. which we want to test.
        uart_ch = StringtoNum( pstr );

    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);

    // test signle
    DmaEnableInt(gdma_test_channel );

    Memory_to_Uart(gdma_test_channel,gUartChannel,(uint32)dmatest_Str,sizeof(dmatest_Str));//ok
//    Dma2Uart(gdma_test_channel,gUartChannel,(uint32)test_Str,sizeof(test_Str));//ok
    while(!gDmaFinish);

    DelayMs(1);

//    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);
//    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX | UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartIntIsr0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartIntIsr1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartIntIsr2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartIntIsr3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartIntIsr4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartIntIsr5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    if(gDmaFinish)
        rk_print_string ("\r\nmemory to uart through DMA test SUCCESS.\r\n");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UartDevShellBspUartToMem
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.27
** Time: 17:13:25
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBspUartToMem(HDC dev, uint8 * pstr)
{
    uint8 uart_ch;

//    char test_Str[30] = {'a','a','a','a','a','b','b','b','b','b', \
//                               'c','c','c','c','c','d','d','d','d','d', \
//                               'e','e','e','e','e','f','f','f','f','f',};
    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;
    gUartint_flag = 0;
    gdma_test_channel = DMA_CHN1;
    gUartData = 0;

    /*send bsp help info*/

     switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    while(pstr[0] == ' ')
        pstr++;
    if( pstr != NULL )  //get the input uart channel no. which we want to test.
        uart_ch = StringtoNum( pstr );

    DmaEnableInt(gdma_test_channel );
    while(1)
    {
        UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);

        if(gUartint_flag)
        {
            Uart_to_Memory(gdma_test_channel,gUartChannel,(uint32)&gUartData,1 ); //ok

            if(gUartData == 'q' || gUartData == 'Q')
                break;
            UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
            UARTWriteByte(gUartChannel,(uint8*)&gUartData,1);

            while(!gUartint_flag);
            gUartint_flag = 0;
        }
    }

    while(!gDmaFinish);

    DelayMs(1);

//    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);
//    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX | UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartIntIsr0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartIntIsr1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartIntIsr2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartIntIsr3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartIntIsr4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartIntIsr5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    if(gDmaFinish)
        rk_print_string ("\r\nuart to memory through DMA test SUCCESS.\r\n");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UartDevShellBspIsrHandle
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.11.21
** Time: 16:57:25
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN void UartDevShellBspIsrHandle(eUART_CH uartID)
{
    uint32 uartIntType, realsize;

    if(uartID >= UART_MAX)
        return;

    uartIntType = UARTGetIntType(uartID);

    if (uartIntType & UART_INT_READ_FIFO_NOT_EMPTY)
    {
        gUartint_flag = 1;
        gUartint_write_flag = 0;
        UARTSetIntDisabled(uartID,UART_IE_RX);
    }

    if (uartIntType & UART_INT_WRITE_FIFO_EMPTY)
    {
        gUartint_flag = 1;
        gUartint_write_flag = 1;
        UARTSetIntDisabled(uartID,UART_IE_TX);
    }
}

/*******************************************************************************
** Name: UartDevShellBspSR
** Input: HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.10.31
** Time: 16:07:33
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBspSR( HDC dev, uint8 * pstr)
{
    uint32 devID = 0;
    uint32 cnt = 0;
    char _data = 0;

    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;
    gUartint_flag = 0;

    //open uart clk
    switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    //Default initialization state, you can change from IOCONTROL
    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);

    //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);
    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    if( *pstr == NULL)
    {
        cnt = 1;
    }
    else
    {
        while(pstr[0] == ' ')
            pstr++;

        cnt = StringtoNum( pstr );
    }

    while( cnt )
    {
        UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);
        UARTReadByte(gUartChannel, (uint8*)&_data,1);
        /*revice data is or not 0x5a*/

        while(!gUartint_flag);
        if( gUartint_flag )
        {
            gUartint_flag = 0;
            if( _data == 0x5a )
            {
                UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
                UARTWriteByte(gUartChannel,(uint8*)&_data,1);

                while(!gUartint_flag);
                gUartint_flag = 0;
                _data = 0;
                cnt--;
            }
        }
    }

//    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);
//    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX | UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartIntIsr0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartIntIsr1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartIntIsr2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartIntIsr3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartIntIsr4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartIntIsr5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: UartDevShellBspRecive
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.10.31
** Time: 16:06:49
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBspRecive(HDC dev, uint8 * pstr)
{
    char _data ;

    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;
    gUartint_flag = 0;

    memset(&gUart_st,0,sizeof(gUart_st));

    switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    //Default initialization state, you can change from IOCONTROL
    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);
    //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    /*recive bsp help info*/
    /*recive bsp test*/
    while(1)
    {
        UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);
        UARTReadByte(gUartChannel, (uint8*)&gUartData,1);

        while(!gUartint_flag);
        if( gUartint_flag )
        {
            /*revice data is or not 0x5a*/
            if(gUartData == 'q' || gUartData == 'Q')
            {
                //rk_printf("  %c",gUartData);
                gUartint_flag = 0;
                break;
            }
            UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
            UARTWriteByte(gUartChannel,(uint8*)&gUartData,1);

            while(!gUartint_flag);
            gUartint_flag = 0;
            gUartData = 0;

        }
    }

    /*??os???uart??????*/
    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_RX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartIntIsr0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartIntIsr1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartIntIsr2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartIntIsr3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartIntIsr4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartIntIsr5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: UartDevShellBspSend
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.10.31
** Time: 16:04:39
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBspSend(HDC dev, uint8 * pstr)
{
    uint32 cnt = 1;
    char _data = 0x5a;

    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;
    gUartint_flag = 0;

    memset(&gUart_st,0,sizeof(gUart_st));

     switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }
    //Default initialization state, you can change from IOCONTROL
    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);
        //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }


    /*send bsp help info*/
    if( *pstr == NULL)
    {
        cnt = 1;
    }
    else
    {
        /*send bsp test*/
        while(pstr[0] == ' ')
            pstr++;

        cnt = StringtoNum( pstr );
    }

    while( cnt )
    {
        UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);
        UARTWriteByte(gUartChannel,(uint8*)&_data,1);

        while(!gUartint_flag);
        gUartint_flag = 0;
        cnt--;
    }

    /*??os???uart??????*/
    UARTSetIntEnabled((eUART_CH)gUartChannel,UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartIntIsr0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartIntIsr1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartIntIsr2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartIntIsr3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartIntIsr4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartIntIsr5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UartDevShellBspSetBaudRate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.10.31
** Time: 16:02:19
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBspSetBaudRate(HDC dev, uint8 * pstr)
{
    uint32 baudRate = 0;

    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;

     switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    //Default initialization state, you can change from IOCONTROL
    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);
        //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }


    if( *pstr == NULL)
    {
        rk_print_string("miss baudrate parameter.please give the baud rate value. \
                \r\n vaule range: 110,300,600,1200,2400,4800,9600,14400,19200,38400,57600, \
                115200,23040,38040,46080,921600.\r\n");
        return RK_PARA_ERR;
    }

    while(pstr[0] == ' ')
        pstr++;

    baudRate = StringtoNum(pstr);
    if( baudRate <  UART_BR_110|| baudRate > UART_BR_921600 )
    {
        rk_print_string("wrong baudRate parameter.please check it.\r\n");
        return RK_PARA_ERR;
    }

    UARTSetBaudRate((eUART_CH)gUartChannel, 24*1000*1000,baudRate);

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: UartDevShellBspSetParity
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.10.31
** Time: 16:01:08
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBspSetParity(HDC dev, uint8 * pstr)
{
    uint32 parity = 0;

    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;

    switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    //Default initialization state, you can change from IOCONTROL
    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);
    //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    if( *pstr == NULL)
    {
        rk_print_string("miss parity parameter.please give the parity. \
                        \r\n0 - odd parity,1 - even parity,2 - none parity.\r\n");
        return RK_PARA_ERR;
    }

    while(pstr[0] == ' ')
        pstr++;

    parity = StringtoNum(pstr);
    if( parity <  UART_ODD_PARITY || parity > UART_PARITY_DISABLE)
    {
        rk_print_string("wrong parity parameter.please check it.\r\n");
        return RK_PARA_ERR;
    }

    UARTSetLcrReg(gUartChannel,  gUartBitWidth,  parity, UART_ONE_STOPBIT);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UartDevShellBspSetStopB
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.10.31
** Time: 15:52:28
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBspSetStopB(HDC dev, uint8 * pstr)
{
    uint32 devID = 0;
    uint32 stopB = 0;
    uint32 byteSize = 0;

    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;

     switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    //Default initialization state, you can change from IOCONTROL
    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);
    //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    if( *pstr == NULL)
    {
        rk_print_string("miss stop bit parameter.please use 0 or 1. \
                        \r\n0 - one stop bit,1 - 1.5 or 2 stop bits.\r\n");
        return RK_PARA_ERR;
    }

    while(pstr[0] == ' ')
        pstr++;

    stopB = StringtoNum(pstr);
    if( stopB !=  UART_ONE_STOPBIT && stopB != UART_ONE_AND_HALF_OR_TWO_STOPBIT )
    {
        rk_print_string("wrong stop bit parameter.please check it.\r\n");
        return RK_PARA_ERR;
    }

    UARTSetLcrReg(gUartChannel,  gUartBitWidth,  UART_PARITY_DISABLE, stopB);

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: UartDevShellDspSetbitw
** Input: HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.10.31
** Time: 15:49:57
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBspSetbitW( HDC dev, uint8 * pstr)
{
    uint32 byteSize = 0;

    gUartChannel = UART_CH1;
    gUartBitRate = UART_BR_115200;
    gUartBitWidth = UART_DATA_8B;

    switch( gUartChannel )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    //Default initialization state, you can change from IOCONTROL
    UARTInit(gUartChannel,
                gUartBitRate,
                gUartBitWidth,
                UART_ONE_STOPBIT,
                UART_PARITY_DISABLE);
        //UARTSetIntEnabled(UART_IE_RX | UART_IE_TX);

    switch( gUartChannel )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartDevShellBspIsrHandle0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartDevShellBspIsrHandle1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartDevShellBspIsrHandle2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartDevShellBspIsrHandle3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartDevShellBspIsrHandle4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartDevShellBspIsrHandle5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;
    }

    if( *pstr == NULL)
    {
        rk_print_string("miss bit width parameter.please use 5 - 8. \r\n");
        return RK_PARA_ERR;
    }

    while(pstr[0] == ' ')
        pstr++;

    byteSize = StringtoNum(pstr);
    if( byteSize <  UART_DATA_5B || byteSize > UART_DATA_8B )
    {
        rk_print_string("wrong stop bit parameter.please use 5 - 8.\r\n");
        return RK_PARA_ERR;
    }

    UARTSetLcrReg(gUartChannel,  byteSize,  UART_PARITY_DISABLE, UART_ONE_AND_HALF_OR_TWO_STOPBIT);

    return RK_SUCCESS;
}

#endif

#ifdef SHELL_BSP

/*******************************************************************************
** Name: UartDevShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.14
** Time: 11:26:02
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellBsp(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellUartBspName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellUartBspName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                                 //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellUartBspName[i].CmdDes, pItem);
    if(ShellUartBspName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellUartBspName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;
}
#endif

/*******************************************************************************
** Name: UsbOtgDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hUartDev;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    //Get UartDev ID...
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
    //Open UartDev...
    hUartDev = RKDev_Open(DEV_CLASS_UART, 0, NOT_CARE);

    if((hUartDev == NULL) || (hUartDev == (HDC)RK_ERROR) || (hUartDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("UartDev open failure");
        return RK_SUCCESS;
    }

    //do test....


    //close UsbOtgDev...

    RKDev_Close(hUartDev);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbOtgDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellDel(HDC dev, uint8 * pstr)
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
    //Get UsbOtgDev ID...
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

    if(RKDev_Delete(DEV_CLASS_UART, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("UartDev delete failure");
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: UsbOtgDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellMc(HDC dev, uint8 * pstr)
{
    UART_DEV_ARG stUartDevArg;
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

    stUartDevArg.dwBitWidth = UART_DATA_8B;
    stUartDevArg.dwBitRate = UART_BR_115200;

    //Create UartDev...
    ret = RKDev_Create(DEV_CLASS_UART, DevID, &stUartDevArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("UartDev create failure");
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: UsbOtgDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_SHELL_
SHELL FUN rk_err_t UartDevShellPcb(HDC dev, uint8 * pstr)
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
    //Get UartDev ID...
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
    if(gpstUartDevISR[DevID] != NULL)
    {
        rk_print_string("\r\n");
        rk_printf_no_time("UartDev ID = %d Pcb list as follow:", DevID);
    }
    else
    {
        rk_print_string("\r\n");
        rk_printf_no_time("UartDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;

}


#endif

#endif


