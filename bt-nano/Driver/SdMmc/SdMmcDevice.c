/*
********************************************************************************************
*
*           Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                                    All rights reserved.
*
* FileName: ..\Driver\SdMmc\SdMmcDevice.c
* Owner: Aaron.sun
* Date: 2014.2.19
* Time: 15:58:59
* Desc: sd mmc control class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.2.19     15:58:59   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_SDMMC_SDMMCDEVICE_C__

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

typedef struct _SDC_TRANS_INFO
{
    uint32     TransLen;                   //已经发送或接收的数据长度
    uint32     NeedLen;                  //需要传输的数据长度
    uint8      *pBuf;                    //中断数据接收或发送数据用到的buf地址                                        //指针用32 bit的uint32指针，就可以满足SDMMC FIFO要求的32 bits对齐的要求了，                                    //这样就算你的数据没有4字节对齐，也会因为用了uint32指针，每次向FIFO操作是4字节对齐的。

}SDC_TRANS_INFO;



typedef  struct _SDC_DEVICE_CLASS
{
    DEVICE_CLASS stSdMmcDevice;

    pSemaphore osSdMmcOperReqSem;
    pSemaphore osSdMmcOperSem;
    uint32 dwSdMmcInt;

    rk_err_t SdMmcErrCode;
    uint32 dwTxRxStatus;

    SDC_TRANS_INFO stTransInfo;

    SDMMC_PORT enSdMmcPort;
    BUS_WIDTH enBusWidith;
    uint32 dwCurBusFreq;
    uint32 DmaCh;
    HDC hDma;
    uint32 Channel;

    void(*isr_hook)(HDC hSdc, void *arg);

}SDC_DEVICE_CLASS;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SDC_DEVICE_CLASS * gpstSdcDevInf[SDC_MAX];


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
rk_err_t SdcDevCheckHandler(HDC dev);
rk_err_t SdMmcShellHelp(HDC dev,  uint8 * pstr);
void SdcDevDmaDRQ0(uint32 ch);
void SdcDevDmaDRQ1(uint32 ch);
void SdcDevDmaIsr(SDMMC_PORT SDCPort, uint32 ch);
void SdcIntIRQ0(void);
rk_err_t SdcDevResume(HDC dev);
rk_err_t SdcDevSuspend(HDC dev, uint32 Level);
rk_err_t SdcDevDeInit(SDC_DEVICE_CLASS * pstSdcDev);
rk_err_t SdcDevInit(SDC_DEVICE_CLASS * pstSdcDev);
void SdcDevIntIsr(SDMMC_PORT SDCPort);
rk_err_t SdMmcShellDel(HDC dev, uint8 * pstr);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SdcDev_SetPower
** Input:HDC dev, BOOL enable
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.13
** Time: 15:59:35
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
COMMON API rk_err_t SdcDev_SetPower(HDC dev, BOOL enable)
{
    SDC_DEVICE_CLASS * pstSdcDev =  (SDC_DEVICE_CLASS *)dev;
    rkos_semaphore_take(pstSdcDev->osSdMmcOperReqSem, MAX_DELAY);

    SDC_SetPower(pstSdcDev->enSdMmcPort, enable);

    rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: SdcDev_SetBusWidth
** Input:HDC dev, uint32 BusWidth
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.16
** Time: 17:12:49
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
COMMON API rk_err_t SdcDev_SetBusWidth(HDC dev, uint32 BusWidth)
{
    SDC_DEVICE_CLASS * pstSdcDev =  (SDC_DEVICE_CLASS *)dev;
    rkos_semaphore_take(pstSdcDev->osSdMmcOperReqSem, MAX_DELAY);
    if (pstSdcDev->enBusWidith != BusWidth)
    {
        SDC_SetBusWidth(pstSdcDev->enSdMmcPort, BusWidth);
        pstSdcDev->enBusWidith = BusWidth;
    }

    rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: SdcDev_SetBusFreq
** Input:HDC dev, uint32 BusFreq
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.16
** Time: 17:10:05
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
COMMON API rk_err_t SdcDev_SetBusFreq(HDC dev, uint32 BusFreq)
{
    SDC_DEVICE_CLASS * pstSdcDev =  (SDC_DEVICE_CLASS *)dev;
    rkos_semaphore_take(pstSdcDev->osSdMmcOperReqSem, MAX_DELAY);
    if (pstSdcDev->dwCurBusFreq != BusFreq)
    {
        SDC_UpdateFreq(pstSdcDev->enSdMmcPort, BusFreq);
        pstSdcDev->dwCurBusFreq = BusFreq;
    }

    rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdcDev_SendCmd
** Input:HDC dev, SDC_CMD_ARG * stCmdArg
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.16
** Time: 17:06:29
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
COMMON API rk_err_t SdcDev_SendCmd(HDC dev, SDC_CMD_ARG * pstCmdArg)
{
    SDC_DEVICE_CLASS * pstSdcDev =  (SDC_DEVICE_CLASS *)dev;

    rkos_semaphore_take(pstSdcDev->osSdMmcOperReqSem, MAX_DELAY);

    SDC_SetCmdArg(pstSdcDev->enSdMmcPort, pstCmdArg->CmdArg);

    pstSdcDev->dwTxRxStatus =  DEVICE_STATUS_IO_CONTROL;

    if (RK_SUCCESS != SDC_StartCmd(pstSdcDev->enSdMmcPort, (pstCmdArg->Cmd & ~(RSP_BUSY)) | START_CMD | USE_HOLD_REG))
    {
        rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
        return RK_ERROR;
    }

    rkos_semaphore_take(pstSdcDev->osSdMmcOperSem, MAX_DELAY);

    pstSdcDev->dwTxRxStatus =  DEVICE_STATUS_IDLE;

    if ((pstSdcDev->SdMmcErrCode == RK_ERROR) ||
            (pstSdcDev->SdMmcErrCode == RK_TIMEOUT))
    {
        rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
        return pstSdcDev->SdMmcErrCode;
    }

    if ((pstCmdArg->Cmd & R_EXPECT) && (pstCmdArg->RespBuf != NULL))
    {
        pstCmdArg->RespBuf[0] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 0);
        if (pstCmdArg->Cmd & LONG_R)
        {
            pstCmdArg->RespBuf[1] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 1);
            pstCmdArg->RespBuf[2] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 2);
            pstCmdArg->RespBuf[3] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 3);
        }
    }

    rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);

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
** Name: SdcDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 15:07:02
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
COMMON FUN rk_err_t SdcDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < SDC_MAX; i++)
    {
        if(gpstSdcDevInf[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: SdcDevDmaDRQ0
** Input:void
** Return: uint32 ch
** Owner:Aaron.sun
** Date: 2014.4.25
** Time: 15:00:21
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
COMMON FUN void SdcDevDmaDRQ0(uint32 ch)
{
    SdcDevDmaIsr(SDC0,ch);
}

/*******************************************************************************
** Name: SdcDevDmaDRQ1
** Input:void
** Return: uint32 ch
** Owner:aron.chen
** Date: 2015.5.22
** Time: 8:00:21
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
COMMON FUN void SdcDevDmaDRQ1(uint32 ch)
{
    SdcDevDmaIsr(SDC1,ch);
}

/*******************************************************************************
** Name: SdcDevDmaIsr
** Input:SDMC_PORT SdcPort
** Return: void
** Owner:Aaron.sun
** Date: 2014.4.25
** Time: 14:57:47
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
COMMON FUN void SdcDevDmaIsr(SDMMC_PORT SDCPort, uint32 ch)
{

    if (gpstSdcDevInf[SDCPort] != NULL)
    {
        if(gpstSdcDevInf[SDCPort]->DmaCh != ch)
        {
            printf("gpstSdcDevInf[SDCPort]->DmaCh = %d, ch = %d", gpstSdcDevInf[SDCPort]->DmaCh,ch);
            while(1);
        }

        DmaDev_RealseChannel(gpstSdcDevInf[SDCPort]->hDma, gpstSdcDevInf[SDCPort]->DmaCh);
    }

}

/*******************************************************************************
** Name: SdMmcIntIRQ
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.2.21
** Time: 17:42:55
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
void SdcIntIRQ0(void)
{
    SdcDevIntIsr(SDC0);
}

/*******************************************************************************
** Name: SdMmcIntIRQ
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.2.21
** Time: 17:42:55
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
void SdcIntIRQ1(void)
{
    SdcDevIntIsr(SDC1);
}

/*******************************************************************************
** Name: SdcDevRead
** Input:HDC dev,uint32 pos, void* buffer, uint32 size,uint8 mode,pRx_indicate Rx_indicate
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.19
** Time: 15:59:49
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
rk_size_t SdcDev_Read(HDC dev, SDC_CMD_ARG * pstCmdArg)
{
    //uint32 i;
    rk_err_t ret;
    DMA_CFGX DmaCfg;

    SDC_DEVICE_CLASS * pstSdcDev =  (SDC_DEVICE_CLASS *)dev;

    if ((pstCmdArg == NULL) && (dev == NULL))
    {
        return (rk_size_t)RK_PARA_ERR;
    }

    if ((pstCmdArg->BlkSize > pstCmdArg->Size) || (pstCmdArg->Size == 0))
    {
        return (rk_size_t)RK_PARA_ERR;
    }

    rkos_semaphore_take(pstSdcDev->osSdMmcOperReqSem, MAX_DELAY);

    SDC_ResetFIFO(pstSdcDev->enSdMmcPort);

    if ((((uint32)pstCmdArg->Buf % 4) == 0) && ((pstCmdArg->Size % 4) == 0) && (pstCmdArg->Size >= (FIFO_DEPTH * 4)))
        //if(0)
    {
        while(1)
        {
            ret = DmaDev_GetChannel(pstSdcDev->hDma);
            if (ret < 0)
            {
                rk_printf("read get channel fail");
                //return (rk_size_t)RK_ERROR;
                rkos_sleep(2);
            }
            else
            {
                break;
            }
        }
        pstSdcDev->DmaCh = (uint32)ret;

        if (pstSdcDev->enSdMmcPort == SDC0)
        {
            DmaCfg.CTLL = DMA_CTLL_EMMC_RD;
            DmaCfg.CFGL = DMA_CFGL_EMMC_RD;
            DmaCfg.CFGH = DMA_CFGH_EMMC_RD;
            DmaCfg.pLLP = 0;
            //DmaCfg = {DMA_CTLL_EMMC_RD, DMA_CFGL_EMMC_RD , DMA_CFGH_EMMC_RD,0};
        }
        else if (pstSdcDev->enSdMmcPort == SDC1)
        {
            DmaCfg.CTLL = DMA_CTLL_SDMMC_RD;
            DmaCfg.CFGL = DMA_CFGL_SDMMC_RD;
            DmaCfg.CFGH = DMA_CFGH_SDMMC_RD;
            DmaCfg.pLLP = 0;
            //DmaCfg = {DMA_CTLL_SDMMC_RD, DMA_CFGL_SDMMC_RD , DMA_CFGH_SDMMC_RD,0};
        }
        pFunc pCallBack;

        if (pstSdcDev->enSdMmcPort == SDC0)
        {
            pCallBack = SdcDevDmaDRQ0;
        }
        else if (pstSdcDev->enSdMmcPort == SDC1)
        {
            pCallBack = SdcDevDmaDRQ1;
        }
        ret = DmaDev_DmaStart(pstSdcDev->hDma, pstSdcDev->DmaCh, SDC_GetFIFOAddr(pstSdcDev->enSdMmcPort),
                              (uint32)pstCmdArg->Buf, pstCmdArg->Size / 4, &DmaCfg,  pCallBack);

        if (ret != RK_SUCCESS)
        {
            rk_printf("read dma start fail %x= %d = %x = %d",pstSdcDev->hDma, pstSdcDev->DmaCh, (uint32)pstCmdArg->Buf, pstCmdArg->Size);
            DmaDev_RealseChannel(pstSdcDev->hDma, pstSdcDev->DmaCh);
            rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
            return (rk_size_t)RK_ERROR;
        }

        pstSdcDev->dwTxRxStatus = DEVICE_STATUS_SYNC_DMA_READ;

        SDC_EnableDMA(pstSdcDev->enSdMmcPort);

        pstSdcDev->stTransInfo.NeedLen = 0;


    }
    else
    {

        pstSdcDev->dwTxRxStatus = DEVICE_STATUS_SYNC_PIO_READ;

        pstSdcDev->stTransInfo.pBuf = pstCmdArg->Buf;
        pstSdcDev->stTransInfo.TransLen = pstCmdArg->Size;
        pstSdcDev->stTransInfo.NeedLen = pstCmdArg->Size;

    }

    SDC_SetBlockSize(pstSdcDev->enSdMmcPort, pstCmdArg->BlkSize);
    SDC_SetByteCnt(pstSdcDev->enSdMmcPort, pstCmdArg->Size);
    SDC_SetCmdArg(pstSdcDev->enSdMmcPort, pstCmdArg->CmdArg);

    //rkos_enter_critical();
    if (RK_SUCCESS != SDC_StartCmd(pstSdcDev->enSdMmcPort, (pstCmdArg->Cmd & ~(RSP_BUSY)) | START_CMD | USE_HOLD_REG))
    {
        rk_printf("sdc read cmd send failure");
        if(pstSdcDev->dwTxRxStatus == DEVICE_STATUS_SYNC_DMA_READ)
        {
            DmaDev_RealseChannel(pstSdcDev->hDma, pstSdcDev->DmaCh);
        }

        pstSdcDev->dwTxRxStatus = DEVICE_STATUS_IDLE;
        rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
        return RK_ERROR;
    }
    //rkos_exit_critical();



    rkos_semaphore_take(pstSdcDev->osSdMmcOperSem, MAX_DELAY);

    if (pstSdcDev->stTransInfo.NeedLen != 0)
    {
        SDC_ReadData(pstSdcDev->enSdMmcPort, pstSdcDev->stTransInfo.pBuf, pstSdcDev->stTransInfo.NeedLen);
    }

    if ((pstCmdArg->Cmd & R_EXPECT) && (pstCmdArg->RespBuf != NULL))
    {
        pstCmdArg->RespBuf[0] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 0);
        if (pstCmdArg->Cmd & LONG_R)
        {
            pstCmdArg->RespBuf[1] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 1);
            pstCmdArg->RespBuf[2] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 2);
            pstCmdArg->RespBuf[3] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 3);
        }
    }

    if(pstSdcDev->dwTxRxStatus == DEVICE_STATUS_SYNC_DMA_READ)
    {
         SDC_DisableDMA(pstSdcDev->enSdMmcPort);
    }

    if ((pstSdcDev->SdMmcErrCode == RK_ERROR)
            ||(pstSdcDev->SdMmcErrCode == RK_TIMEOUT))
    {
        rk_printf("sdmmc read error = %d, size = %d, LBA = %d", pstSdcDev->SdMmcErrCode, pstCmdArg->Size, pstCmdArg->CmdArg);
        if(pstSdcDev->dwTxRxStatus == DEVICE_STATUS_SYNC_DMA_READ)
        {
            DmaDev_RealseChannel(pstSdcDev->hDma, pstSdcDev->DmaCh);
        }

        pstSdcDev->dwTxRxStatus = DEVICE_STATUS_IDLE;
        rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
        return pstSdcDev->SdMmcErrCode;
    }

    pstSdcDev->dwTxRxStatus = DEVICE_STATUS_IDLE;

    rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
    return pstCmdArg->Size;

}


/*******************************************************************************
** Name: SdcDevIntIsr
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.2.19
** Time: 15:59:49
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
void SdcDevIntIsr(SDMMC_PORT SDCPort)
{
    uint32 event,realsize;

    #if 0
    static uint32 x;

    if(x > 1000)
    {
        printf("x");
        x = 0;
    }
    x++;
    #endif

    event = SDC_GetIRQ(SDCPort, &gpstSdcDevInf[SDCPort]->dwSdMmcInt);

    if (gpstSdcDevInf[SDCPort] != NULL)
    {

        if (event & SDC_EVENT_SDC_ERR)
        {
            gpstSdcDevInf[SDCPort]->SdMmcErrCode = RK_ERROR;
            //if((gpstSdcDevInf[SDCPort]->dwTxRxStatus != DEVICE_STATUS_SYNC_DMA_WRITE) && (gpstSdcDevInf[SDCPort]->dwTxRxStatus != DEVICE_STATUS_SYNC_PIO_WRITE))
            rkos_semaphore_give_fromisr(gpstSdcDevInf[SDCPort]->osSdMmcOperSem);
            SDC_ResetFIFO(SDCPort);
            SDC_ClearIRQ(SDCPort, 0xFFFFFFFF);
            return;
        }

        if (event & SDC_EVENT_RESP_TIMEOUT)
        {
            gpstSdcDevInf[SDCPort]->SdMmcErrCode = RK_TIMEOUT;
            //if((gpstSdcDevInf[SDCPort]->dwTxRxStatus != DEVICE_STATUS_SYNC_DMA_WRITE) && (gpstSdcDevInf[SDCPort]->dwTxRxStatus != DEVICE_STATUS_SYNC_PIO_WRITE))
            rkos_semaphore_give_fromisr(gpstSdcDevInf[SDCPort]->osSdMmcOperSem);
            SDC_ResetFIFO(SDCPort);
            SDC_ClearIRQ(SDCPort, 0xFFFFFFFF);
            return;

        }

        if (event & SDC_EVENT_TX_DATA_REQ)
        {
            if ((gpstSdcDevInf[SDCPort]->dwTxRxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                    || (gpstSdcDevInf[SDCPort]->dwTxRxStatus == DEVICE_STATUS_SYNC_PIO_WRITE))
            {
                if (gpstSdcDevInf[SDCPort]->stTransInfo.NeedLen != 0)
                {
                    realsize = SDC_WriteData(SDCPort, gpstSdcDevInf[SDCPort]->stTransInfo.pBuf, gpstSdcDevInf[SDCPort]->stTransInfo.NeedLen);
                    gpstSdcDevInf[SDCPort]->stTransInfo.pBuf += realsize;
                    gpstSdcDevInf[SDCPort]->stTransInfo.NeedLen -= realsize;
                }
            }
        }

        if (event & SDC_EVENT_RX_DATA_REQ)
        {

            if ((gpstSdcDevInf[SDCPort]->dwTxRxStatus == DEVICE_STATUS_ASYNC_PIO_READ)
                    || (gpstSdcDevInf[SDCPort]->dwTxRxStatus == DEVICE_STATUS_SYNC_PIO_READ))
            {
                if (gpstSdcDevInf[SDCPort]->stTransInfo.NeedLen != 0)
                {
                    realsize = SDC_ReadData(SDCPort, gpstSdcDevInf[SDCPort]->stTransInfo.pBuf, gpstSdcDevInf[SDCPort]->stTransInfo.NeedLen);
                    gpstSdcDevInf[SDCPort]->stTransInfo.pBuf += realsize;
                    gpstSdcDevInf[SDCPort]->stTransInfo.NeedLen -= realsize;
                }
            }
        }

        if (event & SDC_EVENT_CMD_OK)
        {
            gpstSdcDevInf[SDCPort]->SdMmcErrCode = RK_SUCCESS;
            if (gpstSdcDevInf[SDCPort]->dwTxRxStatus == DEVICE_STATUS_IO_CONTROL)
            {
                rkos_semaphore_give_fromisr(gpstSdcDevInf[SDCPort]->osSdMmcOperSem);
            }
        }

        if (event & SDC_EVENT_DATA_TRANS_OK)
        {
            gpstSdcDevInf[SDCPort]->SdMmcErrCode = RK_SUCCESS;
            rkos_semaphore_give_fromisr(gpstSdcDevInf[SDCPort]->osSdMmcOperSem);
        }

        if (event & SDC_EVENT_SDIO)
        {
            if (gpstSdcDevInf[SDCPort]->isr_hook)
            {
                gpstSdcDevInf[SDCPort]->isr_hook((HDC)gpstSdcDevInf[SDCPort], NULL);
            }

        }

    }

    SDC_ClearIRQ(SDCPort, gpstSdcDevInf[SDCPort]->dwSdMmcInt);

}

/*******************************************************************************
** Name: SdcDev_DataTransIdle
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.5
** Time: 14:52:31
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
COMMON API rk_err_t SdcDev_DataTransIdle(HDC dev)
{
    SDC_DEVICE_CLASS * pstSdcDev =  (SDC_DEVICE_CLASS *)dev;

    while (SDC_CheckDataBusy(pstSdcDev->enSdMmcPort) == RK_ERROR)
    {
        //vTaskDelay(1);
        //DelayUs(10);
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdcDevWrite
** Input:HDC dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.19
** Time: 15:59:49
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
rk_size_t SdcDev_Write(HDC dev, SDC_CMD_ARG * pstCmdArg)
{
    //uint32 i;
    SDC_DEVICE_CLASS * pstSdcDev =  (SDC_DEVICE_CLASS *)dev;
    rk_err_t ret;
    DMA_CFGX DmaCfg;

    if ((pstCmdArg == NULL) || (dev == NULL))
    {
        return (rk_size_t)RK_PARA_ERR;
    }

    rkos_semaphore_take(pstSdcDev->osSdMmcOperReqSem, MAX_DELAY);

    SDC_ResetFIFO(pstSdcDev->enSdMmcPort);

    if ((((uint32)pstCmdArg->Buf % 4) == 0) && ((pstCmdArg->Size % 4) == 0) && (pstCmdArg->Size >= (FIFO_DEPTH * 4)))
        //if(0)
    {

        while(1)
        {
            ret = DmaDev_GetChannel(pstSdcDev->hDma);
            if (ret < 0)
            {
                 rk_printf("write get channel fail");
                 rkos_sleep(2);
                //return (rk_size_t)RK_ERROR;
            }
            else
            {
                break;
            }

       }
        pstSdcDev->DmaCh = (uint32)ret;
        if (pstSdcDev->enSdMmcPort == SDC0)
        {
            DmaCfg.CTLL = DMA_CTLL_EMMC_WR;
            DmaCfg.CFGL = DMA_CFGL_EMMC_WR;
            DmaCfg.CFGH = DMA_CFGH_EMMC_WR;
            DmaCfg.pLLP = 0;
            //DmaCfg = {DMA_CTLL_EMMC_WD, DMA_CFGL_EMMC_WD , DMA_CFGH_EMMC_WD,0};
        }
        else if (pstSdcDev->enSdMmcPort == SDC1)
        {
            DmaCfg.CTLL = DMA_CTLL_SDMMC_WR;
            DmaCfg.CFGL = DMA_CFGL_SDMMC_WR;
            DmaCfg.CFGH = DMA_CFGH_SDMMC_WR;
            DmaCfg.pLLP = 0;
            //DmaCfg = {DMA_CTLL_SDMMC_WR, DMA_CFGL_SDMMC_WR, DMA_CFGH_SDMMC_WR,0};
        }
        pFunc pCallBack;

        if (pstSdcDev->enSdMmcPort == SDC0)
        {
            pCallBack = SdcDevDmaDRQ0;
        }
        else if (pstSdcDev->enSdMmcPort == SDC1)
        {
            pCallBack = SdcDevDmaDRQ1;
        }

        ret = DmaDev_DmaStart(pstSdcDev->hDma, pstSdcDev->DmaCh, (uint32)pstCmdArg->Buf, SDC_GetFIFOAddr(pstSdcDev->enSdMmcPort), pstCmdArg->Size / 4, &DmaCfg,  pCallBack);

        if (ret != RK_SUCCESS)
        {
            rk_printf("write dma start fail %x= %d = %x = %d",pstSdcDev->hDma, pstSdcDev->DmaCh, (uint32)pstCmdArg->Buf, pstCmdArg->Size);
            rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
            return (rk_size_t)RK_ERROR;
        }

        pstSdcDev->dwTxRxStatus = DEVICE_STATUS_SYNC_DMA_WRITE;

        SDC_EnableDMA(pstSdcDev->enSdMmcPort);


    }
    else
    {
        pstSdcDev->dwTxRxStatus = DEVICE_STATUS_SYNC_PIO_WRITE;
        pstSdcDev->stTransInfo.pBuf = pstCmdArg->Buf;
        pstSdcDev->stTransInfo.TransLen = pstCmdArg->Size;
        pstSdcDev->stTransInfo.NeedLen = pstCmdArg->Size;
    }


    SDC_SetBlockSize(pstSdcDev->enSdMmcPort, pstCmdArg->BlkSize);
    SDC_SetByteCnt(pstSdcDev->enSdMmcPort, pstCmdArg->Size);
    SDC_SetCmdArg(pstSdcDev->enSdMmcPort, pstCmdArg->CmdArg);


    //rkos_enter_critical();

    if (RK_SUCCESS != SDC_StartCmd(pstSdcDev->enSdMmcPort, (pstCmdArg->Cmd & ~(RSP_BUSY)) | START_CMD | USE_HOLD_REG))
    {
        rk_printf("sdc write cmd send failure");

        if(pstSdcDev->dwTxRxStatus == DEVICE_STATUS_SYNC_DMA_WRITE)
        {
            DmaDev_RealseChannel(pstSdcDev->hDma, pstSdcDev->DmaCh);
        }

        pstSdcDev->dwTxRxStatus = DEVICE_STATUS_IDLE;
        rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
        return RK_ERROR;
    }

    //rkos_exit_critical();

    rkos_semaphore_take(pstSdcDev->osSdMmcOperSem, MAX_DELAY);

    if ((pstCmdArg->Cmd & R_EXPECT) && (pstCmdArg->RespBuf != NULL))
    {
        pstCmdArg->RespBuf[0] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 0);
        if (pstCmdArg->Cmd & LONG_R)
        {
            pstCmdArg->RespBuf[1] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 1);
            pstCmdArg->RespBuf[2] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 2);
            pstCmdArg->RespBuf[3] = SDC_GetResponse(pstSdcDev->enSdMmcPort, 3);
        }
    }

    if(pstSdcDev->dwTxRxStatus == DEVICE_STATUS_SYNC_DMA_WRITE)
    {
         SDC_DisableDMA(pstSdcDev->enSdMmcPort);
    }

    if ((pstSdcDev->SdMmcErrCode == RK_ERROR)
            || (pstSdcDev->SdMmcErrCode == RK_TIMEOUT))
    {
        rk_printf("sdmmc write error = %d, size = %d", pstSdcDev->SdMmcErrCode, pstCmdArg->Size);
        if(pstSdcDev->dwTxRxStatus == DEVICE_STATUS_SYNC_DMA_WRITE)
        {
            DmaDev_RealseChannel(pstSdcDev->hDma, pstSdcDev->DmaCh);
        }

        pstSdcDev->dwTxRxStatus = DEVICE_STATUS_IDLE;
        rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
        return pstSdcDev->SdMmcErrCode;
    }

    pstSdcDev->dwTxRxStatus = DEVICE_STATUS_IDLE;
    rkos_semaphore_give(pstSdcDev->osSdMmcOperReqSem);
    return pstCmdArg->Size;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: SdMmcDevCreate
** Input:void
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.2.19
** Time: 15:59:49
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_INIT_
HDC SdcDev_Create(uint32 DevID, void *arg)
{
    DEVICE_CLASS* pstDev;
    SDC_DEVICE_CLASS * pstSdcDev;
    SDMMC_PORT SDCPort;

    if (arg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    SDCPort = DevID;

    pstSdcDev =  rkos_memory_malloc(sizeof(SDC_DEVICE_CLASS));
    if (pstSdcDev == NULL)
    {
        return pstSdcDev;
    }

    pstSdcDev->osSdMmcOperReqSem     = rkos_semaphore_create(1,1);
    pstSdcDev->osSdMmcOperSem = rkos_semaphore_create(1,0);

    if ((pstSdcDev->osSdMmcOperReqSem && pstSdcDev->osSdMmcOperSem) == 0)
    {

        rkos_semaphore_delete(pstSdcDev->osSdMmcOperReqSem);
        rkos_semaphore_delete(pstSdcDev->osSdMmcOperSem);

        rkos_memory_free(pstSdcDev);
        rkos_exit_critical();
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstSdcDev;

    pstDev->suspend = SdcDevSuspend;
    pstDev->resume  = SdcDevResume;
    pstDev->SuspendMode = ENABLE_MODE;

    pstSdcDev->enBusWidith = BUS_WIDTH_8_BIT;
    pstSdcDev->enSdMmcPort = SDCPort;
    pstSdcDev->dwTxRxStatus = DEVICE_STATUS_IDLE;
    pstSdcDev->hDma = ((SDC_DEV_ARG *)arg)->hDma;
    pstSdcDev->Channel = ((SDC_DEV_ARG *)arg)->Channel;
    gpstSdcDevInf[DevID] = NULL;

#ifndef _EMMC_BOOT_
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_SDC_DEV, SEGMENT_OVERLAY_CODE);
#endif
#endif

    SdMmcDevHwInit(DevID, pstSdcDev->Channel);
    SdcDevInit(pstSdcDev);
    gpstSdcDevInf[DevID] = pstSdcDev;
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
** Name: SdcDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.19
** Time: 15:59:49
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_INIT_
rk_err_t SdcDevResume(HDC dev)
{
     SDC_DEVICE_CLASS * pstSdcDev = (SDC_DEVICE_CLASS *)dev;
     if(SdcDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }

     SdMmcDevHwInit(pstSdcDev->stSdMmcDevice.DevID, pstSdcDev->Channel);
     SdcDevInit(pstSdcDev);

     RKDev_Resume(pstSdcDev->hDma);

     pstSdcDev->stSdMmcDevice.State = DEV_STATE_WORKING;

     return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdcDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.19
** Time: 15:59:49
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_INIT_
rk_err_t SdcDevSuspend(HDC dev, uint32 Level)
{
    SDC_DEVICE_CLASS * pstSdcDev = (SDC_DEVICE_CLASS *)dev;
    if(SdcDevCheckHandler(dev) == RK_ERROR)
    {
       return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstSdcDev->stSdMmcDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstSdcDev->stSdMmcDevice.State = DEV_SATE_IDLE2;
    }

    SdcDevDeInit(pstSdcDev);
    SdMmcDevHwDeInit(pstSdcDev->stSdMmcDevice.DevID, pstSdcDev->Channel);

    RKDev_Suspend(pstSdcDev->hDma);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdcDevDelete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.19
** Time: 15:59:49
*******************************************************************************/
//rk_err_t SdcDevDelete(HDC dev)
_DRIVER_SDMMC_SDMMCDEVICE_INIT_
rk_err_t SdcDevDelete(uint32 DevID, void * arg)
{
    SDC_DEV_ARG *stSdcArg = arg;

    if(stSdcArg->hDma != NULL)
    {
        stSdcArg->hDma = gpstSdcDevInf[DevID]->hDma;
    }

    SdcDevDeInit(gpstSdcDevInf[DevID]);
    SdMmcDevHwDeInit(DevID, gpstSdcDevInf[DevID]->Channel);

    gpstSdcDevInf[DevID]->hDma = NULL;

    rkos_semaphore_delete( gpstSdcDevInf[DevID]->osSdMmcOperReqSem );
    rkos_semaphore_delete( gpstSdcDevInf[DevID]->osSdMmcOperSem );

    rkos_memory_free(gpstSdcDevInf[DevID]);

    gpstSdcDevInf[DevID] = NULL;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdcDevDeInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.19
** Time: 15:59:49
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_INIT_
rk_err_t SdcDevDeInit(SDC_DEVICE_CLASS * pstSdcDev)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SdcDevInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.19
** Time: 15:59:49
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_INIT_
rk_err_t SdcDevInit(SDC_DEVICE_CLASS * pstSdcDev)
{

    //open sdmmc  or emmc clk
    rk_printf("enSdMmcPort =%d\n",pstSdcDev->enSdMmcPort);

    //Default initialization state, you can change from IOCONTROL
    //SDC_Init(pstSdcDev->enSdMmcPort, SYS_MAX_FREQ);
    SDC_Init(pstSdcDev->enSdMmcPort, SYS_MAX_FREQ);
    return RK_SUCCESS;
}



/******************************************************************************
 * SdcDevRegisterIoIntIsr -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2014.7.15  written
 * Time : 9:22:04
 * --------------------
 ******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_COMMON_
rk_err_t SdcRegisterIoIntIsr(HDC hSdc, void(*isr_hook)(HDC hSdc, void *arg))
{
    SDC_DEVICE_CLASS * pstSdcDevtmep =  (SDC_DEVICE_CLASS *)hSdc;
    pstSdcDevtmep->isr_hook = isr_hook;
    return RK_SUCCESS;
}




#ifdef _SDEMMC_SHELL_

_DRIVER_SDMMC_SDMMCDEVICE_SHELL_DATA_
static  SHELL_CMD ShellSdMmcName[] =
{
    "pcb",NULL,"NULL","NULL",
    "create",NULL,"NULL","NULL",
    "delete",NULL,"NULL","NULL",
    "help",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",                         // the end
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: EmmcShell
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_SHELL_
rk_err_t SdcDev_Shell(HDC dev, uint8 * pstr)
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

    ret = ShellCheckCmd(ShellSdMmcName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    switch (i)
    {
        case 0x00:
            ret = SdMmcShellPcb(dev,pItem);
            break;

        case 0x01:
            ret = SdMmcShellCreate(dev,pItem);
            break;

        case 0x02:  //help
            ret = SdMmcShellDel(dev,pItem);
            break;

        case 0x03:  //help
            ret = SdMmcShellHelp(dev,pItem);
            break;

            default:
            ret = RK_ERROR;
            break;
    }
    return ret;
}


/*******************************************************************************
** Name: SdMmcShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_SHELL_
SHELL FUN rk_err_t SdMmcShellDel(HDC dev, uint8 * pstr)
{
    SDC_DEV_ARG stSdcArg;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("sdc.del : 删除sdc 设备命令.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    if (RKDev_Delete(DEV_CLASS_SDC, 1, &stSdcArg) != RK_SUCCESS)
    {
        printf("sdc1 delete failure\n");
        return RK_ERROR;
    }

    if(stSdcArg.hDma != NULL)
    {
        if (RKDev_Close(stSdcArg.hDma) != RK_SUCCESS)
        {
            printf("sdmmc hDma close failure\n");
            return RK_ERROR;
        }
    }

    return RK_SUCCESS;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SdMmcShellHelp
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.3
** Time: 16:31:39
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_SHELL_
SHELL FUN rk_err_t SdMmcShellHelp(HDC dev,  uint8 * pstr)
{
    pstr--;

    if ( StrLenA((uint8 *) pstr) != 0)
        return RK_ERROR;

    rk_print_string("sdmmc命令集提供了一系列的命令对sdmmc进行操作\r\n");
    rk_print_string("sdmmc包含的子命令如下:           \r\n");
    rk_print_string("pcb       显示pcb信息         \r\n");
    rk_print_string("open0     打开 SDC0 for eMMC etc       \r\n");
    rk_print_string("open1     打开 SDC1 for TF or WIFI etc      \r\n");
    //rk_print_string("test      测试命令    \r\n");
    rk_print_string("help      显示sdmmc命令帮助信息  \r\n");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: EmmcShellPcb
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_SHELL_
rk_err_t SdMmcShellPcb(HDC dev,  uint8 * pstr)
{
    uint32 DevID;
    SDC_DEVICE_CLASS * pstSdcDev;
    uint32 i;


    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    DevID = String2Num(pstr);

    if(DevID >= SDC_MAX)
    {
        return RK_ERROR;
    }

    pstSdcDev = gpstSdcDevInf[DevID];

    if(pstSdcDev == NULL)
    {
        rk_printf_no_time("sdc%d in not exist", DevID);
        return RK_SUCCESS;
    }

    rk_printf_no_time(".gpstSdcDevInf[%d]", DevID);
    rk_printf_no_time("    .stDirDevice");
    rk_printf_no_time("        .next = %08x",pstSdcDev->stSdMmcDevice.next);
    rk_printf_no_time("        .UseCnt = %d",pstSdcDev->stSdMmcDevice.UseCnt);
    rk_printf_no_time("        .SuspendCnt = %d",pstSdcDev->stSdMmcDevice.SuspendCnt);
    rk_printf_no_time("        .DevClassID = %d",pstSdcDev->stSdMmcDevice.DevClassID);
    rk_printf_no_time("        .DevID = %d",pstSdcDev->stSdMmcDevice.DevID);
    rk_printf_no_time("        .suspend = %08x",pstSdcDev->stSdMmcDevice.suspend);
    rk_printf_no_time("        .resume = %08x",pstSdcDev->stSdMmcDevice.resume);
    rk_printf_no_time("    .osSdMmcOperReqSem = %08x",pstSdcDev->osSdMmcOperReqSem);
    rk_printf_no_time("    .osSdMmcOperSem = %08x",pstSdcDev->osSdMmcOperSem);
    rk_printf_no_time("    .dwSdMmcInt = %08x",pstSdcDev->dwSdMmcInt);
    rk_printf_no_time("    .SdMmcErrCode = %08x",pstSdcDev->SdMmcErrCode);
    rk_printf_no_time("    .dwTxRxStatus = %d",pstSdcDev->dwTxRxStatus);
    rk_printf_no_time("    .stTransInfo");
    rk_printf_no_time("        .TransLen = %d",pstSdcDev->stTransInfo.TransLen);
    rk_printf_no_time("        .NeedLen = %d",pstSdcDev->stTransInfo.NeedLen);
    rk_printf_no_time("        .pBuf = %08x",pstSdcDev->stTransInfo.pBuf);
    rk_printf_no_time("    .enSdMmcPort = %08x",pstSdcDev->enSdMmcPort);
    rk_printf_no_time("    .enBusWidith = %08x",pstSdcDev->enBusWidith);
    rk_printf_no_time("    .dwCurBusFreq = %08x",pstSdcDev->dwCurBusFreq);
    rk_printf_no_time("    .DmaCh = %d",pstSdcDev->DmaCh);
    rk_printf_no_time("    .hDma = %08x",pstSdcDev->hDma);
    rk_printf_no_time("    .Channel = %08x",pstSdcDev->Channel);
    rk_printf_no_time("    .isr_hook = %08x",pstSdcDev->isr_hook);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: EmmcDevInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_SHELL_
rk_err_t SdMmcShellCreate(HDC dev,  uint8 * pstr)
{
    SDC_DEV_ARG stSdcArg;
    rk_err_t ret;

    int DevID;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA((uint8 *) pstr, "help", 4) == 0)
        {
            rk_print_string("sdmmc.open : sdmmc open cmd.\r\n");
            return RK_SUCCESS;
        }
    }
    else
#endif
    {
        pstr++;
        if (StrCmpA((uint8 *) pstr, "/0/0", 2) == 0)
        {
            DevID = 0;
            stSdcArg.Channel = 0;
        }
        else if (StrCmpA((uint8 *) pstr, "/1", 2) == 0)
        {
            DevID = 1;
            stSdcArg.Channel = 0;
        }
        else if (StrCmpA((uint8 *) pstr, "/0/1", 2) == 0)
        {
            DevID = 0;
            stSdcArg.Channel = 1;
        }
        else
        {
            return RK_ERROR;
        }
    }
    stSdcArg.hDma = RKDev_Open(DEV_CLASS_DMA, 0, NOT_CARE);
    if (stSdcArg.hDma <= 0)
    {
        rk_printf("hDma open failure");
    }
    ret = RKDev_Create(DEV_CLASS_SDC, DevID, &stSdcArg);
    if (ret != RK_SUCCESS)
    {
        rk_printf("sdc%d open failure", DevID);
    }

    return RK_SUCCESS;

}

#endif




#endif

